#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "rtc.h"
#include "lib.h"
#define BLOCK_SIZE 4096
#define NUM_FILES 8
#define MAX_FILENAME_LEN 32
#define NUM_DENTRIES 63
#define NUM_DBLOCKS 64*16

typedef struct file_desc file_desc_t;

typedef struct file_ops {
  int32_t (*file_close) (int32_t fd, struct file_desc * openfiles);
  int32_t (*file_read) (int32_t fd, void* buf, int32_t nbytes, struct file_desc * openfiles);
  int32_t (*file_write) (int32_t fd, const void* buf, int32_t nbytes, struct file_desc * openfiles);
} file_ops_t;

// file descriptor type
struct file_desc {
  file_ops_t ops;
  int32_t inode_num;
  int32_t file_pos;
  int32_t flags;
};

// directory entry type
typedef struct dentry {
  uint8_t filename[MAX_FILENAME_LEN];
  int32_t filetype;
  int32_t inode_num;
  int32_t reserved[6];    // 24 bytes reserved
} dentry_t;

// boot block type
typedef struct boot_block {
  int32_t num_dir;
  int32_t num_inode;
  int32_t num_data;
  int32_t reserved[13];   // 52 bytes reserved
  dentry_t d_entries[NUM_DENTRIES];
} boot_block_t;

// inode type
typedef struct inode {
  int32_t length;
  int32_t data[NUM_DBLOCKS-1];
} inode_t;

// data block type
typedef struct data_block {
  int32_t data[NUM_DBLOCKS];
} data_block_t;

// initialise file system
int32_t file_sys_init();
// open for files and directories
int32_t do_open(const uint8_t * filename, file_desc_t * openfiles);
// gets next available fd in fd array
int32_t get_fd(struct file_desc, file_desc_t * openfiles);

// open a directory
int32_t dir_open(file_desc_t * openfiles);
// close fa directory
int32_t dir_close(int32_t fd, file_desc_t * openfiles);
// read from directory
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
// write to directory
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);

// open a file
int32_t file_open(int32_t inode_num, file_desc_t * openfiles);
// close a file
int32_t file_close(int32_t fd, file_desc_t * openfiles);
// read from file
int32_t file_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
// write to file
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);

// open rtc file
int32_t rtc_file_open(file_desc_t * openfiles);
// close rtc file
int32_t rtc_file_close(int32_t fd, file_desc_t * openfiles);
// read from rtc file
int32_t rtc_file_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
// write to rtc file
int32_t rtc_file_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);

int32_t std_close(int32_t fd, file_desc_t * openfiles);
int32_t stdin_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdout_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);

int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdout_write(int32_t fd, const void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdin_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes, file_desc_t * openfiles);
int32_t stdin_close(int32_t fd, file_desc_t * openfiles);
int32_t stdout_close(int32_t fd, file_desc_t * openfiles);


// felper function to find min of 2 numbers
int32_t min(int32_t a, int32_t b);
int32_t max(int32_t a, int32_t b);
// rtc ops jump table
extern file_ops_t rtc_ops;
// directory ops jumptable
extern file_ops_t dir_ops;
// file ops jump table
extern file_ops_t file_ops;
// stdin ops jump table
extern file_ops_t stdin_ops;
// stdout ops jump table
extern file_ops_t stdout_ops;
// global boot block
extern boot_block_t boot_block;

extern int8_t* file_sys_addr;
#endif
