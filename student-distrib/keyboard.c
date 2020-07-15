#include "keyboard.h"
#include "lib.h"
#include "terminal.h"
#include "syscalls.h"
#define CAPS 58
#define LSHIFT 42
#define RSHIFT 54
#define CAPS_R -70
#define LSHIFT_R -86
#define RSHIFT_R -74
#define CTRL 29
#define CTRL_R -99
#define BCKSP 14
#define BCKSP_R -114
#define ENTER_R -100
#define L 38
#define M 37
#define TRUE 1
#define FALSE 0
#define ENTER 28
#define BUF_SIZE 128
#define NO_ROWS 25
#define NO_COLS 80
#define F1 59
#define F2 60
#define ALT 56
#define ALT_R -72
#define MB8 0x800000
#define VIR_OFFSET 0x00048000
#define PHYS_OFFSET 0x400000
#define MB128 0x8000000
#define PID_OFF 0x2000
#define NUM_SCANCODES 55
#define NUM_TERMINALS 3

// Scancode arrays
char arr_n [NUM_SCANCODES] = {'~', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '~', '~', 'q', 'w', 'e',
 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '~','~', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '~',
  '`', '~', '~', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '~'};
char arr_c [NUM_SCANCODES] = {'~', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '~', '~', 'Q', 'W', 'E',
 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '~','~', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '~',
  '`', '~', '~', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '~'};
char arr_ls [NUM_SCANCODES] = {'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '~', '~', 'Q', 'W', 'E',
 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '~','~', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
  '~', '~', '~', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '~'};
char arr_rs [NUM_SCANCODES] ={'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '~', '~', 'Q', 'W', 'E',
 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '~','~', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
  '~', '~', '~', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '~'};
char arr_cs [NUM_SCANCODES] = {'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '~', '~', 'q', 'w', 'e',
 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '~','~', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"',
  '~', '~', '~', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '~'};

// init all global flags
volatile int active_pid[NUM_TERMINALS];
volatile int alt,caps,lshift,rshift,ctrl,print,backspace,backspace_state;
volatile int cur_row[NUM_TERMINALS];
volatile int cur_col[NUM_TERMINALS];
volatile int cur_terminal;
volatile int add;
volatile int base_pid[NUM_TERMINALS];
volatile int enter;
volatile int enter_state;
int len[NUM_TERMINALS];
int flag=1;
char screen[NUM_TERMINALS][NO_ROWS][NO_COLS];
char screen_buf[NUM_TERMINALS][NO_ROWS][NO_COLS];
int pos[NO_ROWS];
int positions[2];
unsigned char buf[NUM_TERMINALS][BUF_SIZE];
/*
 * keyboard_init
 *   DESCRIPTION: Initialises the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Interrupts raised at IRQ 1
 */

int32_t set_page (int cur_pid) {

    // calculate virtual and physical addresses and  then add the paging directory
    int vir_addr, phys_addr;
    if(cur_pid < 0 ){
        return -1;
    }
    vir_addr = MB128;
    phys_addr = MB8 + (PHYS_OFFSET * cur_pid);
    add_pde(vir_addr, phys_addr);
    return 0;
}

void keyboard_init(){
    cur_terminal=0;
    disable_irq(1);
// init all global flags
    int i = 0;
    int j =0;
    for(i=0;i<NO_ROWS;i++){
        pos[i]=0;
        for(j=0;j<NO_COLS;j++){
            screen[cur_terminal][i][j]=NULL;
        }
    }
    caps = 0;
    lshift =0;
    rshift = 0;
    ctrl = 0;
    backspace=0;
    backspace_state=0;
    for(i=0;i<NUM_TERMINALS;i++){
    	cur_row[i]=0;
    	cur_col[i]=0;
    	base_pid[i]=-1;
    	len[i]=0;
    	active_pid[i]=-1;
    }
    base_pid[0]=0;
    active_pid[0]=0;
    // len=0
    alt=0;
// start interrupts for keyboard now
    enable_irq(1);
}


/*
 * keyboard_interrupt_handler
 *   DESCRIPTION: prints the key that was pressed
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends EOI to driver and prints char to screen, handles changing
 *                 terminals
 */
void keyboard_interrupt_handler(){
    char a;
    a = inb(0x60);
    // setup all the flags based on the keycode entered
    switch((int)a){
        case CAPS: caps = caps?0:1; break;
        case LSHIFT: lshift = 1; break;
        case RSHIFT: rshift = 1; break;
        case LSHIFT_R: lshift = 0; break;
        case RSHIFT_R: rshift = 0; break;
        case CTRL: ctrl=1; break;
        case CTRL_R: ctrl=0; break;
        case BCKSP: backspace=1;break;
        case BCKSP_R: {backspace=0;backspace_state=0;} break;
        case ENTER_R: enter_state=1; break;
        case ALT: alt=1; break;
        case ALT_R: alt=0; break;
    }

    int i,j,orig,flag2=1,flag3=1;
    char cur;
    if(alt){
        if(a==F1){
            flag2=0;
            // save cur terminal no and switch it
            orig = cur_terminal;
            if(cur_terminal!=0)
                cur_terminal-=1;
            else
                cur_terminal=2;
            // display new screen
            display_screen();
            PCB *cur_pcb = (PCB *)get_pcb_from_pid(active_pid[orig]);
            // check if shell has been init or not
            if(base_pid[cur_terminal]==-1){
                // save the details about this shell to return here
                // then call execute on the next shell
                asm volatile(
                "movl %%esp, %0         \n\t"
                "movl %%ebp, %1         \n\t"
                :"=r"(cur_pcb->keyboard_esp), "=r"(cur_pcb->keyboard_ebp)
                );
                // Send eoi to keyboard and set glb flag so that we know next
                // execute is of a base shell to assign base pid
                enable_irq(1);
                send_eoi(1);
                glb_flag=1;
                // execute the new shell
        		execute((uint8_t *)"shell");
    	    }
    	    else{
                // save the details about this shell to return here
                // then call execute on the next shell
                asm volatile(
                "movl %%esp, %0         \n\t"
                "movl %%ebp, %1         \n\t"
                :"=r"(cur_pcb->keyboard_esp), "=r"(cur_pcb->keyboard_ebp)
                );
        		PCB * pcb_ptr = (PCB *)get_pcb_from_pid(active_pid[cur_terminal]);
        		asm volatile(
        		"movl %0, %%esp			\n\t"
        		"movl %1, %%ebp			\n\t"
        		:
        		:"r"(pcb_ptr->keyboard_esp),"r"(pcb_ptr->keyboard_ebp)
        		);
                // restore the tss and paging structure of the prev terminal
                set_page(active_pid[cur_terminal]);
                pcb_ptr = (PCB *)get_pcb_from_pid(active_pid[cur_terminal]);
                tss.esp0 = pcb_ptr->esp0_orig;
                tss.ss0  = pcb_ptr->ss0_orig;
                enable_irq(1);
                send_eoi(1);
    	    }
        }
        else if(a==F2){
            flag2=0;
            orig = cur_terminal;
            if(cur_terminal!=2){
                cur_terminal+=1;
            }
            else
                cur_terminal=0;
            // figure out where to context switch
            // context_switch();
            display_screen();
            PCB *cur_pcb = (PCB *)get_pcb_from_pid(active_pid[orig]);
            if(base_pid[cur_terminal]==-1){
        		// save the details about this shell to return here
        		// then call execute on the next shell
        		asm volatile(
                "movl %%esp, %0         \n\t"
                "movl %%ebp, %1         \n\t"
                :"=r"(cur_pcb->keyboard_esp), "=r"(cur_pcb->keyboard_ebp)
                );
                // Send eoi to keyboard and set glb flag so that we know next
                // execute is of a base shell to assign base pid
        		glb_flag=1;
                enable_irq(1);
                send_eoi(1);
        		execute((uint8_t *)"shell");
    	    }
    	    else{
                // save the details about this shell to return here
                // then call execute on the next shel
                asm volatile(
                "movl %%esp, %0         \n\t"
                "movl %%ebp, %1         \n\t"
                :"=r"(cur_pcb->keyboard_esp), "=r"(cur_pcb->keyboard_ebp)
                );
        		PCB * pcb_ptr = (PCB *)get_pcb_from_pid(active_pid[cur_terminal]);
        		asm volatile(
        		"movl %0, %%esp			\n\t"
        		"movl %1, %%ebp			\n\t"
        		:
        		:"r"(pcb_ptr->keyboard_esp),"r"(pcb_ptr->keyboard_ebp)
        		);
                 // restore the tss and paging structure of the prev terminal
                enable_irq(1);
                pcb_ptr = (PCB *)get_pcb_from_pid(active_pid[cur_terminal]);
                tss.esp0 = pcb_ptr->esp0_orig;
                tss.ss0  = pcb_ptr->ss0_orig;
		        set_page(active_pid[cur_terminal]);
                send_eoi(1);
    	    }
        }
    }
    int cur_val = (int)a;
    if(cur_val < 1 || cur_val>60 )
        add=0;
    else{

        if(ctrl==TRUE && (int)a==L){
            // ctrl + l has been pressed
                clear();
                for(i=0;i<NO_ROWS;i++){
                    for(j=0;j<NO_COLS;j++){
                        screen[cur_terminal][i][j]=NULL;
                    }
                }
                add=0;
                cur_row[cur_terminal]=0;
                cur_col[cur_terminal]=0;
                terminal_write((uint8_t *)(uint8_t*)"391OS> ",7);   // 7 characters in "391OS> "
                flag3=0;
        }
        else if(backspace_state == FALSE && backspace == TRUE && len[cur_terminal]!=0){
            // backspace has been set thus making deleteing the last char
                buf[cur_terminal][len[cur_terminal]-1]=NULL;
                len[cur_terminal]--;
                screen[cur_terminal][(cur_row[cur_terminal])][(cur_col[cur_terminal]-1)]=NULL;
                cur_col[cur_terminal]--;
                backspace_state=1;
                add=0;
                print_backspace();
        }
        else if((int)a== ENTER ){
            enter=1;
        }
        else if((int)a>0){
            if((caps && lshift) || (caps && rshift)){
                // caps + shift
                cur = arr_cs[(int)a -1];
                add=1;

            }
            else if(caps){
                // only caps
                cur = arr_c[(int)a -1];
                add=1;
            }
            else if(rshift){
                // only lshift
                cur = arr_rs[(int)a -1];
                add=1;
            }

            else if(lshift){
                // oly rshift
                cur = arr_ls[(int)a -1];
                add=1;
            }
            if((!caps) && (!lshift) &&(!rshift)){
                // no triggers
                cur = arr_n[(int)a -1];
                add=1;
            }
            if(add==1 && cur!='~' && len[cur_terminal]!=BUF_SIZE &&flag2 && flag3){
                // adding if valid key was pressed
                if((int)a != CAPS && (int)a !=CAPS_R){
                    putc(cur);
                    buf[cur_terminal][len[cur_terminal]]=cur;
                    screen[cur_terminal][ (cur_row[cur_terminal]) ][(cur_col[cur_terminal])++]=cur;
                    len[cur_terminal]++;
                    // check for screen overflow and scrolling and call accordingly
                    if(cur_col[cur_terminal]==NO_COLS){
                        screen[cur_terminal][cur_row[cur_terminal]][cur_col[cur_terminal]]='\n';
                        putc('\n');
                        cur_col[cur_terminal]=0;
                        cur_row[cur_terminal]++;
                        if(cur_row[cur_terminal]==NO_ROWS)
                            scroll();
                    }
                }
            }
        }
    }
    if(flag2){
    enable_irq(1);
    send_eoi(1);
    }

}
/*
 * scroll
 *   DESCRIPTION: scrolls the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Screen scrolling done
 */
void scroll(){
    int i=0;
    int j=0;
    memset(screen_buf[cur_terminal],NULL,sizeof(screen_buf[cur_terminal]));
    for(i=0;i<NO_ROWS-1;i++){
        for(j=0;j<NO_COLS;j++)
            screen_buf[cur_terminal][i][j]=screen[cur_terminal][i+1][j];
    }
    clear();
    int b = 1;
    for(i=0;i<NO_ROWS-1;i++){
        b=1;
        for(j=0;j<NO_COLS;j++){
            putc(screen_buf[cur_terminal][i][j]);
            if(screen_buf[cur_terminal][i][j]=='\n')
                break;
        }
    }
    for(i=0;i<NO_ROWS;i++)
        for(j=0;j<NO_COLS;j++)
            screen[cur_terminal][i][j]=screen_buf[cur_terminal][i][j];
    for(i=0;i<NO_COLS;i++)
        screen[cur_terminal][NO_ROWS-1][i]=NULL;
    cur_row[cur_terminal]=NO_ROWS-1;
}
/*
 * get_pcb_from_pid
 *   DESCRIPTION: scrolls the screen
 *   INPUTS: pid - pid to get pcb of
 *   OUTPUTS: none
 *   RETURN VALUE: a pointer to the corresponding pid
 *   SIDE EFFECTS: Screen scrolling done
 */
int32_t get_pcb_from_pid(int pid) {
    return (MB8 - (pid+1)* PID_OFF);
}
