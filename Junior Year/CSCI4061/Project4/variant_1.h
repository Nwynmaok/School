// variant_1.h
#include "helper_func.h"

#include <stdio.h>

#ifndef VARIANT_1_H
#define VARIANT_1_H

// Handles actually getting the information from the file and writing it to the html file
void process_file1(char* fullpath);

// Handles recursively reading the directory structure, defined as a threaded function
void *read_dir1(void *path);

// Handles the specific variant way of processing the files
int process_var1(char* input_path, char* output_path);

#endif
