/* CSCI 4061 Assignment 5
Student ID:4841729, 4872400
Student Name:Isaac Schwab, Nathan Kaufman
client.c

Program Description:
Client : Starts by reading the parameters from the config file. Then creates a
client socket, and connects it to the server IP address that was read from the
config file. After successfully connecting to the server, the client sends the
server the chunk size to be used for file transfers. Then the client receives
the catalog.csv file from the server. We parse the csv file into global
variables to be used later in the program. After successfully downloading the
csv file, the client moves into either interactive or passive mode, depending on
if there was an ImageType field in the config file. While in download mode, the
client sends a file number request to the server and then downloads it. This
continues until the user enters 0, or until passive mode downloads the correct
image types. Note: we implemented error checking in interactive mode, so invalid
input should not break the program. Finally, we compute the MD5 checksum for
downloaded files, and then generate the download.html for the downloaded files.
The client then disconnects from the server, and then the program exits.

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "md5sum.h"

#define MAXFILES 500
#define MAXPATH 1024


// Receives a number from a socket, returns the number as an integer
// Takes as parameters the socket we are receiving from, and the size
// of data we expect the server to send.
int recv_num(int socket);

// Sends a number to a socket. Takes as a parameter the socket to send data to,
// the size of the data we are going to send, and the actual number to send
void send_num(int socket, int num);

// Receives a file from a socket. Takes as a parameter the socket to receive
// from, the file_name we are requesting, the index of the file from the
// catalog.csv, and the chunk size that the file will be sent/received
int recv_file(int sock, char* file_name, int file, int filesize, int chunk);

// Global variables
char *files[MAXFILES];
int files_size[MAXFILES];
char *checksum[MAXFILES];

// Main function
int main(int argc, char* argv[]) {
  // Program variables
  int num_files = -1;  // counts files from csv file, ignore header, thus -1
  FILE* config_file;
  FILE* csv;
  size_t len = 0;
  ssize_t nread;
  char *line;
  char *params[4];
  char *divider = "===============================================";
  int interactive = 0;
  int port_num;
  char *ip_addr;

  // socket variables
  int client_socket;
  struct sockaddr_in server_addr;
  socklen_t addr_size;

  // check for correct number of arguments
  if(argc < 2) {
    printf("Invalid Number of Arguments.\n");
    printf("USAGE: ./client client.config\n");
    exit(1);
  }

  // Check for images directory, if it doesn't exist create it
  mkdir("images", 0777);


  // open config_file
  config_file = fopen(argv[1], "r");
  if (config_file == NULL) {
    perror("Error - Failed to open config_file");
    exit(1);
  }

  // read parameters from config file
  int i;
  for(i = 0; i < 4; i++) {
    params[i] = malloc(100);
    strcpy(params[i], "NULL");  // first intialize params array to NULL
  }
  i = 0;
  while ((nread = getline(&line, &len, config_file)) != -1) {
    params[i] = malloc(100);
    strcpy(params[i],line);
    i++;
  }

  //get server ip
  strtok (params[0],"=");
  ip_addr = strtok (NULL,"= ");
  //get rid of space at end
  size_t len_ip = strlen(ip_addr);
  char* ip = malloc(20);
  strncpy(ip, ip_addr, (int)len_ip-1);

  //get server port
  strtok (params[1],"=");
  char *port = strtok (NULL,"= ");
  port_num = atoi(port);

  //get the chunk size
  strtok (params[2],"=");
  char *chunk_str = strtok (NULL,"= ");
  const int chunk = atoi(chunk_str);

  //check for interactive or passive mode
  char *img_type;
  if(strcmp(params[3], "NULL") == 0) {
    interactive = 1;
  }
  else {
    strtok (params[3],"=");
    img_type = strtok (NULL,"= ");
    strtok(img_type, "\n");
  }

  fclose(config_file);

  // Output
  printf("%s\n", divider);
  printf("Connecting server at %s, port %d\n", ip, port_num);
  printf("Chunk size is %d bytes. ", chunk);
  if(interactive) {
    printf("No image type found\n");
  }
  else {
    printf("Image type is %s\n", img_type);
  }

  // create the client socket
  client_socket = socket(PF_INET, SOCK_STREAM, 0);

  // fill in server_addr struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_num);
  server_addr.sin_addr.s_addr = inet_addr(ip_addr);
  // zero of padding bits
  memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

  // connect client to server
  addr_size = sizeof server_addr;
  int success = -1;
  int attempts = 0;
  while(success == -1 && attempts < 10) {
    success = connect(client_socket, (struct sockaddr*)&server_addr, addr_size);
    if(attempts == 9) {
      printf("\nConnection timed out, check connection parameters and"
              "that the server is running. Exiting program.\n");
      exit(1);
    }
    attempts++;
  };

  // send the chunk size to the server
  send_num(client_socket, chunk);


  // receiving file size
  int file_size = recv_num(client_socket);
  if(file_size == -1) {
    printf("Failed to recieve file size. Exiting program...\n");
    exit(1);
  }


  // get CSV file from server
  int complete = recv_file(client_socket, "catalog.csv", -1, file_size, chunk);
  if(complete == -1) {
    printf("Failed to recieve CSV file. Exiting program...\n");
    exit(1);
  }


  // now dump contents of CSV file
  printf("%s\n", divider);
  // open downloaded csv file in read mode
  csv = fopen("catalog.csv", "r");
  if (csv == NULL) {
    fprintf(stderr, "Failed to open file catalog.csv -%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("Dumping contents of catalog.csv\n");
  char *temp;  // temp variable to hold contents of the line
  while ((nread = getline(&line, &len, csv)) != -1) {
    if(num_files == -1) {
      num_files++;
      continue;
    }
    files[num_files] = malloc(256);
    checksum[num_files] = malloc(256);
    temp = malloc(1024);
    strcpy(temp,line);
    //get filename
    files[num_files] = strtok (temp,",");
    printf("[%d] %s\n", num_files+1, files[num_files]);

    //get filesize
    files_size[num_files] = atoi(strtok (NULL,","));

    // get filename
    checksum[num_files] = strtok (NULL,",");

    num_files++;
  }

  // check that there are files to process
  if(num_files == 0) {
    printf("No files to process, exiting program...\n");
    exit(0);
  }

  // Now dowmload files from server
  char *download_files[num_files];  // holds the filenames the user downloads
  char *server_md5[num_files];  // holds the corresponding md5 for the file
  int index = 0;
  if(interactive) {  // interactive mode
    int input = -1;
    while(input != 0) {
      printf("Enter ID to download (0 to quit): ");
      int result = scanf("%d", &input);
      if (result == 0) {
        while (fgetc(stdin) != '\n') // Read until a newline is found
            ;
      }
      if(input > num_files || input < 0) { //handle invalid user input
        printf("Invalid file, try again...\n");
        input = -1;
        continue;
      }
      if(input == 0) {
        send_num(client_socket, 0);
        break;
      }
      //send file request to server
      char file[256];
      sprintf(file, "%d", input);
      recv_file(client_socket, files[input-1], input, files_size[input-1], chunk);
      download_files[index] = malloc(1024);
      server_md5[index] = malloc(1024);
      strcpy(download_files[index], files[input-1]);
      strcpy(server_md5[index], checksum[input-1]);
      index++;
      input = -1;
    }

  }
  else {  // passive mode
    printf("Running in passive mode... Downloading '%s' files...\n", img_type);
    int x;
    char *type = malloc(5);
    strcpy(type, ".");
    strcat(type, img_type);
    for(x = 0; x < num_files; x++) {
      char *extn = strrchr(files[x], '.');
      if(strcmp(extn, type) == 0) {
        recv_file(client_socket, files[x], x+1, files_size[x], chunk);
        download_files[index] = malloc(1024);
        server_md5[index] = malloc(1024);
        strcpy(download_files[index], files[x]);
        strcpy(server_md5[index], checksum[x]);
        index++;
      }
    }
    // done requesting files so tell server
    send_num(client_socket, 0);
  }

  // compute and check MD5 for files we just downloaded
  printf("Computing checksums for downloaded files...\n");
  int y;
  char *download_md5[index];  // holds the md5 for the downloaded file
  unsigned char sum[MD5_DIGEST_LENGTH];
  char *fullpath = malloc(MAXPATH);
  char c[10];
  for(y = 0; y < index; y++) {
    strcpy(fullpath, "images/");
    strcat(fullpath, download_files[y]);  // fullpath to downloaded image
    md5sum(fullpath, sum);  // compute md5 checksum
    download_md5[y] = malloc(256);
    memset(download_md5[y], 0, index);
    for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
      sprintf(c, "%02x", sum[i]);
      strcat(download_md5[y], c);
    }
  }

  // close connection to server
  close(client_socket);

  // html_file generation
  printf("Generating HTML files...\n");
  FILE* html_file;
  html_file = fopen("download.html", "w");
  fprintf(html_file, "<!DOCTYPE html>\n<html>\n\t"
                      "<head>\n\t\t<title>Downloaded images</title>\n\t</head>"
                      "\n\t<body>\n\t<h1> Downloaded images</h1>\n\t<pre>\n");
  // final html file & close
  // html file checksum + url
  for(y = 0; y < index; y++) {
    if(strcmp(download_md5[y],server_md5[y]) == 0) {
      fprintf(html_file, "(checksum match!)\t");
      fprintf(html_file, "<a href=\"images/%s\">%s</a><br />\n",
              download_files[y], download_files[y]);
    }
    else {
      fprintf(html_file, "(checksum mismatch!)\t");
      fprintf(html_file, "%s</a><br />\n", download_files[y]);
    }

  }
  fprintf(html_file, "\t</pre>\n\t</body>\n</html>");
  fclose(html_file);

  // end of program
  return 0;
}

// function implementations

int recv_file(int sock, char* file_name, int file, int filesize, int chunk) {
  int f;  // file descriptor for the file to download
  ssize_t rcvd_bytes, rcvd_file_size;  // sizes
  int recv_count;  // count of chunks reveived
  char recv_str[chunk];  // buffer to hold received file data */
  send_num(sock, file);  // send file request to server
  char *dir = malloc(MAXPATH);
  if(strcmp(file_name, "catalog.csv") == 0) {  //handle if file is csv
    strcpy(dir, file_name);
  }
  else {  //all other files go to images dir
    strcpy(dir, "images/");
    strcat(dir, file_name);
  }
  // attempt to create file to save received data
  if ( (f = open(dir, O_WRONLY|O_TRUNC, 0644)) < 0 ) {
    perror("error creating file");
    return -1;
  }
  recv_count = 0;
  rcvd_file_size = 0;  // current size of received file
  // continue receiving until recvd_file_size <= filesize
  while ( (rcvd_bytes = recv(sock, recv_str, chunk, 0)) > 0 ) {
    recv_count++;
    rcvd_file_size += rcvd_bytes;
    // write the data to the file
    if (write(f, recv_str, rcvd_bytes) < 0 ) {
      perror("error writing to file");
      return -1;
    }
    if (rcvd_file_size >= filesize) {
      break;
    }
  }
  close(f);  // close file
  printf("Downloaded %s, %d chunks transmitted.\n", file_name, recv_count);
  return rcvd_file_size;
}


int recv_num(int socket) {
  char recv_str[MAXPATH];  // will hold the string we receive
  ssize_t rcvd_bytes;  // number of bytes that we have received
  // read the integer in string form form the socket
  if ( (rcvd_bytes = recv(socket, recv_str, MAXPATH, 0)) < 0) {
    perror("recv error");
    return -1;
  }
  return atoi(recv_str);  // convert from string back to int
}


void send_num(int socket, int num) {
  //checks if we are requesting a file_num
  // if -1, we do not need to send file requesting
  if(num != -1) {
    //send an int to the server
    char num_str[MAXPATH];
    sprintf(num_str, "%d", num);
    ssize_t send_len;
    send_len = send(socket, num_str, MAXPATH, 0);
    if (send_len < 0) {
      fprintf(stderr, "Error on sending number --> %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
}
