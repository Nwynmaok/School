/* Information
 CSci 4061 Spring 2017 Assignment 2
 Name1=Isaac Schwab
 Name2=Nathan Kaufman
 StudentID1=4841729
 StudentID2=4872400
 Commentary=This program is used to convert images from an input directory
 and output them to an output directory. After converting it then creates linked
 HTML fils for each thumbnail in the output directory. This program uses
 parallel processes to handle images. The general flow of the code is as follows:
 -reads in the files from the input directory
 -opens logs for writing
 -converts images by running processes in parallel
 -creates linked HTML images
 Refer to the comments throughout the code and the Code Write-up.pdf for more details
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>

/* convertFile takes in a pid of the current process, the current file name string, path to input file string,
   path to output file string, and the nonImageLog file and log_file for writting.
   The function handles the actual comparision and checking of file extensions to the process id, and then
   converts, writes to a file, or returns invalid, depending on the comparision */
int convertFile(long process, char* file, char* file_in, char* file_out, FILE *bad, FILE *log);

/* getNextFile takes the fileDone array, and an int of the total number of files.
  The functions simply loops through the array looking for the next file to process.
  The first for loop checks returns the index of the first index that contains 0, meaning that the file
  has not been processed. If the function continues on to the next for loop, that means there are no
  unprocessed files. This for loop now checks for an index that contains a 2, meaning that a file is currently
  being processed, we then return a value of -1, which tells the main program that we must wait for the result
  of the files being processed before we can determine the next action. Finally, if this for loop finds no 2's
  then we know that all files have been successfully processed, so return -2, meaning the we are done. */
int getNextFile(int* fileDone, int total_files);

/* The main function handles reading in the files, process creation, waiting, and html creation*/
int main (int argc, char *argv[]) {
  /* Declarartion of variables */
	// Variables for files and directories
  DIR* FD;
	struct dirent* in_file;
	FILE *nonImageLog;
  FILE *log_file;
  FILE *html_file;

  /* Process variables */
  pid_t mainpid = getpid();
  int max_process = atoi(argv[1]);  // argument for max number of processes allowed, valid range (1-10)
  int av_process = atoi(argv[1]);  // current number of processes available, initialize to user input, (1-10)
  pid_t pids[max_process];  // array to hold process pid values
  int processFile[max_process];  //array to hold index to current working file for each process
  int current_process = 0;  //  index of the current_process for the main parent process


  /* File name variables */
	char* output = argv[2];
	char* input = argv[3];
	char* file_in_name;
	char* file_out_name;
  char* nonImage;
  char* log_name;
  char* html_name;
  char files[100][100];
  char html_files[100][100];
  char file_list[100][100];
  char file_list_out[100][100];

  /* Counters and indexs */
  int total_files = 0;
  int files_left;
  int current_file = 0;
  int totalCreated = 0;
  int waits = 0;


	/* Check for valid number of arguments */
	if(argc != 4) {
		printf("Invalid number of arguments, exiting program\n");
    printf("usage: parallel_convert convert_count output_dir input_dir");
		return 1;
	}

  /* Check for valid number of arguments */
	if(max_process < 1 || max_process > 10) {
		printf("Invalid convert_count, must be in the range of 1-10, exiting program\n");
		return 1;
	}

  /* Checking intial arguments */
  printf("\n");
	printf("Program Starting...\n");
  printf("\n");
	printf("Input Arguments:\n");
	printf("%s\n", argv[0]);
	printf("%s\n", argv[1]);
	printf("%s\n", argv[2]);
	printf("%s\n", argv[3]);
  printf("\n");

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


	/* Scanning the input directory */
  if (NULL == (FD = opendir (argv[3]))) {
    perror("Error - Failed to open input directory");
    return 1;
  }

  total_files = 0;
  while ((in_file = readdir(FD))) { // read eadch file in the directory
    /* Don't get the parent and current directory names */
    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    /* Initialize the strings we will build */
    file_in_name = malloc(100);
    file_out_name = malloc(100);
    nonImage = malloc(100);
    log_name = malloc(100);

    /* Create the string for the input filename */
    strcpy(file_in_name, input);  // add output path to string
    strcat(file_in_name, "/");
    strcat(file_in_name, in_file->d_name);  // add filename to string

    /* Create string for output filename */
    strcpy(file_out_name, output);  // add output path to string
    strcat(file_out_name, "/");

    /* Remove the extension from the filename */
    size_t len = strlen(in_file->d_name);
    char *basename = malloc(len-3);
    if (!basename); /* handle error */
    memcpy(basename, in_file->d_name, len-4);
    basename[len - 4] = 0;

    /* Add the basename of the file to the path */
    strcat(file_out_name, basename);
    strcat(file_out_name, "_");

    /* Add the completed file strings to the respective arrays */
    strcpy(file_list[total_files], file_in_name);
    strcpy(file_list_out[total_files], file_out_name);
    strcpy(files[total_files], in_file->d_name);
    total_files = total_files+1;
    free(basename);
  }

  /* If the input directory contains 0 images we can just end the program */
  if(total_files == 0) {
    printf("No files exist in: %s\n", input);
    printf("Exiting program\n");
    return 0;
  }

  /* Create array to hold the status of the files, initialize to 0 */
  int fileDone[total_files];
  int y;
  for(y = 0; y<total_files; y++) {
  	fileDone[y] = 0;
  }

  /* initialize processFile, each will get a different file, up to max_process */
  for(y = 0; y < max_process; y++) {
  	processFile[y] = y;
  }

  /* Open nonImage.txt file for writing */
  strcpy(nonImage, output);  // add output path to string
  strcat(nonImage, "/");
  strcat(nonImage, "nonImage.txt");

  /* Open log.txt file for writing */
  strcpy(log_name, output);  // add output path to string
  strcat(log_name, "/");
  strcat(log_name, "log.txt");
  log_file = fopen(log_name, "w");
  if (log_file == NULL) {
    fprintf(stderr, "Error : Failed to open log_file - %s\n", strerror(errno));
    perror("Error - Failed to open log_file");
    return 1;
  }

  nonImageLog = fopen(nonImage, "w");
  if (nonImageLog == NULL) {
    fprintf(stderr, "Error : Failed to open log_f - %s\n", strerror(errno));
    perror("Error - Failed to nonImage");
    return 1;
  }

  /* If we have less files then convert_count, we just set the convert_count to the number of files
     because there is no benefit to having more processes than files */
  if(total_files < max_process) {
    max_process = total_files;
    av_process = total_files;
  }

  /* This section handles the creation of multiple processes in parallel */
  files_left = total_files;
  while (files_left >= 0) {  // The main while loop
    /* Loop and create child processes until there are no more available processes */
    while (av_process > 0 && getpid() == mainpid) {
      current_file = processFile[current_process];  // Get the current file to process for the current_process
      fileDone[current_file] = 2;  // Sets the new forked file to 2, representing it is blocked while being processed
      /* Create child process and set its pid in the pids[] */
      if ((pids[current_process] = fork()) < 0) {  //  Handle case in which fork() returns incorectly
        perror("Error - Fork failed");
        abort();
      }
      else if (pids[current_process] == 0) {  // This code will be executed only in the child processes
        printf("Created Child with PID %ld, and process: %d, file: %s\n", (long)getpid(), current_process, file_list[current_file]);
        /* call function to process current file, if it can return zero, if not return 1 in exit */
        int success = convertFile((long)getpid(), files[current_file], file_list[current_file], file_list_out[current_file], nonImageLog, log_file);
        exit(success); // kill child process
      }
      /* Main process modifies the counters to reflect the creation of a new process */
      totalCreated++;
      current_process++;
      av_process--;
    }
    printf("\n");

    /* Checks the current state  */
    if(getNextFile(fileDone, total_files) == -2) {
      files_left = -1;
      break;
    }

    /* Wait for children to exit. */
    int status;
    pid_t pid;
    int nextFile;
    if (av_process == 0) { // all processes are being run, wait for child to finish
      printf("All processes are used, WAITING for child process to termintate...\n");
      pid = wait(&status);
      int i;
      for(i = 0; i<max_process; i++) {  //check which child finished
        if(pids[i] == pid) {
          printf("Child with PID %ld exited with status 0x%x, for filename: %s\n", (long)pid, status, files[processFile[i]]);
          if(status == 0) {  //child finished with success, set process to next unprocessed file
            fileDone[processFile[i]] = 1;  // file was converted successfully, so set file status to complete
            waits++;
            nextFile = getNextFile(fileDone, total_files);
            if(nextFile < 0) {
              av_process = nextFile;
              break;
            }
            processFile[i]=nextFile;
            files_left--;
          }
          else {  // child did NOT finish with success, set file to unprocessed
            fileDone[processFile[i]] = 0;
            waits++;
          }
          current_process = i;  // set the current_process to the process that just returned
          av_process++;
        }
      }
    }
    else if(av_process == -1) {  //need to wait for unfinished child processes after there are no more untouched files
      printf("All processes are used, WAITING for child process to termintate...\n");
      pid = wait(&status);
      int i;
      for(i = 0; i<max_process; i++) {
        if(pids[i] == pid) {
          printf("Child with PID %ld exited with status 0x%x, for filename: %s\n", (long)pid, status, files[processFile[i]]);
          if(status == 0)
          {
            fileDone[processFile[i]] = 1;
            files_left--;
            waits++;
            continue;
          }
          else {
            fileDone[processFile[i]] = 0;
            waits++;
          }
          current_process = i;
          av_process = 1;
        }
      }
    }
    printf("\n");
  }

  /* Some status output for the files that were converted */
  printf("Files are done being converted, here is the completed files and status: \n");
  int j;
  for(j = 0; j < total_files; j++) {
    printf("%s  Status: %d\n", files[j], fileDone[j]);
  }
  printf("\n");
  printf("Total Created: %d\n", totalCreated);
  printf("Waits retutned: %d\n", waits);
  printf("\n");
  fprintf(log_file, "All files were successfully completed\n");

  /* Close log files */
  fclose(log_file);
  fclose(nonImageLog);


  /* HTML File Creation */
  printf("Creating html files...\n");
  /* Scanning the output directory */
  int total_html = 0;
  if (NULL == (FD = opendir (argv[2]))) {
    fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
    perror("Error - Failed to open ouput directory to read");
    return 1;
  }

  /* modify the output dir variable so that it no longer has "./" if it is there */
  if(strstr(output, "./") != NULL) {
    size_t len = strlen(output);
    memmove(output, output+2, len - 2 + 1);
  }

  /* Read files from the output directory */
  while ((in_file = readdir(FD))) {
    /* Don't get the parent and current directory names */
    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    /* Checking that all files to process html for are jpg */
    char *extn = strrchr(in_file->d_name, '.');
    if(strcmp(extn, ".jpg") != 0) {
      continue;
    }

    /* build html file name */
    html_name = malloc(200);
    getcwd(html_name, (size_t)html_name);
    strcat(html_name, "/");
    strcat(html_name, output);  // add output path to string
    strcat(html_name, "/");

    /* remove the extension from the filename */
    size_t len = strlen(in_file->d_name);
    char *basename = malloc(len-3);
    if (!basename); /* handle error */
    memcpy(basename, in_file->d_name, len-4);
    basename[len - 4] = 0;
    strcat(html_name, basename);
    strcat(html_name, ".html");
    strcpy(html_files[total_html], html_name);
    total_html++;
    free(basename);
  }

  /* Now build the html file */
  int h;
  for(h = 0; h < total_html; h++) {
    // Open the current html file
    html_file = fopen(html_files[h], "w");
    if (log_file == NULL) {
      fprintf(stderr, "Error : Failed to open log_file - %s\n", strerror(errno));
      return 1;
    }

    /* allocate strings to be used to build html */
    char *nextfile = malloc(200);
    char *image = malloc(200);
    strcpy(nextfile, "<meta http-equiv='refresh' content=\"2;URL=file:///");
    if(h == total_html - 1) {
      strcat(nextfile, html_files[0]);
    }
    else {
      strcat(nextfile, html_files[h+1]);
    }
    strcat(nextfile, "\">");
    strcpy(image, "<img src=\"");

    /* remove the extension from the filename */
    size_t len = strlen(html_files[h]);
    char *basename = malloc(len-4);
    if (!basename); /* handle error */
    memcpy(basename, html_files[h], len-5);
    basename[len - 5] = 0;

    /* Append basename and jpg extension for to image string */
    strcat(image, basename);
    strcat(image, ".jpg");
    strcat(image, "\">");

    /* Add strings to html file */
    fprintf(html_file, "<!DOCTYPE html><html><head></head><body>\n");
    fprintf(html_file, image);
    fprintf(html_file, nextfile);
    fprintf(html_file, "</body></html>\n");
    fclose(html_file);
    free(basename);
    free(nextfile);
    free(image);
  }
  printf("HTML file creation complete\n");
  printf("\n");

  /* cleanup pointers used in program */
  free(file_in_name);
  free(file_out_name);
  free(nonImage);
  free(log_name);
  free(html_name);

  printf("Program Complete\n");
  return 0;
}


int convertFile(long process, char* file, char* file_in, char* file_out, FILE *bad, FILE *log) {
  /* Finish building the output filename, now that we have the process id */
  char pid[10];  // allocate a string for the process id
  snprintf(pid, 10,"%ld",(long)getpid());  // covert process id to string
  strcat(file_out, pid);  // attach pid to string
  strcat(file_out, ".jpg");  // attach extension

  /* Get the extension of the file */
  char *extn = strrchr(file_in, '.');
  /* Check if the file is the correct file to convert */
  if(strcmp(extn, ".png") == 0 || strcmp(extn, ".bmp") == 0 || strcmp(extn, ".gif") == 0
      || strcmp(extn, ".PNG") == 0 || strcmp(extn, ".BMP") == 0 || strcmp(extn, ".GIF") == 0) {

    if(process % 2 == 0 && process % 3 == 0 && (strcmp(extn, ".gif") == 0 || strcmp(extn, ".GIF") == 0)) {
      fprintf(log, "%s converted to jpg of size 200x200 by process with id: %ld\n", file, process);
      printf("%s converted to jpg of size 200x200 by process with id: %ld\n", file, process);
      fflush(log);
      return execl("/usr/bin/convert", "convert", file_in, "-resize", "200x200", file_out, NULL);
    }
    if(process % 2 == 0 && (strcmp(extn, ".png") == 0 || strcmp(extn, ".PNG") == 0)) {
      fprintf(log, "%s converted to jpg of size 200x200 by process with id: %ld\n", file, process);
      printf("%s converted to jpg of size 200x200 by process with id: %ld\n", file, process);
      fflush(log);
      return execl("/usr/bin/convert", "convert", file_in, "-resize", "200x200", file_out, NULL);
    }
    else if(process % 3 == 0 && (strcmp(extn, ".bmp") == 0 || strcmp(extn, ".BMP") == 0)) {
      fprintf(log, "%s converted to jpg of size 200x200 by process with id: %ld\n", file, process);
      printf("%s converted to jpg of size 200x200 by process with id: %ld\n", file, process);
      fflush(log);
      return execl("/usr/bin/convert", "convert", file_in, "-resize",  "200x200", file_out, NULL);
    }
    else {
      printf("do nothing, Process: %ld\n", process);
      return 1;
    }
  }
  else {
    /* add image to log */
    if(process % 2 == 0 || process % 3 == 0) {
      printf("divisible by 2 or 3, write to log, Process: %ld\n", process);
      fprintf(log, "%s is invalid, written to nonImage.txt by process with id: %ld\n", file, process);
      fprintf(bad, "%s\n", file);
      remove(file_in); // remove the invalid file from the input directory
      return 0;
    }
    else {
      /* was an invalid file and we can't process it because pid didn't match one of the given criteria*/
      return 1;
    }
  }
}


int getNextFile(int* fileDone, int total_files) {
	int j;
    /* check for any 0's, which are unprocessed files */
    for(j = 0; j < total_files; j++) {
    	if(fileDone[j] == 0) {
    		return j;
    	}
    }

    /* if we get here it means all files have processed or haven't been killed */
    for(j = 0; j < total_files; j++) {
      if(fileDone[j] == 2) {
        return -1;
      }
    }

    /* if we get here then all files have been processed successfully */
    return -2;
}
