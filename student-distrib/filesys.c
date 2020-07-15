#include "filesys.h"
#include "terminal.h"
#define MAX_FILENAME_LEN 32

/* Array of files currently opened by the process.
  Maintained here for now, will be maintained in PCB later. */
// extern file_desc_t openfiles[8];

/* The entire boot block. */
boot_block_t boot_block;

// address of the filesystem, externed from kernel.c
int8_t* file_sys_addr;

// rtc ops jump table
file_ops_t rtc_ops = {rtc_file_close, rtc_file_read, rtc_file_write};

// directory ops jump table
file_ops_t dir_ops = {dir_close, dir_read, dir_write};

// file ops jump table
file_ops_t file_ops = {file_close, file_read, file_write};

// stdin ops jump table
file_ops_t stdin_ops = {stdin_close, stdin_read, stdin_write};

// stdout ops jump table
file_ops_t stdout_ops = {stdout_close, stdout_read, stdout_write};


/*
    int32_t file_sys_init()
    Description: Initialises the file system.
    Inputs: none
    Outputs: Stores the boot block from file_sys_img to boot_block.
    Returns: -1
    Side effects: none

*/
int32_t file_sys_init() {
  if (memcpy(&boot_block, file_sys_addr, BLOCK_SIZE))
    return 0;
  return -1;
}


/* int32_t do_open(const uint8_t * filename, file_desc_t * openfiles)
    Description: Opens a file
    Inputs: filename: name of the file to be opened
    Outputs: Returns file descriptor of new file
    Returns: -1
    Side effects: none
*/
int32_t do_open(const uint8_t * filename, file_desc_t * openfiles) {
  int i = 0;
  int32_t filename_len = strlen((int8_t*)filename);

  // Look up the file name in the directory entries in the boot block.
  while (i < boot_block.num_dir) {
    uint8_t * cur_filename = boot_block.d_entries[i].filename;
    int32_t cur_filename_len = strlen((int8_t *)cur_filename);
    if (filename_len == min(cur_filename_len,MAX_FILENAME_LEN) &&
      !strncmp((int8_t*)filename, (int8_t*)cur_filename,
      min(MAX_FILENAME_LEN, max(filename_len, cur_filename_len) ) ) ) {
      break;
    }
    i++;
  }

  // If file not found, open failed, return -1
  if (i == boot_block.num_dir)
    return -1;

  if (boot_block.d_entries[i].filetype == 0) {
    return rtc_file_open(openfiles);
  }

  // if the file is a directory
  if (boot_block.d_entries[i].filetype == 1){
    return dir_open(openfiles);
  }

  // if the file is a file
  if (boot_block.d_entries[i].filetype == 2){
    return file_open(boot_block.d_entries[i].inode_num, openfiles);
  }

  return -1;
}

/* int32_t file_open(int32_t inode_num, file_desc_t * openfiles)
    Description: Creates a new file descriptor
    Inputs: inode_num: inode number of the file being opened
    Outputs: assigns new file descriptor in the openfiles array
    Returns: file descriptor of the file opened
    Side effects: none
*/
int32_t file_open(int32_t inode_num, file_desc_t * openfiles) {
  file_desc_t newfile;

  newfile.ops = file_ops;
  newfile.inode_num = inode_num;
  newfile.flags = 1;
  newfile.file_pos = 0;

  return get_fd(newfile, openfiles);
}

/* int32_t dir_open(file_desc_t * openfiles)
    Description: Creates a new file descriptor for the directory
    Inputs: none
    Outputs: assigns new file descriptor in the openfiles array
    Returns: file descriptor of the directory opened
    Side effects: none
*/
int32_t dir_open(file_desc_t * openfiles) {
  file_desc_t newfile;

  newfile.ops = dir_ops;
  newfile.inode_num = 0;
  newfile.flags = 1;
  newfile.file_pos = 0;

  return get_fd(newfile, openfiles);
}

/* int32_t rtc_file_open(file_desc_t * openfiles)
    Description: Creates a new file descriptor for the RTC file
    Inputs: none
    Outputs: assigns new file descriptor in the openfiles array
    Returns: file descriptor of the RTC file opened
    Side effects: none
*/
int32_t rtc_file_open(file_desc_t * openfiles) {
  rtc_open();
  file_desc_t newfile;

  newfile.ops = rtc_ops;
  newfile.inode_num = 0;
  newfile.flags = 1;
  newfile.file_pos = 0;

  return get_fd(newfile, openfiles);
}

/* int32_t get_fd(file_desc_t newfile, file_desc_t * openfiles)
    Description: Gets file descriptor of next unused file and assigns new file to it
    Inputs: newfile: file desc of new file
    Outputs: none
    Returns: file descriptor of the file opened
    Side effects: assigns new file descriptor in the openfiles array
*/
int32_t get_fd(file_desc_t newfile, file_desc_t * openfiles) {
  int i;

  // Find a file descriptor that is not in use, and put it use for the new file
  for (i = 2; i < NUM_FILES; i++) {
    if (!openfiles[i].flags) {
      openfiles[i] = newfile;
      break;
    }
  }

  // if 8 files are already open, open failed, return -1
  if (i == NUM_FILES)
    return -1;

  // return the file descriptor of the new file
  return i;
}

//////// FILE OPS //////////

/* int32_t file_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Reads from a file.
    Inputs: fd: file descriptor of the file to read
            buf: buffer to store read file data
            nbytes: number of bytes to read
    Outputs: Stores the file data in buf.
    Returns: returns number of bytes read if successful, 0 if unsuccessful
    Side effects: none
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles) {

  file_desc_t file = openfiles[fd];
  if( file.flags == 0 ){
    return -1;
  }
  int8_t* inode_addr = file_sys_addr + BLOCK_SIZE*(file.inode_num + 1);
  inode_t inode;
  memcpy(&inode, inode_addr, BLOCK_SIZE);

  int32_t file_size = inode.length;
  int32_t read = 0, to_read, block_idx, offset;

  if (file.file_pos >= file_size || nbytes == 0)
    return 0;

  // loop to go to each data block of our file and memcpy the data there to buf
  do {
    block_idx = file.file_pos/BLOCK_SIZE;
    offset = file.file_pos%BLOCK_SIZE;
    to_read = min(file_size - file.file_pos, BLOCK_SIZE - offset);
    to_read = min(to_read, nbytes - read);
    if (memcpy(buf + read, file_sys_addr + (boot_block.num_inode + 1 + inode.data[block_idx])*BLOCK_SIZE + offset, to_read)) {
      read += to_read;
      file.file_pos += to_read;
    }
    else {
      printf("memcpy failed\n");
    }
  } while (file.file_pos < file_size && read < nbytes);

  openfiles[fd] = file;
  return read;
}

/* int32_t file_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Writes to a file.
    Inputs: fd: file descriptor of the file to read
            buf: buffer containing the data to write to the file
            nbytes: number of bytes to write
    Outputs: nothing because read-only file system
    Returns: returns 0 if writing to terminal, -1 otherwise
    Side effects: none
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles) {
  if (fd == 1) {
    // terminal_write(buf, nbytes);
    return 0;
  }

  return -1;
}

/* int32_t file_close(int32_t fd, file_desc_t * openfiles)
    Description: Closes an opened file.
    Inputs: fd: file descriptor of the file to be closed
    Outputs: Resets the flag of the file descriptor of the closed file to unused.
    Returns: 0 if close is successful, -1 if failed
    Side effects: none
*/
int32_t file_close(int32_t fd, file_desc_t * openfiles) {

  // invalid fd
  if (fd <= 1 || fd >= NUM_FILES) {
    return -1;
  }

  // if file is already closed, close failed, return -1
  if (openfiles[fd].flags == 0)
    return -1;

  // close file and return 0
  openfiles[fd].flags = 0;
  return 0;
}


//////// DIR OPS //////////

/* int32_t dir_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Reads the name of a file in the main directory.
    Inputs: fd: file descriptor of the file to read
            buf: buffer to write the directory data to
            nbytes: number of bytes to write
    Outputs: stores file names in buffer
    Returns: returns number of bytes read
    Side effects: none
*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles) {
  if (openfiles[fd].inode_num != 0) {
    return -1;
  }

  int32_t file_pos = openfiles[fd].file_pos;
  if (file_pos >= boot_block.num_dir) {
    return 0;
  }
  int i;
  // if nbytes is longer than 32 bytes, cut off at 32
  int32_t to_read = min(MAX_FILENAME_LEN, nbytes);
  memcpy(buf, &(boot_block.d_entries[file_pos]), to_read);
  for(i=0;((char *)buf)[i]!='\0' && i<MAX_FILENAME_LEN;i++);
  openfiles[fd].file_pos++;
  return i;
}

/* int32_t dir_write(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Writes to directory.
    Inputs: fd: file descriptor of the file to read
            buf: buffer containing data to write to directory
            nbytes: number of bytes to write
    Outputs: nothing because read-only file system
    Returns: -1
    Side effects: none
*/
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles) {
  return -1;
}

/* int32_t dir_close(int32_t fd, file_desc_t * openfiles)
    Description: Closes an opened directory.
    Inputs: fd: file descriptor of the file to be closed
    Outputs: Resets the flag of the file descriptor of the closed file to unused.
    Returns: 0 if close is successful, -1 if failed
    Side effects: none
*/
int32_t dir_close(int32_t fd, file_desc_t * openfiles) {

  // invalid fd
  if (fd <= 1 || fd >= NUM_FILES) {
    return -1;
  }

  // if file is already closed, close failed, return -1
  if (openfiles[fd].flags == 0)
    return -1;

  // close file and return 0
  openfiles[fd].flags = 0;
  return 0;
}

//////// RTC FILE OPS //////////

/* int32_t rtc_file_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Reads from the RTC device.
    Inputs: fd: file descriptor of the file to read
            buf: buffer to write the directory data to
            nbytes: number of bytes to write
    Outputs: stores file names in buffer
    Returns: returns number of bytes read
    Side effects: none
*/
int32_t rtc_file_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles) {
  return rtc_read();
}

/* int32_t dir_write(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Writes to the RTC.
    Inputs: fd: file descriptor of the file to read
            buf: buffer containing the frequency to write to RTC
            nbytes: number of bytes to write
    Outputs: nothing because read-only file system
    Returns: -1
    Side effects: none
*/
int32_t rtc_file_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles) {
  return rtc_write(*((int *)buf));
}

/* int32_t rtc_file_close(int32_t fd, file_desc_t * openfiles)
    Description: Closes an opened RTC device file.
    Inputs: fd: file descriptor of the file to be closed
    Outputs: Resets the flag of the file descriptor of the closed file to unused.
    Returns: 0 if close is successful, -1 if failed
    Side effects: none
*/
int32_t rtc_file_close(int32_t fd, file_desc_t * openfiles) {
  rtc_close();
  file_close(fd, openfiles);
  return 0;
}


//////// STDIN OPS //////////

/* int32_t stdin_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Reads from the terminal.
    Inputs: fd: file descriptor of the file to read
            buf: buffer to write the directory data to
            nbytes: number of bytes to write
    Outputs: stores read data in buffer
    Returns: returns number of bytes read
    Side effects: none
*/
int32_t stdin_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles) {
  if (fd == 0) {
    //keyboard_read(buf, nbytes);
    return terminal_read((uint8_t *)buf);
  }
  return -1;
}

/* int32_t stdin_write(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Writes to stdin
    Inputs: fd: file descriptor of the file to read
            buf: buffer containing data to write to directory
            nbytes: number of bytes to write
    Outputs: nothing because read-only file system
    Returns: -1
    Side effects: none
*/
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles) {
  return -1;
}

/* int32_t stdin_close(int32_t fd, file_desc_t * openfiles)
    Description: Closes stdin.
    Inputs: fd: file descriptor of the file to be closed
    Outputs: Resets the flag of the file descriptor of the closed file to unused.
    Returns: 0 if close is successful, -1 if failed
    Side effects: none
*/
int32_t stdin_close(int32_t fd, file_desc_t * openfiles) {
  return -1;
}

//////// STDOUT OPS //////////

/* int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Reads from the terminal.
    Inputs: fd: file descriptor of the file to read
            buf: buffer to write the directory data to
            nbytes: number of bytes to write
    Outputs: stores read data in buffer
    Returns: returns number of bytes read
    Side effects: none
*/
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles) {
  return -1;
}

/* int32_t stdout_write(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles)
    Description: Writes to stdout
    Inputs: fd: file descriptor of the file to read
            buf: buffer containing data to write to terminal
            nbytes: number of bytes to write
    Outputs: writes to terminal
    Returns: -1
    Side effects: none
*/
int32_t stdout_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles) {
  if (fd == 1) {
    terminal_write((unsigned char *)buf,nbytes);
    return 0;
  }
  return -1;
}

/* int32_t stdout_close(int32_t fd, file_desc_t * openfiles)
    Description: Closes stdout.
    Inputs: fd: file descriptor of the file to be closed
    Outputs: Resets the flag of the file descriptor of the closed file to unused.
    Returns: 0 if close is successful, -1 if failed
    Side effects: none
*/
int32_t stdout_close(int32_t fd, file_desc_t * openfiles) {
  return -1;
}

/// HELPER FUNCTIONS ///

/* int32_t min(int32_t a, int32_t b)
    Description: Helper function to find smaller of two numbers.
    Inputs: two numbers, a and b
    Outputs: smaller of a and b
    Returns: smaller of a and b
    Side effects: none
*/
int32_t min(int32_t a, int32_t b) {
  return (a < b) ? a : b;
}
/* int32_t max(int32_t a, int32_t b)
    Description: Helper function to find smaller of two numbers.
    Inputs: two numbers, a and b
    Outputs: bigger of a and b
    Returns: bigger of a and b
    Side effects: none
*/
int32_t max(int32_t a, int32_t b) {
  return (a > b) ? a : b;
}
