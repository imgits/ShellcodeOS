#pragma once
#include "page_frame.h"
#include <stdio.h>
#include "kernel.h"

#define  PAGE_DIR	_BASE		0xC0300000
#define  PAGE_TABLE_BASE		0xC0000000

#define  MMU_BLOCK_FREE		0x00
#define  MMU_BLOCK_USED		0x01
#define  MMU_BLOCK_RESERVED	0xFF

#define  MMU_SIZE_TO_BLOCKS(size)  ((((uint32)size) +   MMU_BLOCK_SIZE -1 ) / MMU_BLOCK_SIZE)
#define  MMU_BLOCKS_TO_SIZE(block) ((block) * MMU_BLOCK_SIZE) 
#define  MMU_TOTAL_BLOCKS    ((MMU_MEM_SIZE + MMU_BLOCK_SIZE - 1)/ MMU_BLOCK_SIZE)
template <uint32 MMU_START_ADDRESS, uint32 MMU_MEM_SIZE, uint32 MMU_BLOCK_SIZE>
class MMU
{
private:
	uint32 m_next_free_block;
	byte   m_block_map[MMU_TOTAL_BLOCKS];

private:
	uint32	find_free_blocks(uint32 from, uint32 blocks)
	{
		for (uint32 i = m_next_free_block; i < MMU_TOTAL_BLOCKS - blocks; i++)
		{
			if (m_block_map[i] == MMU_BLOCK_FREE)
			{
				int j = i;
				for (;j < i + blocks && j < MMU_TOTAL_BLOCKS - blocks; j++)
				{
					if (m_block_map[i] != MMU_BLOCK_FREE) break;
				}
				if (j = i + blocks)
				{
					return i;
				}
			}
		}
		return -1;
	}
public:
	MMU()
	{
		printf("MMU_START_ADDRESS=%08X MMU_MEM_SIZE=%08X MMU_BLOCK_SIZE=%08X\n", MMU_START_ADDRESS, MMU_MEM_SIZE, MMU_BLOCK_SIZE);
		m_next_free_block = 0;
		memset(m_block_map, 0, sizeof(m_block_map));
	}

	void Init()
	{
		printf("MMU_START_ADDRESS=%08X MMU_MEM_SIZE=%08X MMU_BLOCK_SIZE=%08X\n", MMU_START_ADDRESS, MMU_MEM_SIZE, MMU_BLOCK_SIZE);
		m_next_free_block = 0;
		memset(m_block_map, 0, sizeof(m_block_map));
	}

	uint32		alloc_virtual_space(uint32 size)
	{
		CHECK_PAGE_ALGINED(size);

		uint32 blocks = MMU_SIZE_TO_BLOCKS(size);

		uint32 start_block = find_free_blocks(m_next_free_block, blocks);
		if (start_block == -1) start_block = find_free_blocks(0, blocks);
		if (start_block == -1)
		{
			printf("alloc_virtual_space(%08X) failed\n", size);
			size = 0;
			return 0;
		}
		for (uint32 i = start_block; i < start_block + blocks; i++)
		{
			m_block_map[i] = MMU_BLOCK_USED;
		}
		size = MMU_BLOCKS_TO_SIZE(blocks);
		m_next_free_block = start_block + blocks;
		uint32 virtual_address = MMU_START_ADDRESS + MMU_BLOCKS_TO_SIZE(start_block);
		printf("alloc_virtual_space start_block = %08X %08X %08X\n", start_block, virtual_address, size);
		return virtual_address;
	}
	
	bool		free_virtual_space(uint32 start_address, uint32 size)
	{
		CHECK_PAGE_ALGINED(start_address);
		CHECK_PAGE_ALGINED(size);

		if (start_address < MMU_START_ADDRESS ||
			(uint64)start_address + (uint64)size >= (uint64)MMU_START_ADDRESS + (uint64)MMU_MEM_SIZE )
		{
			return false;
		}

		start_address -= MMU_START_ADDRESS;
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
		CHECK_PAGE_ALGINED(start_address);
		CHECK_PAGE_ALGINED(size);

		if (start_address < MMU_START_ADDRESS ||
			(uint64)start_address + (uint64)size >= (uint64)MMU_START_ADDRESS + (uint64)MMU_MEM_SIZE)
		{
			printf("reserve_virtual_space(%08X,%08X) out of range\n", start_address, size);
			return false;
		}
		start_address -= MMU_START_ADDRESS;
		uint32 blocks = MMU_SIZE_TO_BLOCKS(size);
		uint32 start_block = MMU_SIZE_TO_BLOCKS(start_address);
		for (int i = start_block; i < start_block + blocks;i++)
		{
			if (m_block_map[i] != MMU_BLOCK_FREE)
			{
				printf("reserve_virtual_space(%08X,%08X) m_block_map[%d] != MMU_BLOCK_FREE\n", start_address, size, i);
				return false;
			}
			m_block_map[i] = MMU_BLOCK_RESERVED;
		}
		printf("reserve_virtual_space(%08X,%08X) OK\n", start_address + MMU_START_ADDRESS, size);
		return true;
	}

	uint32		alloc_virtual_memory(uint32 size, uint32 protect = PT_PRESENT | PT_WRITABLE)
	{
		CHECK_PAGE_ALGINED(size);

		uint32 virtual_addr = alloc_virtual_space(size);
		if (virtual_addr == 0) return 0;
		uint32 pages = size >> 12;
		uint32 phys_addr = PAGE_FRAME_DB::alloc_physical_pages(pages);
		return PAGER::map_pages(phys_addr, virtual_addr, size, protect);
	}

	bool		free_virtual_memory(uint32 start_address, uint32 size)
	{
		CHECK_PAGE_ALGINED(start_address);
		CHECK_PAGE_ALGINED(size);

		free_virtual_space(start_address, size);
		PAGER::unmap_pages(start_address, size);
		return true;
	}
};

