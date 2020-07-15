
#include "terminal.h"
volatile int enter;
volatile int add;
#define NO_R 25
#define NO_T 3
#define NO_C 80
#define BUF_SIZE 128
int len[NO_T];
int input_len;
volatile int cur_row[NO_T];
volatile int cur_col[NO_T];
volatile int cur_terminal;
unsigned char buf[NO_T][BUF_SIZE];
volatile int enter_state;
char screen[NO_T][NO_R][NO_C];
/*
 * terminal_read
 *   DESCRIPTION: copies keyboard buffer when enter is hit
 *   INPUTS: none
 *   OUTPUTS: return lenght of buffer read
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int terminal_read(unsigned char * input){
  int i;
  // wait for the enter to be hit
  while(1){
    // make a mini state machine with enter_state to make sure that
    // we process enter once
    if(enter && enter_state){
      // go to new line whenever enter is pressed
      putc('\n');
      // update the screen buffer
      screen[cur_terminal][cur_row[cur_terminal]][cur_col[cur_terminal]]='\n';
      cur_row[cur_terminal]++;
      cur_col[cur_terminal] = 0;
      // check if we need to scroll
      if(cur_row[cur_terminal]==NO_R){
        scroll();
      }
      // copy the buffer
      for(i=0;i<len[cur_terminal];i++)
        input[i]=buf[cur_terminal][i];
      // copy the length
      input_len = len[cur_terminal];
      // put newline at the end of what was read
      if(input_len == BUF_SIZE){
        input[BUF_SIZE-1] = '\n';
      }
      else{
        input[input_len++]= '\n';
      }
      // reset the input buffer to NULL and reinit the state vars
      memset(buf[cur_terminal],0,BUF_SIZE);
      len[cur_terminal] = 0;
      enter_state = 0;
      return input_len;
    }
  }
}
/*
 * terminal_write
 *   DESCRIPTION: Writes to the cur terminals
 *   INPUTS: input - the buffer to write, nbytes - number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes to video memory
 */
void terminal_write(unsigned char * input,int num_to_write){
  int i;
  for(i = 0 ; i<num_to_write; i++ ){
    // check for end of the current line
    if(cur_col[cur_terminal] == NO_C){
    // check if we will ahve to go to row no 26 if we do new line
      putc('\n');
      screen[cur_terminal][cur_row[cur_terminal]][cur_col[cur_terminal]]='\n';
      cur_row[cur_terminal]++;
      cur_col[cur_terminal] = 0;
      if(cur_row[cur_terminal] == NO_R)
          scroll();
    }
    // if a in same line then just put it and update cur_col[cur_terminal]
    putc(input[i]);
    screen[cur_terminal][cur_row[cur_terminal]][cur_col[cur_terminal]] = input[i];
    cur_col[cur_terminal]++;
    // if the thing just put in is a new line then we move to next row and reset col
    if(input[i]=='\n'){
      cur_row[cur_terminal]++;
      cur_col[cur_terminal] = 0;
    }
    if(cur_row[cur_terminal]==NO_R){
      scroll();
    }
  }
}
/*
 * desplay screen
 *   DESCRIPTION: Redraws screen with current terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes to video memory
 */
void display_screen(){
  int i, j;
  clear();
  for(i=0;i<NO_R;i++)
    for(j=0;j<strlen(screen[cur_terminal][i]);j++)
      putc(screen[cur_terminal][i][j]);
}
