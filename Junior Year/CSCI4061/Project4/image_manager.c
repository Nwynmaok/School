/* CSCI 4061 Assignment 4
   Student ID:4841729, 4872400
   Student Name:Isaac Schwab, Nathan Kaufman
   image_manager.c

   Program Description:
   The image_manager program is made up of three variants. We abstracted out our project
   into a .h and .c file for each of the variants. We also make use of a helper_func .h and .c
   that have global variables. The image_manager.c file is in charge of initializing the output
   files and calling the variant functions corresponding to the given variant argument. Once
   called the corresponding variant process functions are in charge of calling the recursive functions
   that create threads for parsing through the files. Each variant functions slightly different, so
   refer to the individual variant file comments for how they specically work.

   Refer to the README for program usage, and other details.
*/

#include "variant_1.h"
#include "variant_2.h"
#include "variant_3.h"
#include "helper_func.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

/* Main function */
int main(int argc, char* argv[]) {

  struct sigaction ta;
  memset (&ta, 0, sizeof(ta));
  ta.sa_handler = &fulltimer_handler;
  sigaction (SIGALRM, &ta, NULL);
  //timer stuff
  struct itimerval fulltimer;
  fulltimer.it_value.tv_sec = 0;
  fulltimer.it_value.tv_usec = 1000;
  fulltimer.it_interval.tv_sec = 0;
  fulltimer.it_interval.tv_usec = 1000;
  //start timer
  setitimer (ITIMER_REAL, &fulltimer, NULL);

  char* log_name = "output.log";
  char* html_name = "catalog.html";
  char* catalog_name = "catalog.log";
  char* log_path;
  char* html_path;
  char* catalog_path;

  //time of day for logging
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // initialize counter variables
  dir_traversed = 0;
  file_traversed = 0;
  jpg_file = 0;
  bmp_file = 0;
  png_file = 0;
  gif_file = 0;
  num_threads = 0;

  //check for valid number of arguments
  if(argc < 4) {
    printf("Invalid Number of Arguments.\n");
    printf("USAGE: ./image_manager [variant_options] ~/[input_path] /[output_dir]\n");
    exit(1);
  }

  /* Check for output_dir, if it doesn't exist create it */
  int result = mkdir(argv[3], 0777);

  // build path for output files
  log_path = malloc(200);
  html_path = malloc(200);
  catalog_path = malloc(200);
  strcpy(log_path, argv[3]);
  strcpy(html_path, argv[3]);
  strcpy(catalog_path, argv[3]);
  strcat(log_path, "/");
  strcat(html_path, "/");
  strcat(catalog_path, "/");
  strcat(log_path, log_name);
  strcat(html_path, html_name);
  strcat(catalog_path, catalog_name);

  // Open html and log files and timing files
  html_file = fopen(html_path, "a");
  if (html_file == NULL) {
    perror("Error - Failed to open html_file");
    exit(1);
  }

  fprintf(html_file, "<!DOCTYPE html><html><head><title>My Image Manager</title></head><body>\n");

  log_file = fopen(log_path, "a");
  if (log_file == NULL) {
    perror("Error - Failed to open log_file");
    exit(1);
  }

  catalog_file = fopen(catalog_path, "a");
  if (catalog_file == NULL) {
    perror("Error - failed to open catalog_file");
    exit(1);
  }

  fprintf(log_file, "Starting Program...\n");

  fprintf(log_file, "Created catalog.html and catalog.log\n");
  fprintf(log_file, "\n");

//begining of catalog.log
  fprintf(catalog_file, "log for %s\n", argv[1]);
  fprintf(catalog_file, "\n------------------------------------------------\n");

  // variant 1
  if(strcmp(argv[1], "v1") == 0) {
    process_var1(argv[2], argv[3]);
  }

  // variant 2
  if(strcmp(argv[1], "v2") == 0) {
    process_var2(argv[2], argv[3]);
  }

  // variant 3
  if(strcmp(argv[1], "v3") == 0) {
    process_var3(argv[2], argv[3]);
  }

  fprintf(html_file, "</body></html>\n");
  fclose(html_file);
  fprintf(log_file, "Program has completed.\n\n\n");
  fclose(log_file);

  //catalog.log finishing information
  fprintf(catalog_file, "\nProgramme initiation: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  fprintf(catalog_file, "Number of jpg files = %d\n", jpg_file);
  fprintf(catalog_file, "Number of bmp files = %d\n", bmp_file);
  fprintf(catalog_file, "Number of png files = %d\n", png_file);
  fprintf(catalog_file, "Number of gif files = %d\n", gif_file);
  fprintf(catalog_file, "\nTotal number of valid image files = %d\n", jpg_file+bmp_file+png_file+gif_file);
  fprintf(catalog_file, "\nTotal time of execution = %d ms\n", fulltime+runtime);
  fprintf(catalog_file, "--------------------------------------\n");
  fprintf(catalog_file, "number of threads created = %d\n\n", num_threads);
  fclose(catalog_file);



  return 0;
}
