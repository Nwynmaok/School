// helper_func.h

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#ifndef HELPER_FUNC_H
#define HELPER_FUNC_H

#define MAXFILES 128

// Declare files
FILE* html_file;
FILE* log_file;
FILE* catalog_file;

// Declare Mutexs
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dir_lock = PTHREAD_MUTEX_INITIALIZER;

//logging global var
int dir_traversed;
int file_traversed;
int jpg_file;
int bmp_file;
int png_file;
int gif_file;
int num_threads;
int runtime;
int fulltime;

//logging timer handler
void timer_handler (int signum);
void fulltimer_handler (int signum);

#endif
