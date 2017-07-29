// variant_3.c

#include "variant_3.h"

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
 int process_var3(char* input_path, char* output_path) {
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

   //tid for linux
   pthread_t tid;
   int retval;
   pid_t tid2 = syscall(__NR_gettid);
   fprintf(log_file, "Starting Variant 3 \n");
   // create thread for the starting directory
   if(pthread_create(&tid, NULL, read_dir3, (void*)input_path)) {
     fprintf(log_file, "FAILED to create thread\n");
     exit(EXIT_FAILURE);
   }
   num_threads = num_threads + 1;
  //  printf("Created Thread: %s\n", input_path);
   fprintf(log_file, "Created Thread: %s\n", input_path);
   // wait for thread to finish
   pthread_join(tid, retval);
   return 0;
  }

// handles actually writing the image file information to the html and log files
void process_file3(void* fullpath) {
  // create nutex lock for writing to files
  int error;
  if(error = pthread_mutex_lock(&lock)) {
    perror(error);
  }
  // cast fullpath back to char
  fullpath = (char*)fullpath;
  //increment files traversed counter
  file_traversed = file_traversed + 1; //increment files traversed for logging
  //get the current thread id
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

  // Add to html_file
  char* temp = malloc(200);
  strcpy(temp, "../");
  strcat(temp, fullpath);
  // printf("Process file: %s tid: %ld\n", fullpath, (long) curthread);
  fprintf(html_file, "<a href='%s'><img src='%s' width=100 height=100/></a>\n", temp, temp);
  fprintf(html_file, "<p align='left'> iNode: %d, Type: %d, Size: %ld bytes, Last Modified: %s, Thread Id: %ld</p>\n", (int)sb.st_ino, (int)sb.st_mode, (long)sb.st_size, timbuf, (long)curthread);
  fprintf(log_file,"Process file: %s tid: %ld\n", fullpath, (long) curthread);
  // now unlock the mutex because we are done writing to the files
  pthread_mutex_unlock(&lock);
}


/* Recursive Helper function, used to scan directorys recursivly. If we encounter a
  file that is not a directory that we call writeFile, if it is a directory, then we recursivly
  call this function again with the directory as the arguement*/
void *read_dir3(void *path) {
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
  // process files in directory
  for(i = 0; i < num_files; i++) {
    char *extn = strrchr(files[i], '.');
    //check that file is a valid image
    if(strcmp(extn, ".jpg") != 0 && strcmp(extn, ".png") != 0 && strcmp(extn, ".bmp") != 0 && strcmp(extn, ".gif") != 0) {
     continue;
    }
    //counter for types of files
    if(strcmp(extn, ".jpg") == 0) {
      jpg_file = jpg_file + 1;
    }
    if(strcmp(extn, ".bmp") == 0) {
      bmp_file = bmp_file + 1;
    }
    if(strcmp(extn, ".png") == 0) {
      png_file = png_file + 1;
    }
    if(strcmp(extn, ".gif") == 0) {
      gif_file = gif_file + 1;
    }

    // declare a thread id for the current file
    pthread_t file_tid;
    // create thread for the starting directory
    if(pthread_create(&file_tid, NULL, process_file3, (void*)files[i])) {
      fprintf(log_file, "FAILED to create thread\n");
      exit(EXIT_FAILURE);
    }
    num_threads = num_threads + 1;
    pthread_join(file_tid, NULL);
  }

  // array of thread ids for each thread we create
  pthread_t tids[num_dirs];
  // recursivly handle directories
  for(i = 0; i < num_dirs; i++) {
    // call new directory in a new thread per variant_1 requirements
    if(pthread_create(&tids[i], NULL, read_dir3, (void*)directories[i])) {
      fprintf(log_file, "FAILED to create thread\n");
      exit(EXIT_FAILURE);
    }
    num_threads = num_threads + 1;
    printf("Created Thread: %s\n", directories[i]);
    fprintf(log_file, "Created Thread: %s\n", directories[i]);

  }
  // wait for each thread to finish
  for(i = 0; i < num_dirs; i++) {
    pthread_join(tids[i], NULL);
  }


}
