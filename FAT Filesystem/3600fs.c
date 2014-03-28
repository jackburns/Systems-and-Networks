/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file contains all of the basic functions that you will need 
 * to implement for this project.  Please see the project handout
 * for more details on any particular function, and ask on Piazza if
 * you get stuck.
 */

#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#define _POSIX_C_SOURCE 199309
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "3600fs.h"
#include "disk.h"

vcb myvcb;

// Helpers for repeat tasks
void dwriteVCB(vcb myvcb){
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myvcb, sizeof(vcb));
  dwrite(0, tmp);
}

vcb dreadVCB(){
  vcb myvcb;
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  dread(0, tmp);
  memcpy(&myvcb, tmp, sizeof(vcb));
  return myvcb;
}

void dwriteDirent(int i, dirent mydirent) {
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &mydirent, sizeof(dirent));
  dwrite(i, tmp);
}

dirent dreadDirent(int i){
  dirent mydirent;
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  dread(i, tmp);
  memcpy(&mydirent, tmp, sizeof(dirent));
  return mydirent;
}


fatent dreadFAT(int i){
  int blocknum = (i / 128) + myvcb.fat_start;
  int blockent = i % 128;

  fatent *fat = (fatent*) malloc(BLOCKSIZE);

  memset(fat, 0, BLOCKSIZE);

  return fat[blockent];
}

void dwriteFAT(int i, fatent myfat) {
  int blocknum = i / 128 + myvcb.fat_start;
  int blockent = i % 128;

  fatent *fat = (fatent*) malloc(BLOCKSIZE);

  memset(fat, 0, BLOCKSIZE);
  dread(blocknum, fat);

  fat[blockent] = myfat;

  dwrite(blocknum, fat);
}

static int nextFreeBlock() {
  int i;
  fatent myfat;
  for(i = 0; i < myvcb.fat_length; i++){
    myfat = dreadFAT(i);
    if(!myfat.used) {
      myfat.used = 1;
      myfat.eof = 1;
      dwriteFAT(i, myfat);
      break;
    }
  }
  return i;
}  
   
/*
 * Initialize filesystem. Read in file system metadata and initialize
 * memory structures. If there are inconsistencies, now would also be
 * a good time to deal with that. 
 *
 * HINT: You don't need to deal with the 'conn' parameter AND you may
 * just return NULL.
 *
 */
static void* vfs_mount(struct fuse_conn_info *conn) {
  fprintf(stderr, "vfs_mount called\n");

  // Do not touch or move this code; connects the disk
  dconnect();

  /* 3600: YOU SHOULD ADD CODE HERE TO CHECK THE CONSISTENCY OF YOUR DISK
           AND LOAD ANY DATA STRUCTURES INTO MEMORY */
  myvcb = dreadVCB();

  // Checking consistency
  if(myvcb.safe == 1){
    myvcb.safe = 0;
  }

  // Make sure magic number is correct
  if(myvcb.magic != 177){
    exit(-1);
  }

  dwriteVCB(myvcb);

  return NULL;
}

/*
 * Called when your file system is unmounted.
 *
 */
static void vfs_unmount (void *private_data) {
  fprintf(stderr, "vfs_unmount called\n");

  /* 3600: YOU SHOULD ADD CODE HERE TO MAKE SURE YOUR ON-DISK STRUCTURES
           ARE IN-SYNC BEFORE THE DISK IS UNMOUNTED (ONLY NECESSARY IF YOU
           KEEP DATA CACHED THAT'S NOT ON DISK */

  //Unmounted safely
  myvcb.safe = 1;
  dwriteVCB(myvcb);

  // Do not touch or move this code; unconnects the disk
  dunconnect();
}

/* 
 *
 * Given an absolute path to a file/directory (i.e., /foo ---all
 * paths will start with the root directory of the CS3600 file
 * system, "/"), you need to return the file attributes that is
 * similar stat system call.
 *
 * HINT: You must implement stbuf->stmode, stbuf->st_size, and
 * stbuf->st_blocks correctly.
 *
 */
static int vfs_getattr(const char *path, struct stat *stbuf) {
  fprintf(stderr, "vfs_getattr called\n");

  // Do not mess with this code 
  stbuf->st_nlink = 1; 
  stbuf->st_rdev  = 0;
  stbuf->st_blksize = BLOCKSIZE;

  /* 3600: YOU MUST UNCOMMENT BELOW AND IMPLEMENT THIS CORRECTLY */

  dirent mydirent;
  int i;
  int hit = 0;

  // Resolve the file
  for(i = myvcb.de_start; i < myvcb.de_end; i++){
    mydirent = dreadDirent(i);

    if(strcmp(mydirent.name, path) == 0){
      if(mydirent.valid){
        hit = 1;
        break;
      }
    }
  }

  // Fill in file info
  if(hit){
    if (strcmp(path, "/")  == 0) {
      stbuf->st_mode  = 0777 | S_IFDIR;
    }
    else {
      stbuf->st_mode  = (mydirent.mode & 0x0000ffff) | S_IFREG;
    }

  stbuf->st_uid = mydirent.user;
  stbuf->st_gid = mydirent.group;
  stbuf->st_blocks = mydirent.size/BLOCKSIZE;
  stbuf->st_size = mydirent.size;

  stbuf->st_ctime = mydirent.create_time.tv_sec;
  stbuf->st_atime = mydirent.access_time.tv_sec;
  stbuf->st_mtime = mydirent.modify_time.tv_sec;

  return 0;
 }

 // Path doesn't exist
 else{
    return -ENOENT;
 }
}

/*
 * Given an absolute path to a directory (which may or may not end in
 * '/'), vfs_mkdir will create a new directory named dirname in that
 * directory, and will create it with the specified initial mode.
 *
 * HINT: Don't forget to create . and .. while creating a
 * directory.
 *
 *
 * NOTE: YOU CAN IGNORE THIS METHOD, UNLESS YOU ARE COMPLETING THE 
 *       EXTRA CREDIT PORTION OF THE PROJECT.  IF SO, YOU SHOULD
 *       UN-COMMENT THIS METHOD.
 *
static int vfs_mkdir(const char *path, mode_t mode) {

  return -1;
} 
 */

/** Read directory
 *
 * Given an absolute path to a directory, vfs_readdir will return 
 * all the files and directories in that directory.
 *
 * HINT:
 * Use the filler parameter to fill in, look at fusexmp.c to see an example
 * Prototype below
 *
 * Function to add an entry in a readdir() operation
 *
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stat file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @return 1 if buffer is full, zero otherwise
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *                                 const struct stat *stbuf, off_t off);
 *         
 * Your solution should not need to touch fi
 *
 */
static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {

  if(strcmp(path, "/") == 0){

    dirent mydirent;
    int i;

    for(i = myvcb.de_start + offset; i < myvcb.de_end; i++){
      mydirent = dreadDirent(i);

      if(mydirent.valid) {

        if(filler(buf, mydirent.name + 1, NULL, i + 1 - myvcb.de_start) != 0);{
        return 0;
      }
      }
    }
    return 0;
  }
  return -1;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), vfs_create 
 * will create a new file named myFile in the /a/b directory.
 *
 */
static int vfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  dirent mydirent;
  int i;
  int de_end = myvcb.de_start + myvcb.de_length;

  // Make sure file does not already exist
  for(i = myvcb.de_start; i < de_end; i++){
    mydirent = dreadDirent(i);

    if(strcmp(mydirent.name, path) == 0){
      return -1;
    }
  }

  // find new Dirent to write to
  for(i = myvcb.de_start; i < de_end; i++){
    mydirent = dreadDirent(i);

    if(mydirent.valid == 0){

      strcpy(mydirent.name, path);

      mydirent.valid = 1;
      mydirent.user = geteuid();
      mydirent.group = getegid();
      mydirent.mode = mode;


      struct timespec currenttime;
      clock_gettime(CLOCK_REALTIME, &currenttime);

      mydirent.access_time = currenttime;
      mydirent.create_time = currenttime;
      mydirent.modify_time = currenttime;
     
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}

/*
 * The function vfs_read provides the ability to read data from 
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes starting at the specified
 * offset (offset) from the specified file (path)
 * on your filesystem into the memory address 'buf'. The return 
 * value is the amount of bytes actually read; if the file is 
 * smaller than size, vfs_read will simply return the most amount
 * of bytes it could read. 
 *
 * HINT: You should be able to ignore 'fi'
 *
 */
static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
  dirent mydirent;
  fatent myfat;

  int i;
  int bytesRead = 0;
  int currentBlock;
  int fileSize = 0;
  char* file = (char*) calloc(BLOCKSIZE, sizeof(char));

  //Find the file
  for(i = myvcb.de_start; i < myvcb.de_end; i++){
    mydirent = dreadDirent(i);
    if(strcmp(mydirent.name, path) == 0 && mydirent.valid ){
      clock_gettime(CLOCK_REALTIME, &mydirent.access_time);

      if(mydirent.first_block != -1) {
        currentBlock = mydirent.first_block;

        // Find which block to begin reading from
        while(BLOCKSIZE <= offset){
          myfat = dreadFAT(currentBlock);
          if(myfat.used){
            currentBlock = myfat.next;
          }
          offset -= BLOCKSIZE;
        }

      int j;
      for(j = 0; j < size; j++) {
        // Check for files that span multiple blocks
          if(BLOCKSIZE <= fileSize) {
            myfat = dreadFAT(currentBlock);
            if(!myfat.eof) {
              currentBlock = myfat.next;
            }
          }
          fileSize = offset;

          // Read file
          dread(currentBlock + myvcb.db_start, file);

          // Copy file to given buffer
          while(fileSize < BLOCKSIZE){
            size--;
            bytesRead++;
            fileSize++;
          }
          memcpy(buf, file, fileSize);
        }
      }

     dwriteDirent(i, mydirent);
     return bytesRead;
    }
  }
  return -1;
}

/*
 * The function vfs_write will attempt to write 'size' bytes from 
 * memory address 'buf' into a file specified by an absolute 'path'.
 * It should do so starting at the specified offset 'offset'.  If
 * offset is beyond the current size of the file, you should pad the
 * file with 0s until you reach the appropriate length.
 *
 * You should return the number of bytes written.
 *
 * HINT: Ignore 'fi'
 */
static int vfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
  { 

  /* 3600: NOTE THAT IF THE OFFSET+SIZE GOES OFF THE END OF THE FILE, YOU
           MAY HAVE TO EXTEND THE FILE (ALLOCATE MORE BLOCKS TO IT). */
  dirent myde;
  fatent myfat;

  int i;
  int currentBlock;
  int fileSize = 0;
  int bytesWritten = 0;
  char* file = (char*) calloc(BLOCKSIZE, sizeof(char));

  // Find the dirent
  for(i = myvcb.de_start; i < myvcb.de_end; i++ ) {
    myde = dreadDirent(i);
    myde.size = myde.size + size; 
    if(strcmp(myde.name, path) == 0 && myde.valid) {
  
      // Check for first block
      if(myde.first_block == -1)
        myde.first_block = nextFreeBlock();
      
        currentBlock = myde.first_block;
        myfat = dreadFAT(currentBlock);

        // Find which block to begin writing to
      while(BLOCKSIZE <= offset) {
        if(myfat.used) {
          currentBlock = myfat.next;
        }
        offset -= BLOCKSIZE;
      }

      int j;
      for(j = 0; j < size; j++) {
        // Check if file spans multiple blocks
        if(BLOCKSIZE <= fileSize) {
          currentBlock = nextFreeBlock();
          myfat.next = currentBlock;
        }

        fileSize = offset;

        // Copy buf to file 
        while(fileSize < BLOCKSIZE && size > 0) {
          size--;
          bytesWritten++;
          fileSize++;
        }

        memcpy(file, buf, fileSize);
        dwrite(currentBlock + myvcb.db_start, file);
      }

      dwriteDirent(i, myde);
      return bytesWritten;
    }
  }
  return -1;
}


/**
 * This function deletes the last component of the path (e.g., /a/b/c you 
 * need to remove the file 'c' from the directory /a/b).
 */
static int vfs_delete(const char *path)
{

  /* 3600: NOTE THAT THE BLOCKS CORRESPONDING TO THE FILE SHOULD BE MARKED
           AS FREE, AND YOU SHOULD MAKE THEM AVAILABLE TO BE USED WITH OTHER FILES */
  dirent mydirent;
  int i;
  // Make sure file exists and is a valid file
  for(i = myvcb.de_start; i < myvcb.de_start + myvcb.de_length; i++){
    mydirent = dreadDirent(i);

    if(strcmp(mydirent.name, path) == 0){

      // Remove file's entry from directory
      mydirent.valid = 0;
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}

/*
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * HINT: Renaming could also be moving in disguise
 *
 */
static int vfs_rename(const char *from, const char *to)
{
  int i;
  int de_end = myvcb.de_start + myvcb.de_length;
  for(i = myvcb.de_start; i < de_end; i++){
    dirent mydirent = dreadDirent(i);
    if(strcmp(from, mydirent.name) == 0){
      strcpy(mydirent.name,to);
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}


/*
 * This function will change the permissions on the file
 * to be mode.  This should only update the file's mode.  
 * Only the permission bits of mode should be examined 
 * (basically, the last 16 bits).  You should do something like
 * 
 * fcb->mode = (mode & 0x0000ffff);
 *
 */
static int vfs_chmod(const char *file, mode_t mode)
{
  int i;
  int de_end = myvcb.de_start + myvcb.de_length;
  for(i = myvcb.de_start; i < de_end; i++){
    dirent mydirent = dreadDirent(i);
    if(strcmp(file, mydirent.name) == 0){
      mydirent.mode = (mode & 0x0000ffff);
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}

/*
 * This function will change the user and group of the file
 * to be uid and gid.  This should only update the file's owner
 * and group.
 */
static int vfs_chown(const char *file, uid_t uid, gid_t gid)
{
  int i;
  int de_end = myvcb.de_start + myvcb.de_length;
  for(i = myvcb.de_start; i < de_end; i++){
    dirent mydirent = dreadDirent(i);
    if(strcmp(file, mydirent.name) == 0){
      mydirent.group = gid;
      mydirent.user = uid;
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}

/*
 * This function will update the file's last accessed time to
 * be ts[0] and will update the file's last modified time to be ts[1].
 */
static int vfs_utimens(const char *file, const struct timespec ts[2])
{
  int i;
  int de_end = myvcb.de_start + myvcb.de_length;
  for(i = myvcb.de_start; i < de_end; i++){
    dirent mydirent = dreadDirent(i);
    if(strcmp(file, mydirent.name) == 0){
      mydirent.access_time = ts[0];
      mydirent.modify_time = ts[1];
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}



// static void cleanBlocks(int index)
// {
//   if(index != 0) {
//     fatent myfat = dreadFAT(index);
//     while(myfat.used) {
//       myfat.used = 0;
//       dwriteFAT(index, myfat);
//       if(!myfat.eof) 
//         index = myfat.next;
//       myfat = dreadFAT(index);
//     }
//   }
// }
/*
 * This function will truncate the file at the given offset
 * (essentially, it should shorten the file to only be offset
 * bytes long).
 */
static int vfs_truncate(const char *file, off_t offset)
{
  dirent mydirent;
  int i;
  fatent myfat;
  int pos;

  // Find directory
  for(i = myvcb.de_start; i < myvcb.de_start + myvcb.de_length; i++) {
    mydirent = dreadDirent(i);
    if(strcmp(mydirent.name, file) == 0) {
      mydirent.size = offset;
      pos = mydirent.first_block;

      // Check if offset is greater than one block
      if(offset > BLOCKSIZE){
        int j;
        for(j = 0; j < offset / BLOCKSIZE; j++){
          myfat = dreadFAT(pos);
          if(myfat.used){
            pos = myfat.next;
          }
        }
      }

      myfat = dreadFAT(pos); 

      if((pos = myfat.next) != 0) {
        fatent myfat = dreadFAT(pos);
        while(myfat.used) {
          myfat.used = 0;
          dwriteFAT(pos, myfat);
        }
      } 
      dwriteDirent(i, mydirent);
      return 0;
    }
  }
  return -1;
}



/*
 * You shouldn't mess with this; it sets up FUSE
 *
 * NOTE: If you're supporting multiple directories for extra credit,
 * you should add 
 *
 *     .mkdir  = vfs_mkdir,
 */
static struct fuse_operations vfs_oper = {
    .init    = vfs_mount,
    .destroy = vfs_unmount,
    .getattr = vfs_getattr,
    .readdir = vfs_readdir,
    .create  = vfs_create,
    .read  = vfs_read,
    .write   = vfs_write,
    .unlink  = vfs_delete,
    .rename  = vfs_rename,
    .chmod   = vfs_chmod,
    .chown   = vfs_chown,
    .utimens   = vfs_utimens,
    .truncate  = vfs_truncate,
};

int main(int argc, char *argv[]) {
    /* Do not modify this function */
    umask(0);
    if ((argc < 4) || (strcmp("-s", argv[1])) || (strcmp("-d", argv[2]))) {
      printf("Usage: ./3600fs -s -d <dir>\n");
      exit(-1);
    }
    return fuse_main(argc, argv, &vfs_oper, NULL);
}



