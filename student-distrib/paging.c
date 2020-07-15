#include "paging.h"
#define NOT_PRESENT 0x00000002
#define MASK_1 0x1000
#define VIDEO_OFF 0xB8
#define VIDEO 0xB8000
#define ENTERY_NO 1024
#define ALIGNMENT 4096
#define KERNEL_ADDR 0x400000
#define S_RW_P 0x3
#define PD_MASK 0xFFFFFFE7
#define SRP 0x83
#define SURP 0x87
#define U_RW_P 0x7
#define VID_ADD 8388608
#define MB4 4194304
/*
 * init_paging
 *   DESCRIPTION: Initialises paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets up video memory and kernel memory
 */

void init_paging(){
    int i ;

    // set all pages to Supervisor (kernel only access)
    // Write Enabled (can be written and read)
    // Not Present (no table present yet)
    // Set first page table's pages to same settings to
    for(i = 0; i<ENTERY_NO; i++){
        page_directory[i] = NOT_PRESENT;
        page_table_one[i] = ((i* MASK_1 ) | NOT_PRESENT);
        page_table_video[i] = ((i* MASK_1 ) | NOT_PRESENT);
    }

    page_table_one[VIDEO_OFF]  = (VIDEO | S_RW_P);
    printf("%d\n",page_table_one[VIDEO_OFF]);
    // set first 2 page directpries
    // first PDE broken into 4kb sgments using page_table_one
    // Second PDE mapped to one page of the Kernel at 0x400000
    page_directory[0] = (((unsigned int)page_table_one) | S_RW_P);
    page_directory[1] = (KERNEL_ADDR | SRP);
    page_directory[33] = (((unsigned int)page_table_video)| U_RW_P);
    // calling assembly functions to load values into cr3 cr0 cr4
    load_paging((unsigned int)page_directory & PD_MASK);
    enable_paging();
}
/*
 * add_pde
 *   DESCRIPTION: Initialises paging directory entry
 *   INPUTS: phys_addr- physical addr to be mappd
 *           virtia;_addr- cirtual addr to be mapped to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets up a pde entry to map the addresses inputted
 *                 flushes the TLB
 */
void add_pde(unsigned long virtual_addr, unsigned long phys_addr){
    int temp = ((virtual_addr)/MB4);
    page_directory[temp] = phys_addr | SURP;

    // flushing the TLB
    asm volatile(
        "movl %%cr3, %%ecx      \n\t"
        "movl %%ecx, %%cr3      \n\t"
        :
        :
        :"ecx"
        );

}
/*
 * set_vidmap
 *   DESCRIPTION: Sets the vidmpa for user program
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: new address video mem is mapped to
 *   SIDE EFFECTS: Sets up video memory and kernel memory
 */
int32_t set_vidmap(){
    page_table_video[VIDEO_OFF]=VIDEO | U_RW_P;
    return 0x84b8000;
}
