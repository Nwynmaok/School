// variant_2.h

#include "helper_func.h"

#include <stdio.h>

#ifndef VARIANT_2_H
#define VARIANT_2_H


struct process_args {
  char* files[MAXFILES];
  int length;
};


void process_file1(char* fullpath);

void process_png(void* args);
void process_jpg(void* files);
void process_bmp(void* files);
void process_gif(void* files);

void *read_dir2(void *path);

// Handles the specific variant way of processing the files
int process_var2(char* input_path, char* output_path);

#endif
