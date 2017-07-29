# 4061_HW5
Isaac Schwab
Nathan Kaufman

Tested on machines: Lind40-34, KHK1250-13

Program Description:
This program contains a server and client that communicate via Sockets.
Both the client and server read parameters from a .config file.

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


Server Usage:
run "make" from Server directory
./server server.config

Client Usage:
run "make" from Client directory
./client client.config


Example client.config:
Server = 127.0.0.1
Port = 56432
Chunk_Size = 500
ImageType = jpg

Example server.config:
Port = 56432
Dir = images
