#ifndef _RTC_H
#define _RTC_H
#include "lib.h"
// all funcitons to be used

// init funct
void rtc_init();
// init and set freq to 2 hz
int rtc_open();
// does nothing for now
int rtc_close();
// blocks till next interrupt has ocurred
int rtc_read();
// changes the freq of the rtc
int rtc_write(int freq);
// helper funciton to change the rtc freq
void rtc_change_freq(uint16_t freq);

// volatile flag to signify when the interrupt has ocurred
extern volatile int inter;
#endif
