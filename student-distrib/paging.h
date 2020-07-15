
#ifndef _PAGING_H
#define _PAGING_H

#include "lib.h"
#include "types.h"

#define ENTERY_NO 1024
#define ALIGNMENT 4096

/* Intialise all paging tables as gloab vars*/
uint32_t page_directory[ENTERY_NO] __attribute__((aligned(ALIGNMENT)));
uint32_t page_table_one[ENTERY_NO] __attribute__ ((aligned(ALIGNMENT)));
uint32_t page_table_video[ENTERY_NO] __attribute__ ((aligned(ALIGNMENT)));
void init_paging();
extern void load_paging(uint32_t ptr);
extern void enable_paging();
void add_pde(unsigned long virtual_addr, unsigned long phys_addr);
int32_t set_vidmap();
#endif
