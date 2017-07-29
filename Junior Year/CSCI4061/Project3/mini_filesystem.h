/* CSCI 4061 Assignment 3
   mini_filesystem.h
   Isaac Schwab, Nathan Kaufman
*/
#include <stdio.h>


#ifndef MINI_FILESYSTEM_H
#define MINI_FILESYSTEM_H

#define MAXFILES 128
#define MAXBLOCKS 8192
#define BLOCKSIZE 512

typedef struct superblock
{
  int next_free_inode;
  int next_free_block;

}Super_block;

typedef struct inode
{
  int Inode_Number;
  int User_Id;
  int Group_Id;
  int File_Size;
  int Start_Block;
  int End_Block;
  int Flag;
}Inode;

typedef struct directory
{
  char Filename[21];
  int Inode_Number;
}Directory;

/* Declare Filesystem structures */
Super_block Superblock;
Directory Directory_Structure[MAXFILES];
Inode Inode_List[MAXFILES];
char* Disk_Blocks[MAXBLOCKS];

/* Declare variable for Count and Log Filename */
int Count;
char* Log_Filename;
extern int current_files;
/* Filesystem Interface Declaration
   See the assignment for more details */

int Initialize_Filesystem(char* log_filename);
int Create_File(char* filename, int uid, int gid);
int Open_File(char* filename);
int Read_File(int inode_number, int offset, int count, char* to_read);
int Write_File(int inode_number, int offset, char* to_write, long size);
int Close_File(int inode_number);
int getFilesize(int inode_number);

/* Filesystem Calls Low level */
int Search_Directory(char* filename);
int Add_to_Directory(char* filename, int inode_number);
Inode Inode_Read(int inode_number);
int Inode_Write(int inode_number, Inode input_inode);
int Block_Read(int block_number, int num_bytes, char* to_read, int start_read);
int Block_Write(int block_number, int num_bytes, char* to_write, int start_write);
Super_block Superblock_Read();
int Superblock_Write(Super_block input_superblock);
void getFilesystem(FILE *summary);
void updateLog(char* structure, char* access);

#endif
