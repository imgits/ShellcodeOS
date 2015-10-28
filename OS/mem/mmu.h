#pragma once
#include "page_frame.h"
#include "liballoc.h"

#define  PAGE_DIR	_BASE		0xC0300000
#define  PAGE_TABLE_BASE		0xC0000000

#define  MMU_BLOCK_FREE		0x00
#define  MMU_BLOCK_USED		0x01
#define  MMU_BLOCK_RESERVE	0xFF

#define  MMU_SIZE_TO_BLOCKS(size)  ((((uint32)size) +   MMU_BLOCK_SIZE -1 ) / MMU_BLOCK_SIZE)
#define  MMU_BLOCKS_TO_SIZE(block) ((block) * MMU_BLOCK_SIZE) 

template <int MMU_BLOCK_SIZE>
class MMU
{
private:
	uint32 m_block_size;
	uint32 m_start_block;
	uint32 m_end_block;
	uint32 m_next_free_block;
	byte   m_block_map[GB(2)/ MMU_BLOCK_SIZE];

private:
	uint32	find_free_blocks(uint32 from, uint32 blocks)
	{
		for (uint32 i = m_next_free_block; i < m_end_block - blocks; i++)
		{
			if (m_block_map[i] == MMU_BLOCK_FREE)
			{
				int j = i;
				for (;j < i + blocks && j < m_end_block - blocks; j++)
				{
					if (m_block_map[i] != MMU_BLOCK_FREE) break;
				}
				if (j = i + blocks)
				{
					return j;
				}
			}
		}
		return 0;
	}
public:
	MMU()
	{
		m_start_block = 0;
		m_end_block = 0;
		m_next_free_block = 0;
		memset(m_block_map, 0, sizeof(m_block_map));
	}

	void	Init(uint32 start_addr, uint32 end_addr)
	{
		m_block_size = MMU_BLOCK_SIZE;
		m_start_block = MMU_SIZE_TO_BLOCKS(start_addr);
		m_end_block = MMU_SIZE_TO_BLOCKS(end_addr);
		memset(m_block_map, 0, sizeof(m_block_map));
		m_next_free_block = m_start_block;
	}

	uint32		alloc_virtual_space(uint32 &size)
	{
		uint32 blocks = MMU_SIZE_TO_BLOCKS(size);

		uint32 start_block = find_free_blocks(m_next_free_block, blocks);
		if (start_block == 0) start_block = find_free_blocks(m_start_block, blocks);
		if (start_block == 0)
		{
			size = 0;
			return 0;
		}
		for (uint32 i = start_block; i < start_block + blocks; i++)
		{
			m_block_map[i] = MMU_BLOCK_USED;
		}
		size = MMU_BLOCKS_TO_SIZE(blocks);
		m_next_free_block = start_block + blocks;
		uint32 virtual_address = MMU_BLOCKS_TO_SIZE(start_block);
		return virtual_address;
	}
	
	bool		free_virtual_space(uint32 start_address, uint32 size)
	{
		uint32 blocks = MMU_SIZE_TO_BLOCKS(size);
		uint32 start_block = MMU_SIZE_TO_BLOCKS(start_address);
		for (int i = start_block; i < start_block + blocks;i++)
		{
			if (m_block_map[i] == MMU_BLOCK_FREE)
			{
				return false;
			}
			m_block_map[i] = MMU_BLOCK_FREE;
		}
		if (m_next_free_block > start_block) m_next_free_block = start_block;
		return true;
	}

	bool		reserve_virtual_space(uint32 start_address, uint32 size)
	{
		uint32 blocks = MMU_SIZE_TO_BLOCKS(size);
		uint32 start_block = MMU_SIZE_TO_BLOCKS(start_address);
		for (int i = start_block; i < start_block + blocks;i++)
		{
			if (m_block_map[i] != MMU_BLOCK_FREE)
			{
				return false;
			}
			m_block_map[i] = MMU_BLOCK_RESERVE;
		}
		return true;
	}

	uint32		alloc_virtual_memory(uint32 &size, uint32 protect)
	{
		uint32 virtual_addr = alloc_virtual_space(size);
		if (virtual_addr == 0) return 0;
		uint32 pages = size >> 12;
		uint32 phys_addr = PAGE_FRAME_DB::alloc_physical_pages(pages);
		return PAGE_FRAME_DB::map_pages(phys_addr, virtual_addr, size, protect);
	}

	bool		free_virtual_memory(uint32 start_address, uint32 size)
	{
		free_virtual_space(start_address, size);
		PAGE_FRAME_DB::unmap_pages(start_address, size);
		return true;
	}

	void*	alloc(uint32 size)
	{
		void* ptr = ::malloc(size);
		return ptr;
	}
};

