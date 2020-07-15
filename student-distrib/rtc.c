#include "rtc.h"
#include "i8259.h"
#define REG_A 0x8A
#define REG_B 0x8B
#define REG_C 0x0C
#define SEL_REG 0x70
#define RW_REG 0x71
#define EN_VAL 0x40
#define RTC_IRQ 8
#define HZ_2 15
volatile int inter;




/*
 * rtc_init
 *   DESCRIPTION: Initialises the rtc enables it and freq to 2 Hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Interrupts raised at IRQ8 at 2Hz
 */

void rtc_init(){

    // Taken from OSDEV
    // int flags;
    // cli_and_save_flags(flags);
    char hold;
    // select register b
    outb(REG_B,SEL_REG);
    // read its value
    hold = inb(RW_REG);
    // select register b
    outb(REG_B,SEL_REG);
    // write to the register with 0x40
    outb(hold|EN_VAL,RW_REG);
    // 15 to make it 2 Hz
    rtc_change_freq(HZ_2);

    enable_irq(RTC_IRQ);
    // sti();
    // restore_flags(flags);+

}

/*
 * rtc_change_freq
 *   DESCRIPTION: Changes freq of the rtc
 *   INPUTS: freq which is the amount the freq is to be left shifted
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes the freq
 */

void rtc_change_freq(uint16_t freq){

    char hold;
    // select reg a
    outb(REG_A,SEL_REG);
    // get value from reg a
    hold = inb(RW_REG);
    // select reg a
    outb(REG_A,SEL_REG);
    // write rate to reg a
    // 0xF0 as we want to write only to top byte
    outb(((hold & 0xF0) | freq), RW_REG);
}


/*
 * rtc_opem
 *   DESCRIPTION: Initialises the rtc enables it and freq to 2 Hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:  0 for success and 1 for faliure
 *   SIDE EFFECTS: Interrupts raised at IRQ8 at 2Hz
 */

int rtc_open(){
     rtc_init();
     rtc_change_freq(HZ_2); // add variable for chanign the frequency
     return 0;
}


/*
 * rtc_close
 *   DESCRIPTION: Does Nothing
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:  0 e
 *   SIDE EFFECTS: Does Nothing
 */

int rtc_close(){
    return 0;
}

/*
 * rtc_read
 *   DESCRIPTION: Waits for interrupt to occur
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:  Returns 0 when interrupt occurs
 *   SIDE EFFECTS: Blocks execution till next RTC interrupt
 */


int rtc_read(){
// Wait for RTC interrupt and then return
// use volatile flag in handler
    while(1){
        if(inter){
            inter = 0;
            return 0;
        }
    }
    return -1;
}

/*
 * rtc_init
 *   DESCRIPTION: Changes the frequency of the RTC interrupt
 *   INPUTS: freq
 *   OUTPUTS: none
 *   RETURN VALUE:  0 for success and 1 for faliure
 *   SIDE EFFECTS: Changes Freq of the RTC
 */

int rtc_write(int freq){

    int i =0;
// check if the inmput into rtc read is a power of 2
// check if it is a valid freq
// if so then call rtc change freq
// and then return 0
// else return -1
    if(freq==NULL){
        printf("Null freq passed in \n");
        return -1;
    }
    // init all 15 possible frequences as powers of 2
    int frequncies_1[15] = {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
    // loop through 15 times to check against all possible values
    for( i=0; i<16 ; i++ ){
        if(frequncies_1[i] == freq){
            // 15-1 asfrom the right shift formula for freq and rate
            // taken from osdev
            rtc_change_freq(15-i);
            return 0;
        }
    }
    // if none of them in the array are equal to the input
    printf("Invalid Frequncyt passed in \n");
    return -1;
}
