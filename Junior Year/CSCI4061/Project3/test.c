/* CSCI 4061 Assignment 3
   Student ID:4841729, 4872400
   Student Name:Isaac Schwab, Nathan Kaufman
   test.c

   Program Description:
   This program takes in an input_directory, output_directory, and log_filename.
   It then creates a Filesystem using the mini_filesystem API. The filesystem is
   created by recursivly reading in files from the input_directory. This is done
   in write_into_filesystem. After the files have been written to the filesystem
   we call make_filesystem_summary to output summary.txt, which displays a
   summary of the filesystem. After that we read the images back from the filesystem
   and then write them to the output directory. Finally, we convert the images in the
   output directory using fork() and execl to convert the images. Then an html file
   is generated with all of these images and thumbnails.

   Helper Functions:
   We wrote various helper functions throughout test.c and the mini_filesystem.
   They are listed below:
    - void get_and_write_files(char *path)
    - void writeFile(char* filename, char* path)
    - int getFilesize(int inode_number);
    - void getFilesystem(FILE *summary);
    - void updateLog(char* structure, char* access);
   These functions helped simplify the code, and also kept the mini_filesystem
   data structures unexposed.
   In addition to creating these helper functions, we also added a few parameters
   to some of the given functions. These added parameters made sense to add as they
   helped with the looping.

   Refer to the README for program usage, and other details.
*/

#include "mini_filesystem.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>

/* Test Helper Interface */
void write_into_filesystem(char* input_directory, char *log_filename);
void make_filesystem_summary(char* filename);
void read_images_from_filesystem_and_write_to_output_directory(char* output_directory);
void generate_html_file(char* filename);
void get_and_write_files(char *path);
int writeFile(char* filename, char* path);

char* files[128];
int TOTAL_FILES;

/* Main function */
int main(int argc, char* argv[]){
  // check for valid number of arguments
  if(argc < 4) {
    printf("Invalid Number of Arguments.\n");
    printf("USAGE: ./test <input_dir> <output_dir> <log_filename>\n");
    exit(1);
  }
  printf("\nStarting Program...\n");
  /* Check for output_dir, if it doesn't exist create it */
	int result = mkdir(argv[2], 0777);
	if(result == -1) {
		printf("Output directory: %s was successfully opened\n", argv[2]);
    printf("\n");
	}
	if(result == 0) {
		printf("Created output directory: %s\n", argv[2]);
    printf("\n");
	}

	TOTAL_FILES = 0;

  write_into_filesystem(argv[1], argv[3]);
  make_filesystem_summary("summary.txt");
  read_images_from_filesystem_and_write_to_output_directory(argv[2]);
  generate_html_file(argv[2]);

  printf("\nProgram complete!\n");

  return 0;
}

// Auxillary function that calls the the Filesystem API calls Create_File and Write_File for the input file
int writeFile(char* filename, char* path) {
	FILE *fp;
	int inode_num;
	long lSize;
	char *buffer;
	int bytes_wrote;
  uid_t uid = getuid();
  gid_t gid = getgid();

  // Create the new file in the Filesystem by calling Create_File from API
  inode_num = Create_File(filename, (int)uid, (int)gid);
  if(inode_num == -1) {
  	printf("File: %s already exists, moving on to next file.\n", filename);
  	return -1;
  }

  /* Open file */
	fp = fopen ( path , "r" );
	if( !fp ) perror(path),exit(1);
	//get size
	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );
	/* allocate memory for entire content */
	buffer = calloc( 1, lSize+1 );
	if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

	/* copy the file into the buffer */
	if( 1!=fread( buffer , lSize, 1 , fp) )
	  fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

	// now write the data to the newly created filesystem
	bytes_wrote = Write_File(inode_num, 0, buffer, lSize);
  if(bytes_wrote == -1) {
    printf("Error - Failed to Write File\n");
  }
	printf("File: %s, bytes: %d\n", filename, bytes_wrote);

	fclose(fp);
	free(buffer);
	return 0;
}

/* Recursive Helper function, used to scan directorys recursivly. If we encounter a
  file that is not a directory that we call writeFile, if it is a directory, then we recursivly
  call this function again with the directory as the arguement*/
void get_and_write_files(char *path) {
	DIR* FD;
	struct dirent* in_file;
	/* Scanning the input directory */
  if (NULL == (FD = opendir (path))) {
    perror("Error - Failed to open input directory");
  }
  while ((in_file = readdir(FD))) { // read eadch file in the directory
    /* Don't get the parent and current directory names */
    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    // Build path
    char fullpath[300];
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, in_file->d_name); // corrected

    if(in_file->d_type == DT_DIR) {
	    get_and_write_files(fullpath); // recursivly call for the subdirectory
		}
    else {
	    char *extn = strrchr(in_file->d_name, '.');
	    if(strcmp(extn, ".jpg") != 0) {  //check that file is a jpg
	    	continue;
	    }
      //add file to file list
	    files[TOTAL_FILES] = malloc(21);
	    strcpy(files[TOTAL_FILES], in_file->d_name);
	    TOTAL_FILES++;
	    writeFile(in_file->d_name, fullpath);  // call helper function to write the file
    }
  }
  closedir(FD);
}

/* This function will read all of the files inside the  input_directory (which might be nested).
Also it will provide the  log_filename ; check if the file exists, and if it does, remove and create a new one.
Your program should first initialize the file system bymaking a call to Initialize_Filesystem by passing log_filename to it. */
void write_into_filesystem(char* input_directory, char *log_filename) {
  //check if log_filename already exists, if it does remove it before we create a new one
  FILE * file;
  file = fopen(log_filename, "r");
  if (file){
   remove(log_filename);
   fclose(file);
  }
  printf("Writing into filesystem...\n");
  // Now initialize filesystem and then write into it by calling helper function get_and_write_files
	Initialize_Filesystem(log_filename);
	get_and_write_files(input_directory);
}

/* This function will read all the image (jpg) files from the filesystem and write into the
output_directory */
void read_images_from_filesystem_and_write_to_output_directory(char* output_directory) {
	FILE *output_file;
	struct dirent* in_file;
	int inode_num;
	int filesize;
	char *path;
	char *buffer;
	int bytes_wrote;

  printf("\n");
  printf("Reading Images from Filesystem and Writing to %s...\n", output_directory);
  // read files that are currently stored in the filesystem from the list we created
  int i;
  for(i = 0; i < TOTAL_FILES; i++) {
  	printf("File: %s  ", files[i]);
    // Open the file and get filesize
    inode_num = Open_File(files[i]);
    filesize = getFilesize(inode_num);
    printf(" filesize: %d\n", filesize);
    // create array that we will read to based off the filesize
    buffer = malloc(filesize);
    path = malloc(200);
    // read file to buffer using Filesystem API call
    if(Read_File(inode_num, 0, filesize, buffer) == -1) {
    	printf("Error - Failed to Read_File\n");
    }
    // build the new path
    strcpy(path, output_directory);
    strcat(path, "/");
    strcat(path, files[i]);
    // Open the current html file
    output_file = fopen(path, "w");
    if (output_file == NULL) {
      perror("Error : Failed to open log_file");
      exit(1);
    }
    fwrite(buffer, 1, filesize, output_file);  // write the buffer to the output file
		fclose(output_file);
		free(buffer);
  }
}

/* This function creates a summary of the filesystem */
void make_filesystem_summary(char* filename) {
	FILE *summary;
	summary = fopen(filename, "w");
  printf("\nFilesystem Summary...\n");
  if (summary == NULL) {
    perror("Error - Failed to open summary");
  }
  getFilesystem(summary);
}

/* This function will convert the image files inside output directory to thumbnail images,
write into the same directory and prepare a html file using the thumbnails. */
void generate_html_file(char* output_directory) {
	// read files in output directory
	DIR* FD;
	FILE *html_file;
	struct dirent* in_file;
	pid_t pid;
	char* file_in = malloc(200);
	char* file_out = malloc(200);
	char* src = malloc(200);
	char* href = malloc(200);

  printf("\nGenerating HTML files...\n");
	/* Convert files to thumb */
	/* Scanning the input directory */
  if (NULL == (FD = opendir (output_directory))) {
    perror("Error - Failed to open output_directory");
    exit(1);
  }
  //open the html file we will write to
  html_file = fopen("filesystem_content.html", "w");
  if (html_file == NULL) {
    perror("Error - Failed to open html_file");
    exit(1);
  }
	fprintf(html_file, "<!DOCTYPE html><html><head><title>Filesystem Images</title></head><body>\n");

  while ((in_file = readdir(FD))) { // read eadch file in the directory
    /* Don't get the parent and current directory names */
    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    printf("File: %s\n", in_file->d_name);
    //build strings for convert
    strcpy(file_in, output_directory);
    strcat(file_in, "/");
    strcat(file_in, in_file->d_name);

    strcpy(file_out, output_directory);
    strcat(file_out, "/");

    /* remove the extension from the filename */
    size_t len = strlen(in_file->d_name);
    char *basename = malloc(len-3);
    if (!basename); /* handle error */
    memcpy(basename, in_file->d_name, len-4);
    basename[len - 4] = 0;
    strcat(file_out, basename);
    strcat(file_out, "_thumb.jpg");

    // create child for exec calls to convert
    pid = fork();
    if(pid == 0) {
    	//execl("/usr/local/bin/convert", "convert", file_in, "-resize", "200x200", file_out, NULL);
      execl("/usr/bin/convert", "convert", file_in, "-resize", "200x200", file_out, NULL);
    }
    //wait for children to finish
  	wait(&pid);
  	/* Now build html file with thumbs */
  	//build href string
  	strcpy(href, "<a href=\"");
  	strcat(href, file_in);
  	strcat(href, "\">");
  	//build src string
  	strcpy(src, "<img src=\"");
  	strcat(src, file_out);
  	strcat(src, "\"></a>");

	  /* Add strings to html file */
	  fprintf(html_file, href);
	  fprintf(html_file, src);
	  }
	fprintf(html_file, "</body></html>\n");
	fclose(html_file);
}
