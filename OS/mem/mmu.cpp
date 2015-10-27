#include "mmu.h"
#include <string.h>
#include "page_frame.h"
#include "paging.h"

MMU::MMU()
{
	m_start_block =0;
	m_end_block =0;
	m_next_free_block = 0;
	memset(m_block_map,0, sizeof(m_block_map));
}

MMU::~MMU()
{
}

void MMU::Init(uint32 start, uint32 end)
{
	m_start_block = MMU_SIZE_TO_BLOCKS(start);
	m_end_block = MMU_SIZE_TO_BLOCKS(end);
	memset(m_block_map, 0, sizeof(m_block_map));
	m_next_free_block = m_start_block;
}

uint32	MMU::find_free_blocks(uint32 from, uint32 blocks)
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

uint32 MMU::alloc_virtual_space(uint32& size)
{
	uint32 blocks = MMU_SIZE_TO_BLOCKS(size);
	
	uint32 start_block = find_free_blocks(m_next_free_block, blocks);
	if (start_block ==0) start_block = find_free_blocks(m_start_block, blocks);
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

bool		MMU::free_virtual_space(uint32 start_address, uint32 size)
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

uint32	MMU::alloc_virtual_memory(uint32 &size, uint32 protect)
{
	uint32 virtual_addr = alloc_virtual_space(size);
	if (virtual_addr == 0) return 0;
	uint32 pages = size >> 12;
	uint32 phys_addr = PAGE_FRAME_DB::alloc_physical_pages(pages);
	return PAGER::map_pages(phys_addr, virtual_addr, size, protect);
}

bool		MMU::free_virtual_memory(uint32 start_address, uint32 size)
{
	free_virtual_space(start_address, size);
	PAGER::unmap_pages(start_address, size);
	return true;
}