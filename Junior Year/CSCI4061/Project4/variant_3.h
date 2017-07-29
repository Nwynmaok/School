// variant_3.h

#include "helper_func.h"

#include <stdio.h>

#ifndef VARIANT_3_H
#define VARIANT_3_H

void process_file3(void* fullpath);

void *read_dir3(void *path);

// Handles the specific variant way of processing the files
int process_var3(char* input_path, char* output_path);

#endif
