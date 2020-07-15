#include "syscalls.h"
#define USERSPACE_END 138412032
#define USERSPACE_START 134217728
#define NEW_L '\n'
#define SPACE ' '
#define TAB '\t'
#define ZERO '\0'
#define KERNEL_DS   0x0018
#define KERNEL_BASE 0x800000
#define KERNEL_STACK_OFFEST 0x2000
#define STACK_POINTER 0x8400000
#define MAX_ARGS 64
#define PHYS_OFFSET 0x400000
#define MB128 0x8000000
#define MB8 0x800000
#define MB4 0x400000
#define KB8 0x2000
#define VIR_OFFSET 0x00048000
#define PCB_FIND_MASK 0xFFFFE000
#define COMMAND_LEN 128
#define BUFFER_LEN 6000
#define NUM_TERMINAL 3
#define NUM_FD 8

file_desc_t openfiles[NUM_FD];
volatile int cur_terminal;
volatile int base_pid[NUM_TERMINAL];
uint8_t buffer[BUFFER_LEN];
inode_t inode;
volatile int glb_flag;
boot_block_t boot_block;
int8_t* file_sys_addr;
file_ops_t stdin_ops;
file_ops_t stdout_ops;
long pid_bit = 0;
volatile int active_pid[NUM_TERMINAL];
/*
 * getargs
 *   DESCRIPTION: retrieves given args then stores it for the program to use
 *   INPUTS: buf, nbytes
 *   OUTPUTS: writes directly on buf
 *   RETURN VALUE: none
 *   SIDE EFFECTS: if args longer than nbytes, it returns -1;
 */
int32_t getargs (uint8_t * buf, int32_t nbytes) {
    int i;
    long esp = (long)&i;
    PCB * cur_pcb = (PCB *)(esp & PCB_FIND_MASK );
    // uint8_t* buf = pt->ebx;
    // int nbytes = pt->ecx;
    if(buf == NULL || nbytes< 0 || strlen(cur_pcb->arg1) == 0)
        return -1;
    else{
        for(i=0;i<nbytes && cur_pcb->arg1[i] != NULL ;i++)
            buf[i]=cur_pcb->arg1[i];
    }
    buf[i]=NULL;

    return 0;
}
/*
 * vidmap
 *   DESCRIPTION: gets user pointer, checks null, then sets vidmap
 *   INPUTS: screen_start
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on sucess -1 on faliure
 *   SIDE EFFECTS: Gives user program acess to video memory
 */
int32_t vidmap (uint8_t ** screen_start) {
    if((int)screen_start>USERSPACE_END || (int)screen_start < USERSPACE_START
     ||screen_start==NULL)
        return -1;
    *screen_start = (uint8_t *)set_vidmap();
    return 0;
}


/*
 * open
 *   DESCRIPTION: opens file
 *   INPUTS: filename buf
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on faliure 0 on sucess
 *   SIDE EFFECTS:
 */
int32_t open (const uint8_t* filename) {
    if (filename == NULL || strlen((char *)filename) == 0){
        return -1;
    }
    int i;
    long esp = (long)&i;
    // PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK )- 4);
    PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK ));
    uint32_t a = do_open(filename, cur_pcb->fd_arr);
    return a;
}
/*
 * close
 *   DESCRIPTION: closes file
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on sucess -1 on faliure
 *   SIDE EFFECTS:
 */
int32_t close (int32_t fd) {
    if(fd <0 || fd >NUM_FD-1){
        return -1;
    }

    int i;
    long esp = (long)&i;
    // PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK )- 4);
    PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK ));
    if((cur_pcb->fd_arr)[fd].flags == 0){
        return -1;
    }
    return (int32_t)((cur_pcb->fd_arr)[fd].ops.file_close)(fd, cur_pcb->fd_arr);
}
/*
 * read
 *   DESCRIPTION: read file
 *   INPUTS: pt_reg pointer
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on sucess, for rtc waits for interrupt then ret 0
 *   SIDE EFFECTS:
 */
int32_t read (int32_t fd , void* buf, int32_t nbytes) {
    if (buf == NULL || fd <0 || fd >NUM_FD-1 || nbytes < 0){
        return -1;
    }
    int i;
    long esp = (long)&i;
    // PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK )- 4);
    PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK ));
    if((cur_pcb->fd_arr)[fd].flags == 0){
        return -1;
    }
    return (int32_t)((cur_pcb->fd_arr)[fd].ops.file_read)(fd, (uint8_t *)buf, nbytes, cur_pcb->fd_arr);
}
/*
 * write
 *   DESCRIPTION: writes file
 *   INPUTS: fd file descripter, buf, nbytes
 *   OUTPUTS: none
 *   RETURN VALUE:  -1 for all but rtc and stdout
 *   SIDE EFFECTS: none, only for rtc changes frequency and for stdout prints on screen
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL || fd <0 || fd >NUM_FD-1 || nbytes < 0 || strlen(buf) == 0){
        return -1;
    }

    int i;
    long esp = (long)&i;
    // PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK )- 4);
    PCB * cur_pcb =    (PCB * )((esp & PCB_FIND_MASK ));

    if((cur_pcb->fd_arr)[fd].flags == 0){
        return -1;
    }

    return (int32_t)((cur_pcb->fd_arr)[fd].ops.file_write)(fd, buf, nbytes, cur_pcb->fd_arr);
}


/*
 * halt
 *   DESCRIPTION: stops process
 *   INPUTS: status
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int32_t halt (uint8_t status) {
    int parent_pid;
    long esp = (long) &parent_pid;
    // get the current and parent PCB
    PCB* current_pcb_ptr = (PCB*)(esp & PCB_FIND_MASK );
    if(current_pcb_ptr->pid==0 || active_pid[cur_terminal]==base_pid[cur_terminal]){
        // do shit for the first shell
        free_pid(current_pcb_ptr->pid);
        char buf[6]={'s','h','e','l','l'};
        glb_flag=1;
        execute((uint8_t*)buf);

    }
    else{
        PCB * parent = current_pcb_ptr->parent;
        // Restore TSS to parent's stuff
        tss.esp0 = parent->kernel_stack_base;
        tss.ss0 = parent->ss0_orig;
        // calculte the new phys address and call add_pde on that
        add_pde(MB128, MB8 + (PHYS_OFFSET * (parent->pid)));
        // do cleanup
        free_pid(current_pcb_ptr->pid);
        clear_fd(current_pcb_ptr->fd_arr);
        if(active_pid[cur_terminal]!=base_pid[cur_terminal])
            active_pid[cur_terminal] = parent->pid;

        // restore esp,ebp and put return value
        asm volatile(
            "xorl %%eax, %%eax      \n\t"
            "movl %0, %%esp         \n\t"
            "movl %1, %%ebp         \n\t"
            "mov %2, %%al           \n\t"
            "jmp jmp_halt           \n\t"
            :
            :"r"(current_pcb_ptr->esp_orig),"r"(current_pcb_ptr->ebp_orig),"r"(status)
            );
    }
    // program never really reaches this
    return 0;
}
/*
 * execute
 *   DESCRIPTION: executes file
 *   INPUTS: command (array with filename and args)
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int32_t execute (const uint8_t* command) {
    char filename[COMMAND_LEN];
    int ret;
    int firs_ins = 0;
    int file_size;
    PCB cur_pcb;
    int cur_pid = get_next_pid();
    if (command == NULL || strlen((char *)command) == 0){
        return -1;
    }

    parse_filename((char*)command, (char*)filename, &cur_pcb);
    if (copy_file_to_buf((uint8_t *)filename, &firs_ins, cur_pid, &file_size)) {
        return -1;
    }
    set_paging(cur_pid, file_size);
    firs_ins = get_firs_ins(buffer);
    cur_pcb.pid=cur_pid;
    fill_pcb(&cur_pcb,(uint8_t *)command,filename);
    // PCB* new_pcb_addr = (PCB*)(cur_pcb.kernel_stack_base - KERNEL_STACK_OFFEST+4);
    PCB* new_pcb_addr = (PCB*)(cur_pcb.kernel_stack_base - KERNEL_STACK_OFFEST);

    if(!memcpy(new_pcb_addr, &cur_pcb, sizeof(PCB))){
        printf("memcpy Failed \n");
    }

    // save the tss information
    tss.ss0 = KERNEL_DS;
    tss.esp0 = cur_pcb.kernel_stack_base;
    new_pcb_addr->esp0_orig= tss.esp0;
    new_pcb_addr->ss0_orig=tss.ss0;
    // int push1=STACK_POINTER - (cur_pcb.pid)*KERNEL_STACK_OFFEST - 4; // put over here the location of the stack user level
    int push1=STACK_POINTER - (cur_pcb.pid)*KERNEL_STACK_OFFEST; // put over here the location of the stack user level

    int push2=firs_ins; // Push over here location of the 1st instruction
    asm volatile(
        "movl %%esp, %0         \n\t"
        "movl %%ebp, %1         \n\t"
        :"=r"(new_pcb_addr->esp_orig), "=r"(new_pcb_addr->ebp_orig)
        );
    asm volatile(
        "pushl $0x02B           \n\t"   // pushinf the data segment
        "pushl %0               \n\t"   // pushing the stack ponter
        "pushfl                 \n\t"   // pushing flags
        "pop %%eax              \n\t"   // pop it back into a register
        "orl $0x200, %%eax      \n\t"   // or it to set the IF flag
        "pushl %%eax            \n\t"   // push it back to the stack
        "pushl $0x23            \n\t"   // push the code segment
        "pushl %1               \n\t"   // push the entry point of the file Basically pushing into the new eip after we go to the push 1 address in mem
        "iret                   \n\t"
        // "jmp_halt:              \n\t"
        :
        :"r"(push1),"r"(push2) // edx ,ecx
        :"eax"
        );
    asm volatile(
        "jmp_halt:              \n\t"
        "movl %%eax, %0         \n\t"
        :"=r"(ret)
        :
        :"eax"
        );
    return ret;
}

/*
 * fd_init
 *   DESCRIPTION: initializes file descripter array
 *   INPUTS: file_desc_t*, pointer to file_desc array
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
void fd_init(file_desc_t * fd_arr) {
    // set whole array to 0
    memset(fd_arr, 0, (NUM_FD)*sizeof(file_desc_t));

    file_desc_t newfile;

    newfile.inode_num = 0;
    newfile.flags = 1;
    newfile.file_pos = 0;

    newfile.ops = stdin_ops;
    fd_arr[0] = newfile;
    newfile.ops = stdout_ops;
    fd_arr[1] = newfile;
}
/*
 * clear fd
 *   DESCRIPTION: clears file descripter array
 *   INPUTS: file_desc_t*, pointer to file_desc array
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
void clear_fd(file_desc_t * fd_arr) {
    memset(fd_arr, 0, sizeof(fd_arr));
}
/*
 * get_next_pid
 *   DESCRIPTION: retrieves next open pid
 *   INPUTS: none
 *   OUTPUTS: next unused pid
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int get_next_pid (){
    int i = 0;
    for(i= 0 ; i<MAX_ARGS; i ++){
        if(!((pid_bit >> i)&1)){
            pid_bit |= 1<<i;
            return i;
        }
    }
    return -1;
}
/*
 * free_pid
 *   DESCRIPTION: frees the pid
 *   INPUTS: pid
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
void free_pid (int pid){
    int pid_left_half = (int)pid_bit & ((1 << pid)-1);
    pid_bit = pid_bit & ((0xFE << pid) | pid_left_half);
}


/*
 * fill_pcb
 *   DESCRIPTION: fills given pcb
 *   INPUTS: pcb pointer, string command and filename
 *   OUTPUTS:
 *   RETURN VALUE: 0 on success, -1 on fail
 *   SIDE EFFECTS:
 */
int fill_pcb (PCB* pcb, uint8_t * command, char *filename){
	// basically check if this is the execute called on a base shell
    // then set all the falgs appropriately so that we can handle it's exit
    if(glb_flag==1){
	pcb->base_shell=1;
	glb_flag=-1;
   	base_pid[cur_terminal]=pcb->pid;
	active_pid[cur_terminal]=pcb->pid;
    }
    else{
	pcb->keyboard_esp=-1;
	pcb->keyboard_ebp=-1;
	pcb->base_shell=0;
    }


    memcpy(pcb->filename, filename, strlen(filename));
    memcpy(pcb->command, command, strlen((char *)command));
    fd_init(pcb->fd_arr);
    int status;
    long esp = (long)&status;
    if(pcb->pid==0){
        pcb->parent=(PCB*) -1;  // init task
    }
    else{
	PCB* current_pcb_ptr = (PCB*)(esp & PCB_FIND_MASK );
	pcb->parent = current_pcb_ptr;
	current_pcb_ptr->child=pcb;

    // task has a parent and is not a base shell and is then the active task on that term for now
   	if(pcb->base_shell==0){
		active_pid[cur_terminal]=pcb->pid;
	}

    }
    // fill with garbage currently
    pcb->esp0_orig = tss.esp0;
    pcb->ss0_orig = tss.ss0;
//     pcb.gp_regs = ?;////////////////////
    // pcb->kernel_stack_base = KERNEL_BASE - (pcb->pid) * KERNEL_STACK_OFFEST - 4; // -4 to skip last 4 byte
    pcb->kernel_stack_base = KERNEL_BASE - (pcb->pid) * KERNEL_STACK_OFFEST; // -4 to skip last 4 byte
    return 0;
}

/*
 * parse_args
 *   DESCRIPTION: parses a string into list of args
 *   INPUTS: string args, array for args
 *   OUTPUTS: directly writes in the array
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */

char* parse_filename(char* arr,char* filename, PCB* pcb){
    int i,j, k;
    for(i=0; (arr[i]!=' ') && (arr[i] != NULL) &&(arr[i] != '\0') ;i++)
        filename[i]=arr[i];
    filename[i]=NULL;
    if(arr[i+1] == '\0'){
        pcb->arg1[0] = NULL;
        goto parse_end;
    }
    filename[i]=NULL;
    for(j=i+1;arr[j]!=' ' && arr[j] != NULL && arr[j] != '\0';j++){
        pcb->arg1[j - i - 1] = arr[j];
    }
    pcb->arg1[j - i - 1] = NULL;
    if(arr[j] == '\0'){
        goto parse_end;
    }
    for(k=j+1;arr[k]!=' ' && arr[k] != NULL && arr[k] != '\0';j++){
        pcb->arg2[k - j - 1] = arr[k];
    }
    pcb->arg2[k - j - 1] = NULL;

    parse_end:
    return filename;
}

/*
 * copy_file_to_buf
 *   DESCRIPTION: copies file to given buffer
 *   INPUTS: filename, firs_ins_ptr, cur_pid, file_size_ptr
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int32_t copy_file_to_buf (uint8_t * filename, int * firs_ins_ptr, int cur_pid, int* file_size_ptr){
    int i = 0;
    while (i < boot_block.num_dir) {
        if (!strncmp((int8_t*)filename, (int8_t*)boot_block.d_entries[i].filename, strlen((int8_t*)filename))) {
            break;
        }
        i++;
    }

    // If file not found, open failed, return -1
    if (i == boot_block.num_dir)
        return -1;

    int8_t* inode_addr = file_sys_addr + BLOCK_SIZE * (boot_block.d_entries[i].inode_num + 1);


    memcpy(&inode, inode_addr, BLOCK_SIZE);

    int32_t file_size = inode.length;
    *file_size_ptr = file_size;
    int32_t read = 0, to_read, block_idx;

    // loop to go to each data block of our file and memcpy the data there to buf
    do {
        block_idx = read / BLOCK_SIZE;
        to_read = min(file_size - read, BLOCK_SIZE);
        if (memcpy(buffer + read, file_sys_addr + (boot_block.num_inode
            + 1 + inode.data[block_idx])*BLOCK_SIZE, to_read)) {
            read += to_read;
        }
        else {
            printf("memcpy failed\n");
        }
    } while (read < file_size);

        if (check_exec(buffer)) {
            return -1;
        }
    return 0;
}





/*
 * set_paging
 *   DESCRIPTION: takes in info of current processer, adds paging and copies file to the new page
 *   INPUTS: string filename, first instruction pointer, current pid
 *   OUTPUTS:
 *   RETURN VALUE: 0 on success, -1 on fail
 *   SIDE EFFECTS:
 */
int32_t set_paging (int cur_pid, int file_size) {

    // calculate virtual and physical addresses and  then add the paging directory
    int vir_addr, phys_addr;
    if(cur_pid < 0 ){
        return -1;
    }
    vir_addr = MB128;
    phys_addr = MB8 + (PHYS_OFFSET * cur_pid);
    add_pde(vir_addr, phys_addr);


    memcpy((void*)MB128 + VIR_OFFSET, buffer, file_size);


    return 0;
}
/* get_firs_ins
input: buffer holding the whole file
retval = first instruction addr
*/

int get_firs_ins(uint8_t* buffer){
    int first_instruction = 0x0;


    first_instruction=first_instruction|buffer[BIT_NUM_27]<<BYTE_SHIFT_3;
    first_instruction=first_instruction|buffer[BIT_NUM_26]<<BYTE_SHIFT_2;
    first_instruction=first_instruction|buffer[BIT_NUM_25]<<BYTE_SHIFT_1;
    first_instruction=first_instruction|buffer[BIT_NUM_24];
    return first_instruction;
}

/*
 * check_exec
 *   DESCRIPTION: checks if file is executable
 *   INPUTS: buffer holding file data
 *   OUTPUTS:
 *   RETURN VALUE: 0 on executable, -1 on nons
 *   SIDE EFFECTS:
 */
int32_t check_exec(void * buf) {

    uint8_t magic_num[4];
    magic_num[0] = EXEC_M_NUM_0;
    magic_num[1] = EXEC_M_NUM_1;
    magic_num[2] = EXEC_M_NUM_2;
    magic_num[3] = EXEC_M_NUM_3;
    if (strncmp(buf, (char*)magic_num, 4))
        return -1;
    return 0;
}
/*
 * set_handler
 *   DESCRIPTION: looks like we have a lot of functions
 *   INPUTS: signum, handler_address
 *   OUTPUTS:
 *   RETURN VALUE: 0 on executable
 *   SIDE EFFECTS:
 */
int32_t set_handler (int32_t signum, void* handler_address) {
    return 0;
}
/*
 * sigreturn
 *   DESCRIPTION: NULL
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t sigreturn (void) {
    return 0;
}
/*
 * pid_from_esp
 *   DESCRIPTION: Returns pid based on the esp
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the pid of the current process
 *   SIDE EFFECTS: none
 */
int32_t pid_from_esp(void){
    int i;
    long esp = (long)&i;
    PCB * new_pcb = (PCB *)(esp & PCB_FIND_MASK );
    return new_pcb->pid;
}
