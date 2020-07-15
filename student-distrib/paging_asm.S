#define ASM 1

.text

.globl load_paging,enable_paging

// Taken from OSDev

/*
 * load_paging
 *   DESCRIPTION: Sets cr3 to address of the page directory
 *   Convention : C style
 *   INPUTS: Pointer to the page directory
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Setscr3 to address of page directory
 */
load_paging:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    mov %eax, %cr3
    mov %ebp, %esp
    pop %ebp
    ret
/*
 * enable_paging
 *   DESCRIPTION: Enables PSE and Paging
 *   Convention : C style
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables PSE and Paging
 */
enable_paging:
    push %ebp
    mov %esp, %ebp

    // enabling PSE for 4 MiB pages
    movl %cr4, %eax
    // set the 5th bit of cr4 to enable PSE
    or $0x00000010, %eax
    mov %eax, %cr4

    // enabling pages
    movl %cr0, %eax
    // set the 32nd bit of cr0 to enable paging
    or $0x80000000, %eax
    mov %eax, %cr0
    mov %ebp, %esp

    pop %ebp
    ret
