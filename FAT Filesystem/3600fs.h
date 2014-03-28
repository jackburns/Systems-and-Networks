/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#include <time.h>
#include <fuse.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/statfs.h>

#include "disk.h"

#ifndef __3600FS_H__
#define __3600FS_H__


#endif

// VCB struct
typedef struct vcb_s {
  int magic;

  int blocksize;
  int de_start;
  int de_length;
  int fat_start;
  int fat_length;
  int db_start;
  int safe;
  int de_end;
  
  uid_t user;
  gid_t group;
  mode_t mode;
  struct timespec access_time;
  struct timespec modify_time;
  struct timespec create_time;
} vcb;

// Directory Entry struct
typedef struct dirent_s {
  unsigned int valid;
  int first_block;
  unsigned int size;

  uid_t user;
  gid_t group;
  mode_t mode;
  struct timespec access_time;
  struct timespec modify_time;
  struct timespec create_time;
  char name[28];
} dirent;


typedef struct fatent_s {
  unsigned int used:1;
  unsigned int eof:1;
  unsigned int next:30;
} fatent;

int getMagic();

void dwriteVCB(vcb myvcb);

vcb dreadVCB();

void dwriteDirent(int i, dirent myde);

dirent dreadDirent();