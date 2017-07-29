/* CSCI 4061 Assignment 5
Student ID:4841729, 4872400
Student Name:Isaac Schwab, Nathan Kaufman
server.c

Program Description
Server : Starts by reading parameters from config file, then reads images from
the Dir parameter into catalog.csv. The server is then bound to all available
addresses on the machine it is being run from using the INADDR_ANY constant.
This means that the client must know the IP address of the server machine. Now
the server listens for client connections. On a client connect, the server waits
to receive a chunk size from the client. Then the server sends the catalog.csv
file to the client. Now the server waits for file requests from the client, and
then sends the requested files until the client disconnects. On a client
disconnect the server will go back to listening for new client connections. To
terminate the server use CRTL-C from command line.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>


#include "md5sum.h"

#define MAXPATH 1024
#define MAXFILES 500

// Declare functions used in the program

// Recursive function to read files from a directory and its subdirectory.
// Also computes MD5 checksum and files the global file variables, and writes
// to the csv file
void read_dir(char *path, FILE* csv_file);

// Sends a file to the socket. Takes as input the socket to send to, the
// fullpath of the file to read in and to send, and the chunk size
int send_file(int sock, char *file_name, int chunk);

// Sends a number to a socket. Takes as a parameter the socket to send data to,
// the size of the data we are going to send, and the actual number to send
void send_num(int socket, int num);

// Receives a number from a socket, returns the number as an integer
// Takes as parameters the socket we are receiving from, and the size
// of data we expect the server to send.
int recv_num(int socket);

// Global variables
char *files[MAXFILES];
int files_size[MAXFILES];
char *checksum[MAXFILES];
int count = 0;
char* files_to_send[MAXFILES];
int count_files = 0;

// Main function
int main(int argc, char* argv[]){
  // Program variables
  FILE* config_file;
  FILE* csv_file;
  size_t len = 0;
  ssize_t nread;
  char *line;
  char *params[2];
  int port_num;
  char* img_dir;
  char *divider = "===============================================";

  // Socket variables
  int server_socket;
  int peer_socket;
  socklen_t sock_len;
  struct sockaddr_in server_addr;
  struct sockaddr_in peer_addr;

  // check for correct number of arguments
  if(argc < 2) {
    printf("Invalid Number of Arguments.\n");
    printf("USAGE: ./server server.config\n");
    exit(1);
  }
  printf("%s\n", divider);
  printf("Server Startup...\n");

  // open config_file
  config_file = fopen(argv[1], "r");
  if (config_file == NULL) {
    perror("Error - Failed to open config_file");
    exit(1);
  }

  // read in lines from config file
  int i = 0;
  printf("Reading config_file...\n");
  while ((nread = getline(&line, &len, config_file)) != -1) {
    params[i] = malloc(100);
    strcpy(params[i],line);
    i++;
  }

  // read port
  strtok (params[0],"=");
  char *port = strtok (NULL,"= ");
  port_num = atoi(port);
  printf("Port: %d\n", port_num);
  // read img_dir
  strtok (params[1],"=");
  img_dir = strtok(NULL,"= ");
  size_t len_img = strlen(img_dir);
  // get rid of space at end
  char* path = malloc(20);
  strncpy(path, img_dir, (int)len_img-1);
  printf("Dir: %s\n", path);
  // done with config so close it
  fclose(config_file);

  // create the csv file
  csv_file = fopen("catalog.csv", "w");
  if (csv_file == NULL) {
    perror("Error - Failed to open csv_file");
    exit(1);
  }
  // write header to csv file
  fprintf(csv_file, "filename,size,checksum\n");
  // write remaining files to csv
	read_dir(path, csv_file);
  printf("catalog.csv created successful, contains %d files.\n", count_files);
  fclose(csv_file);

  // Create the server socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    fprintf(stderr, "Error creating socket - %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // 0 out the server_addr struct
  memset(&server_addr, 0, sizeof(server_addr));
  // fill in server_addr struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port_num);

  // bind the server
  if((bind(server_socket, (struct sockaddr *)&server_addr,
      sizeof(struct sockaddr))) == -1) {
    fprintf(stderr, "Error on bind - %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // listen for incoming connections
  if ((listen(server_socket, 5)) == -1) {
    fprintf(stderr, "Error on listen - %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  printf("Server now listening...\n");
  printf("%s\n", divider);

  while(1) {
    // accepting incoming client
    sock_len = sizeof(struct sockaddr_in);
    peer_socket = accept(server_socket, (struct sockaddr*)&peer_addr,&sock_len);
    if (peer_socket == -1) {
      fprintf(stderr, "Error on accept - %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Client connected - %s\n", inet_ntoa(peer_addr.sin_addr));

    // receiving chunk size
    int chunk = recv_num(peer_socket);
    if(chunk == -1) {
      printf("Failed to recieve chunk size. Exiting program...\n");
      exit(1);
    }
    printf("Receiving chunk size: %d\n", chunk);

    // get catalog.csv file info
    struct stat sb;
    if (stat("catalog.csv", &sb) == -1) {
      perror("stat");
      exit(EXIT_FAILURE);
    }
    printf("Sending file size to the client: %d\n", (int)sb.st_size);

    // sending csv file size
    send_num(peer_socket, (int)sb.st_size);

    // send the csv file
    send_file(peer_socket, "catalog.csv", chunk);

    // now wait for client to request files
    int file = 1;
    while(file > 0) {
      file = recv_num(peer_socket);
      if(file == 0) {
        break;
      }
      send_file(peer_socket, files[file-1], chunk);
    }

    close(peer_socket);
    printf("Client disconnected\n");
    printf("%s\n", divider);
  }

  close(server_socket);
  return 0;
}

void send_num(int socket, int num) {
  // checks if we are requesting a file_num
  // if -1, we do not need to send file requesting
  if(num != -1) {
    // send an int to the socket
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

int send_file(int sock, char *file_name, int chunk) {
  int sent_count;  //keep track of how many chunks we have sent
  ssize_t read_bytes;  // bytes we read from file
  ssize_t sent_bytes;  // bytes sent to connected socket
  ssize_t sent_file_size;  // size of the file
  char send_buf[chunk];  // max chunk size to send char
  int f;  // file descriptor for file to send
  sent_count = 0;
  sent_file_size = 0;
  // attempt to open requested file for reading
  if( (f = open(file_name, O_RDONLY)) < 0) {  // can't open requested file
    perror(file_name);
    return -1;
  }
  else {  // opened file successfully
    // read from file and send until end of file
    while( (read_bytes = read(f, send_buf, chunk)) > 0 ) {  // read bytes
      // send the bytes we just read
      if( (sent_bytes = send(sock, send_buf, read_bytes, 0)) < read_bytes ) {
        perror("send error");
        return -1;
      }
      sent_count++;
      sent_file_size += sent_bytes;
    }
    close(f);
  }

  printf("Sent %s, %zd bytes in %d chunks\n",
          file_name, sent_file_size, sent_count);
  return sent_count;
}


void read_dir(char *path, FILE* csv_file) {
  DIR* FD;
  struct dirent* in_file;
  unsigned char sum[MD5_DIGEST_LENGTH];
  int i = 0;
  // Scanning the input directory
  if (NULL == (FD = opendir (path))) {
    perror("Error - Failed to open input directory");
    exit(EXIT_FAILURE);
  }

  // read current directory and copy files to corresponding array
  while ((in_file = readdir(FD))) { // read each file in the directory
    // Don't get the parent and current directory names
    if (!strcmp (in_file->d_name, "."))
      continue;
    if (!strcmp (in_file->d_name, ".."))
      continue;

    // Build path
    char fullpath[300];
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, in_file->d_name); // corrected

    // encountered directory add to directory array
    if(in_file->d_type == DT_DIR) {
      read_dir(fullpath, csv_file);
    }
    else { // is a file so add to file array
      char *extn = strrchr(in_file->d_name, '.');
      if(strcmp(extn, ".jpg") == 0 || strcmp(extn, ".png") == 0 ||
          strcmp(extn, ".bmp") == 0 || strcmp(extn, ".gif") == 0) {
        // add files to array for later
        files_to_send[count_files] = malloc(strlen(fullpath)+1);
        strcpy(files_to_send[count_files], fullpath);
        count_files++;

        // get file info
        struct stat sb;
        if (stat(fullpath, &sb) == -1) {
          perror("stat");
          exit(EXIT_FAILURE);
        }
        // write to csv size and get md5 and write it
        fprintf(csv_file, "%s,", in_file->d_name);
        fprintf(csv_file, "%ld,", (long)sb.st_size);
        files[count] = malloc(1024);
        strcpy(files[count], fullpath);
        files_size[count] = (int)sb.st_size;
        // compute md5 checksum
        md5sum(fullpath, sum);
        count++;
        // add md5 to csv file
        for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
          fprintf(csv_file, "%02x", sum[i]);
        }
        fprintf(csv_file, ",\n");
      }
    }
  }
  closedir(FD); // finished reading through directory so close directory
}
