// variant_2.c

#include "variant_2.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>


// Handles the specific variant way of processing the files
int process_var2(char* input_path, char* output_path) {
  //signal stuff
  struct sigaction sa;
  memset (&sa, 0, sizeof(sa));
  sa.sa_handler = &timer_handler;
  sigaction (SIGALRM, &sa, NULL);
  //timer stuff
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 1000;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 1000;
  //start timer
  setitimer (ITIMER_REAL, &timer, NULL);

  //declare variables for thread
  pthread_t tid;
  int retval;
  pid_t tid2 = syscall(__NR_gettid);
  fprintf(log_file, "Starting Variant 2\n");

  // create thread for the starting directory
  if(pthread_create(&tid, NULL, read_dir2, (void*)input_path)) {
    fprintf(log_file, "FAILED to create thread\n");
    exit(EXIT_FAILURE);
  }
  num_threads = num_threads + 1;
  // printf("Created Thread: %s\n", input_path);
  fprintf(log_file, "Created Thread: %s\n", input_path);
  // wait for thread to finish
  pthread_join(tid, retval);
  return 0;
}


// handles actually processing the file and writing it to the html file
void process_file2(char* fullpath) {
  file_traversed = file_traversed + 1; //increment files traversed for logging
  pid_t curthread = syscall(__NR_gettid);

  // get file info
  struct stat sb;
  if (stat(fullpath, &sb) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }

  // process modified time
  time_t t = sb.st_mtime;
  struct tm lt;
  localtime_r(&t, &lt);
  char timbuf[80];
  strftime(timbuf, sizeof(timbuf), "%c", &lt);

  // get file type
  char *extn = strrchr(fullpath, '.');

  // Add to html_file
  char* temp = malloc(200);
  strcpy(temp, "../");
  strcat(temp, fullpath);
  // printf("Process file: %s tid: %ld\n", fullpath, (long) curthread);
  fprintf(html_file, "<a href='%s'><img src='%s' width=100 height=100/></a>\n", temp, temp);
  fprintf(html_file, "<p align='left'> iNode: %d, Type: %d, FileType: %s Size: %ld bytes, Last Modified: %s, Thread Id: %ld</p>\n",
          (int)sb.st_ino, (int)sb.st_mode, extn, (long)sb.st_size, timbuf, (long)curthread);
  fprintf(log_file,"Process file: %s tid: %ld\n", fullpath, (long) curthread);
}


/* Recursive Helper function, used to scan directorys recursivly. If we encounter a
  file that is not a directory that we call writeFile, if it is a directory, then we recursivly
  call this function again with the directory as the arguement*/
void *read_dir2(void *path) {
  dir_traversed = dir_traversed+1; //increment directories traversed for logging
  DIR* FD;
  struct dirent* in_file;

	/* Scanning the input directory */
  if (NULL == (FD = opendir ((char *) path))) {
    perror("Error - Failed to open input directory");
    exit(EXIT_FAILURE);
  }

  // declare arrays and counters to hold files and directories that we read in
  char* files[MAXFILES];
  char* directories[MAXFILES];
  int num_files = 0;
  int num_dirs = 0;
  int i;

  // read current directory and copy files to corresponding array
  while ((in_file = readdir(FD))) { // read eadch file in the directory
    /* Don't get the parent and current directory names */
    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    // Build path
    char fullpath[300];
    strcpy(fullpath, (char *) path);
    strcat(fullpath, "/");
    strcat(fullpath, in_file->d_name); // corrected

    // encountered directory add to directory array
    if(in_file->d_type == DT_DIR) {
      directories[num_dirs] = malloc(256);
      strcpy(directories[num_dirs], fullpath);
      num_dirs++;
    }
    else { // is a file so add to file array
      files[num_files] = malloc(256);
      strcpy(files[num_files], fullpath);
      num_files++;
    }
  }
  closedir(FD); // finished reading through directory so close directory

  //declare variables required to create threads for each image
  pthread_t tid_png, tid_jpg, tid_bmp, tid_gif;
  int retval_png, retval_jpg, retval_bmp, retval_gif;
  struct process_args args;
  for(i = 0; i < num_files; i++) {
    args.files[i] = malloc(256);
    strcpy(args.files[i],files[i]);
  }
  args.length = num_files;


  // create PNG thread
  if(pthread_create(&tid_png, NULL, process_png, (void *)&args)) {
    fprintf(log_file, "FAILED to create thread\n");
    exit(EXIT_FAILURE);
  }
  num_threads = num_threads + 1;
  fprintf(log_file, "Created Thread: png\n");
  // wait for thread to finish
  pthread_join(tid_png, retval_png);


  // create JPG thread
  if(pthread_create(&tid_jpg, NULL, process_jpg, (void *)&args)) {
    fprintf(log_file, "FAILED to create thread\n");
    exit(EXIT_FAILURE);
  }
  num_threads = num_threads + 1;
  fprintf(log_file, "Created Thread: jpg\n");
  // wait for thread to finish
  pthread_join(tid_jpg, retval_jpg);


  // create BMP thread
  if(pthread_create(&tid_bmp, NULL, process_bmp, (void *)&args)) {
    fprintf(log_file, "FAILED to create thread\n");
    exit(EXIT_FAILURE);
  }
  num_threads = num_threads + 1;
  fprintf(log_file, "Created Thread: bmp\n");
  // wait for thread to finish
  pthread_join(tid_bmp, retval_bmp);


  // create GIF thread
  if(pthread_create(&tid_gif, NULL, process_gif, (void *)&args)) {
    fprintf(log_file, "FAILED to create thread\n");
    exit(EXIT_FAILURE);
  }
  num_threads = num_threads + 1;
  fprintf(log_file, "Created Thread: gif\n");
  // wait for thread to finish
  pthread_join(tid_gif, retval_gif);


  // recursivly handle directories
  for(i = 0; i < num_dirs; i++) {
    pthread_t tid_dir;
    int retval;
    // call new directory in a new thread per variant_1 requirements
    if(pthread_create(&tid_dir, NULL, read_dir2, (void*)directories[i])) {
      fprintf(log_file, "FAILED to create thread\n");
      exit(EXIT_FAILURE);
    }
    num_threads = num_threads + 1;
    // printf("Created Thread: %s\n", directories[i]);
    fprintf(log_file, "Created Thread: %s\n", directories[i]);
    // wait for thread to finish
    pthread_join(tid_dir, retval);
  }
} //end read_dir2


// threaded function that takes as an aruguement an array of files and the length in a struct
// if the file type is correct it then calls process file
void process_png(void *arguments) {
  struct process_args *args = (struct process_args *)arguments;
  // process files in directory
  int i;
  for(i = 0; i < args->length; i++) {
    char *extn = strrchr(args->files[i], '.');
    //check that file is a valid image
    if(strcmp(extn, ".png") == 0) {  //check that file is a jpg
      png_file = png_file + 1; //increment counter for catalog.log
      process_file2(args->files[i]);  // call helper function to write the file
    }
  }
}

void process_jpg(void *arguments) {
  struct process_args *args = (struct process_args *)arguments;
  // process files in directory
  int i;
  for(i = 0; i < args->length; i++) {
    char *extn = strrchr(args->files[i], '.');
    //check that file is a valid image
    if(strcmp(extn, ".jpg") == 0) {  //check that file is a jpg
      jpg_file = jpg_file + 1; //increment counter for catalog.log
      process_file2(args->files[i]);  // call helper function to write the file
    }
  }
}

void process_gif(void *arguments) {
  struct process_args *args = (struct process_args *)arguments;
  // process files in directory
  int i;
  for(i = 0; i < args->length; i++) {
    char *extn = strrchr(args->files[i], '.');
    //check that file is a valid image
    if(strcmp(extn, ".gif") == 0) {  //check that file is a jpg
      gif_file = gif_file + 1; //increment counter for catalog.log
      process_file2(args->files[i]);  // call helper function to write the file
    }
  }
}

void process_bmp(void *arguments) {
  struct process_args *args = (struct process_args *)arguments;
  // process files in directory
  int i;
  for(i = 0; i < args->length; i++) {
    char *extn = strrchr(args->files[i], '.');
    //check that file is a valid image
    if(strcmp(extn, ".bmp") == 0) {  //check that file is a jpg
      bmp_file = bmp_file + 1; //increment counter for catalog.log
      process_file2(args->files[i]);  // call helper function to write the file
    }
  }
}
