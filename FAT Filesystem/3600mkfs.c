/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount (you will call it manually when you format 
 * your disk file).
 */

#include "3600fs.h"

int main(int argc, char** argv) {
  // Do not touch this function
  if (argc != 2) {
    printf("Invalid number of arguments \n");
    printf("usage: %s diskSizeInBlockSize\n", argv[0]);
    return 1;
  }

  unsigned long size = atoi(argv[1]);
  printf("Formatting the disk with size %lu \n", size);
  myformat(size);
}

void myformat(int size) {
  // Do not touch or move this function
  dcreate_connect();

  /* 3600: FILL IN CODE HERE.  YOU SHOULD INITIALIZE ANY ON-DISK
           STRUCTURES TO THEIR INITIAL VALUE, AS YOU ARE FORMATTING
           A BLANK DISK.  YOUR DISK SHOULD BE size BLOCKS IN SIZE. */

  /* 3600: AN EXAMPLE OF READING/WRITING TO THE DISK IS BELOW - YOU'LL
           WANT TO REPLACE THE CODE BELOW WITH SOMETHING MEANINGFUL. */

  vcb myvcb;
  myvcb.magic = 177;
  myvcb.blocksize = BLOCKSIZE;
  myvcb.de_start = 1;
  myvcb.de_length = (size * 10/100);
  myvcb.de_end = myvcb.de_start + myvcb.de_length;
  myvcb.safe = 1;

  myvcb.fat_start = 101;  
  myvcb.fat_length = 7;  
  myvcb.db_start = 108;

  myvcb.user = getuid();
  myvcb.group = getgid();
  myvcb.mode = 0777;

  struct timespec currenttime;
  clock_gettime(CLOCK_REALTIME, &currenttime);

  myvcb.access_time = currenttime;
  myvcb.create_time = currenttime;
  myvcb.modify_time = currenttime;

  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myvcb, sizeof(vcb));
  dwrite(0, tmp);

  dirent mydirent;
  mydirent.valid = 0;
  mydirent.first_block = -1;
  mydirent.size = 0;

  mydirent.user = getuid();
  mydirent.group = getgid();
  mydirent.mode = 0777;

  int i;
  for (i = myvcb.de_start; i < myvcb.de_length; i++) 
    memset(tmp, 0, BLOCKSIZE);
    memcpy(tmp, &mydirent, sizeof(dirent));
    dwrite(i, tmp);

  // Do not touch or move this function
  dunconnect();
}