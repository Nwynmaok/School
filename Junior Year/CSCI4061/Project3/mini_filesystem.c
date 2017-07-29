/* mini_filesystem.c
  Isaac Schwab, Nathan Kaufman
  Implementation Details */
  
#include "mini_filesystem.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>

int current_files = 0;
// File System Interface
/* Set the log file name as the filename provided as input. Set count to 0.
Initialize (or allocate) anything else needed for your file system to work,
for example, initial values for superblock. Return Success or Failure appropriately.
 */
int Initialize_Filesystem(char* log_filename) {
  Log_Filename = log_filename;
  Count = 0;
  int i;
  //set initial values for superblock
  Superblock.next_free_inode = 0;
  Superblock.next_free_block = 0;
  //allocate each block of disk_blocks to have 512bytes
  for(i = 0; i < MAXBLOCKS; i++) {
    Disk_Blocks[i] = malloc(512);
  }
}

/* ​ Check whether the file you are going to create already exists in the
directory structure. If yes, return with an appropriate error. If not, get the next free inode
from the super block and create an entry for the file in the Inode_List. Then, using the
returned inode number and filename, add the entry to the directory structure. Also,
update the superblock since the next free inode index needs to be incremented. Then
return appropriately. */
int Create_File(char* filename, int uid, int gid) {
  // search directory for the file
  int inode_num = Search_Directory(filename);
  // if the file already exists, return -1
  if(inode_num != -1) {
    return -1;
  }
  //get next inode from superblock
  Super_block sb = Superblock_Read();
  int next_inode = sb.next_free_inode;
  //create new empty inode
  Inode new_file;
  new_file.Inode_Number = next_inode;
  new_file.File_Size = 0;
  new_file.User_Id = uid;
  new_file.Group_Id = gid;
  new_file.Start_Block = sb.next_free_block;
  new_file.End_Block = sb.next_free_block;
  //add the new inode to the Inode_list
  Inode_List[next_inode] = new_file;
  //add new directory entry to Directory_Structure
  if(Add_to_Directory(filename, next_inode) == -1) {
      printf("Add to Directory_Structure failed");
      return -1;
  }
  //update next inode in superblock
  Super_block new_sb;
  new_sb.next_free_inode = next_inode+1;
  new_sb.next_free_block = sb.next_free_block;
  Superblock_Write(new_sb);
  return next_inode;
}

/* Search the directory for the provided file name. If found, set the inode flag
for it to 1 and return the inode number. Otherwise, return appropriately. */
int Open_File(char* filename) {
  // search directory for the file
  int inode_num = Search_Directory(filename);
  // if the file doesn't exist, return -1
  if(inode_num == -1) {
    return -1;
  }
  // read the inode structure from the filesystem
  Inode cur_inode = Inode_Read(inode_num);
  // handle error if Inode_Read fails
  if(cur_inode.Flag == -1) {
    printf("Unable to read inode\n");
    return -1;
  }
  // the inode was returned correctly, so set flag to 1, to represent its open
  cur_inode.Flag = 1;
  // write the updated file back to the filesystem
  if(Inode_Write(inode_num, cur_inode) != 1) {  //check for error
    printf("Failed to write the Inode to the filesystem\n");
    return -1;
  }
  return inode_num;
}

/* For the given inode number, check whether the provided offset and number of bytes to be read is
correct by comparing it with the size of the file. If correct, read the provided number of bytes and
store them in the provided character array; return the number of bytes read. Otherwise, return an
appropriate error. */
int Read_File(int inode_number, int offset, int count, char* to_read) {
  // get the inode fo the given inode number, return -1 if inode_number is invalid
  Inode cur_inode = Inode_Read(inode_number);
  if(cur_inode.Flag == -1) {
    printf("Unable to read inode\n");
    return -1;
  }
  //check that offset and count are valid
  int filesize = cur_inode.File_Size;
  if(count+offset > filesize) {
    printf("Offset and count are invalid\n");
    return -1;
  }
  int bytes_read = 0;
  int cur_block = cur_inode.Start_Block;
  //read the file
  while(count > 0) {
    int read = Block_Read(cur_block, count, to_read, bytes_read);
    cur_block++;
    count = count - read;
    bytes_read = bytes_read + read;
    offset = bytes_read;
  }
  return bytes_read;
}

/* ​ For the given inode number, first check if the provided offset is correct by
comparing it with the size of the file (​ Note: a file is contiguous in this filesystem, you
cannot write at any offset). If correct, write the provided string to the file blocks, update
the ​ inode (since this changes the ​ file size​ , ​ last block etc.) and ​ superblock (as the
next free disk block will change) with the right information and return the number of
bytes written. If incorrect, return appropriately. */
int Write_File(int inode_number, int offset, char* to_write, long size) {
  // get the inode fo the given inode number, return -1 if inode_number is invalid
  int written = 0;
  Inode cur_inode = Inode_Read(inode_number);
  if(cur_inode.Flag == -1) {
    printf("Unable to read inode\n");
    return -1;
  }
  //check offset
  if(offset > 0) {
    printf("This filesystem doesn't allow offset.\n");
    return -1;
  }
  //get size of to_write
  int bytes_to_write = size;
  Super_block sb = Superblock_Read();
  int block_num = cur_inode.Start_Block;  //get the starting block
  char *temp_str = malloc(bytes_to_write);
  strncpy(temp_str, to_write, bytes_to_write);
  while(bytes_to_write > 0) {  //write until there are no more bytes
    int wrote = Block_Write(block_num, bytes_to_write, to_write, written); //write to block
    block_num++; //increment block num
    written += wrote;  //add to total number of bytes written
    bytes_to_write -= wrote; //subtract from bytes remaining
    strncpy(temp_str, to_write+wrote, bytes_to_write); // get the next bytes of the string to write
    cur_inode.File_Size += wrote;  //increment the filesize
    cur_inode.End_Block++;  //increment the end block
    Inode_Write(inode_number, cur_inode);
  }
  sb.next_free_block = cur_inode.End_Block;
  Superblock_Write(sb);
  return written;
}

int Close_File(int inode_number) {
  // read the inode structure from the filesystem
  Inode cur_inode = Inode_Read(inode_number);
  // handle error if Inode_Read fails
  if(cur_inode.Flag == -1) {
    printf("Unable to read inode\n");
    return -1;
  }
  // the inode was returned correctly, check if its open, if it is set flag to 0
  if(cur_inode.Flag == 1) {  //check if file is open
    cur_inode.Flag = 0;  //close file
  }
  // write the updated file back to the filesystem
  if(Inode_Write(inode_number, cur_inode) != 1) {  //check for error
    printf("Failed to write the Inode to the filesystem\n");
    return -1;
  }
  return inode_number;
}



/* Filesystem Calls Low level */
/* Search through the directory structure for the given filename, return Inode number of the
file, if the file is found and error (-1) if it is not. */
int Search_Directory(char* filename) {
  Count++;
  updateLog("Directory_Structure", "Read");
  int i;
  for(i = 0; i < MAXFILES; i++) {
    char *t_filename = malloc(21);
    t_filename = Directory_Structure[i].Filename;
    if(strcmp(t_filename, filename) == 0) {
      return Directory_Structure[i].Inode_Number;
    }
  }
  return -1;
}

/* Add an entry to the directory structure with the ​ filename and ​ inode_number provided
as input. Return the ​ status indicating whether it was able to add the entry to the
directory successfully or not */
int Add_to_Directory(char* filename, int inode_number) {
  updateLog("Directory_Structure", "Write");
  Count++;
  if(current_files >= MAXFILES) {  // No available files left
    return -1;  // return invalid
  }
  else {
    Directory_Structure[current_files].Inode_Number = inode_number;
    strcpy(Directory_Structure[current_files].Filename, filename);
    current_files++;
  }
}

/* For the given ​ inode_number​ , if the file exists, return the ​ Inode structure or ​ -1 if the file
doesn’t exist. */
Inode Inode_Read(int inode_number) {
  updateLog("Inode_List", "Read");
  Count++;
  int i;
  for(i = 0; i < MAXFILES; i++) {
    if(Inode_List[i].Inode_Number == inode_number) {
      return Inode_List[i];
    }
  }
  Inode notfound;
  notfound.Inode_Number = -1;
  return notfound;
}

/* Copy the contents of Inode structure ​ input_inode to the Inode present at the
inode_number​ . */
int Inode_Write(int inode_number, Inode input_inode) {
  updateLog("Inode_List", "Write");
  Count++;
  int i;
  for(i = 0; i < MAXFILES; i++) {
    if(Inode_List[i].Inode_Number == inode_number) {
      Inode_List[i] = input_inode;
      return 1;  //return 1 on success
    }
  }
}

/* Read the given ​ num_bytes from the ​ block_number and write it to the provided
character String ​ to_read​ ; return the number of bytes read. */
int Block_Read(int block_number, int num_bytes, char* to_read, int start_read) {
  updateLog("Disk_Blocks", "Read");
  Count++;
  // printf("Block number: %d\n", block_number);
  int read = 0;
  int i;
  // printf("BLOCKREAD\n");
  if(num_bytes > 512) {
    num_bytes = 512;
  }
  for(i = 0; i < num_bytes; i++) { //possibly will have issue with array out of bounds
    to_read[start_read] = Disk_Blocks[block_number][i];
    read++;
    start_read++;
  }
  return read;
}

/* Given the ​ block_number​ , write the contents of the string ​ to_write to the block and
return the number of bytes written. */
int Block_Write(int block_number, int num_bytes, char* to_write, int start_write) {
  if(block_number>=MAXBLOCKS) {
    printf("Error : FILESYSTEM OUT OF SPACE.\n");
    exit(1);
  }
  updateLog("Disk_Blocks", "Write");
  Count++;
  int write = 0;
  int i;
  if(num_bytes > 512) {
    num_bytes = 512;
  }
  for(i = 0; i < num_bytes; i++) { //possibly will have issue with array out of bounds
    Disk_Blocks[block_number][i] = to_write[start_write];
    write++;
    start_write++;
  }
  return write;
}

/* Return the superblock structure. */
Super_block Superblock_Read() {
  updateLog("Superblock", "Read");
  Count++;
  return Superblock;
}

/* Copy the contents of ​ input_superblock​ to the superblock structure. */
int Superblock_Write(Super_block input_superblock) {
  updateLog("Superblock", "Write");
  Count++;
  Superblock.next_free_inode = input_superblock.next_free_inode;
  Superblock.next_free_block = input_superblock.next_free_block;
}


/* Extra functions */
/* Used to access filesystem details so we can print a summary */
void getFilesystem(FILE *summary) {
  int i = 0;
  fprintf(summary, "Low level system calls: %d\n", Count);
  printf("Low level system calls: %d\n", Count);

  while(i < current_files) {
    char *fname = Directory_Structure[i].Filename;
    char *extn = strrchr(fname, '.');
    int inum = Directory_Structure[i].Inode_Number;
    int filesize = Inode_List[inum].File_Size;
    int start_block = Inode_List[inum].Start_Block;
    int end_block = Inode_List[inum].End_Block;
    fprintf(summary, "Filename: %s, Extensions: %s, Inode: %d Size: %d, Start: %d, End: %d\n", fname, extn, inum, filesize, start_block, end_block);
    printf("Filename: %s, Extensions: %s, Inode: %d Size: %d, Start: %d, End: %d\n", fname, extn, inum, filesize, start_block, end_block);
    i++;
  }
}

/* Handles writing to the log file */
void updateLog(char* structure, char* access) {
  FILE *log_file;
  char *timestamp = malloc(200);
  // build the timestamp for the log
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  struct timeval tv;
  gettimeofday(&tv,NULL);
  sprintf(timestamp, "%d:%d:%d:%d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec);
  log_file = fopen(Log_Filename, "a");
  fprintf(log_file, "<%s> <%s> <%s>\n", timestamp, structure, access);
  fclose(log_file);
}

/* Given an inode, this function accesses the filesystem and returns the filesize for the given inode */
int getFilesize(int inode_number) {
  int filesize;
  Inode temp;
  temp = Inode_Read(inode_number);
  filesize = temp.File_Size;
  return filesize;
}
