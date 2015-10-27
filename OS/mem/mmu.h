#pragma once
#include "page_frame.h"

#define  PAGE_DIR	_BASE		0xC0300000
#define  PAGE_TABLE_BASE		0xC0000000
#define  MMU_BLOCK_SIZE		KB(64)
#define  MMU_BLOCK_MASK		0xffff0000
#define  MMU_DB_SIZE			(GB(2)/KB(64))

#define  MMU_BLOCK_FREE		0
#define  MMU_BLOCK_USED		1

#define  MMU_SIZE_TO_BLOCKS(size) ((size +   MMU_BLOCK_SIZE -1 )>>16)
#define  MMU_BLOCKS_TO_SIZE(block) (block <<16) 

class MMU
{
private:
	uint32 m_start_block;
	uint32 m_end_block;
	uint32 m_next_free_block;
	byte   m_block_map[MMU_DB_SIZE];

private:
	uint32	find_free_blocks(uint32 from, uint32 blocks);
public:
	MMU();
	~MMU();
	void		Init(uint32 from, uint32 to);
	uint32	alloc_virtual_space(uint32 &size);
	bool		free_virtual_space(uint32 start_address, uint32 size);

	uint32	alloc_virtual_memory(uint32 &size, uint32 protect);
	bool		free_virtual_memory(uint32 start_address, uint32 size);
	
};

