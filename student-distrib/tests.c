#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

// #include <stdint>

// #include "../syscalls/ece391support.h"
// #include "../syscalls/ece391syscall.h"

#define SBUFSIZE 33

#define PASS 1
#define FAIL 0
extern void dir_open_test();
/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
// int idt_test(){
// 	TEST_HEADER;

// 	int i;
// 	int result = PASS;
// 	// checking first ten entries
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) &&
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}
// 	return result;
// }


// // file to check how the IDT works
// int idt_check(){
// 	// divide by zero error
// 	// int b = 2/0;

// 	// page fault
// 	// uint32_t * ptr = NULL;
// 	// *ptr;
// 	return PASS;
// }
// // add more tests here




// // he,lper function to print whenever rtc read returns and thus print
// // when an interupt occurs
// void rtc_printing(int k){
// 	int j = 0;
// 	// Print k number of times
// 	while(j<k){
// 		// wait for read to return
// 		if(! rtc_read()){
// 			// print to show interrupt was raised
// 			printf("!");
// 			j++;
// 		}
// 	}
// }
// // Test function to test the rtc_read and rtc_write functionality
// int read_write_test(){
// 	rtc_open();
// 	// frequencies to test
// 	// initlise all 15 possible freqs in an arrAy
// 	int frequncies_1[15] = {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
// 	int i = 0;
// 	// loop through frequencies as needed
// 	for(i=0 ; i < 15; i++){
// 		printf("Trying %d\n",frequncies_1[i]);
// 		// change the frequency
// 		rtc_write(frequncies_1[i]);
// 		// print in that frequency for 5 iters
// 		rtc_printing(5);
// 		clear();
// 	}
// 	return PASS;
// }

// // test to test bad value for rtc write
// // use rtc open to inin the pic and set to 2hz
// int bad_value_and_open_test(){
// 	// open the rtc using rtc open
// 	printf("Openinging RTC \n");
// 	rtc_open();
// 	// print to show freq for 10 iters
// 	rtc_printing(10);
// 	printf("\n Trying bad value \n");
// 	// try bad value
// 	int ret = rtc_write(2132);
// 	printf("\nThe returned value is %d \n",ret);
// 	// show freq has not changed
// 	rtc_printing(10);
// 	printf("\n");
// 	return PASS;
// }
/* Checkpoint 3 tests */

// int syscall_test ()
// {
//     int32_t fd, cnt;
//     uint8_t buf[SBUFSIZE];

//     if (-1 == (fd = system_open ((uint8_t*)"."))) {
//         printf (1, (uint8_t*)"directory open failed\n");
//         return 2;
//     }

//     while (0 != (cnt = system_read (fd, buf, SBUFSIZE-1))) {
//         if (-1 == cnt) {
// 	        printf (1, (uint8_t*)"directory entry read failed\n");
// 	        return 3;
// 	    }
// 	    buf[cnt] = '\n';
// 	    if (-1 == system_write (1, buf, cnt + 1))
// 	        return 3;
//     }

//     printf("%s\n", buf);

//     return 0;
// }

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){

	// TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here

	// TEST_OUTPUT("idt_divide_by_zero",idt_check());

	// CHECKPOINT 2 TESTS
	// File tests

	// clear();
	// TEST_OUTPUT("File Open", file_open_test_launch());
	// TEST_OUTPUT("Dir Open test", syscall_test());

	// RTC tests

	// clear();
	// TEST_OUTPUT("RTC RW TEST",read_write_test());
	// TEST_OUTPUT("RTC BAD VALUE AND OPEN TEST", bad_value_and_open_test())

	// ece391_open(NULL);
	// char a [5]={'l','s',' ','.','\0'};
	// ece391_execute(a,1,1);
	// char buf[128];
	// ece391_read(0,buf);
	// ece391_write(1,buf);
	// char buf[6]={'s','h','e','l','l'};
	// ece391_execute(buf);
	// char read_buf[26];
	// int fd = ece391_open(buf);
	// ece391_read(fd,read_buf,26);
	// printf("%s\n",read_buf );
	// ece391_close(fd);
	// fd = ece391_open(buf);
	// printf("%d\n",fd );
}
