/*
 * File name: server.h
 * Functionality: contains data structures and function prototypes
 * used by the server.
 */
 #include <stdio.h>
 #include <sys/select.h>

 //Metadata data structures
 struct metadata{
  char file_name[20];
  int chunk_id;
  //to keep track of which server hosts which chunk, we store the connection fd of that server connections
  int fd;
  struct metadata *next;
 };

 struct metadata *head;
 fd_set readfds;
 int max_clients;
 int conn_fds[3];
 int client_fds[3];
 int client_fd_index;


 //Function prototypes
 int peer_connect(char *ip_address);
 int insert_into_metadata(char *filenm, char *chunkid);
 int server();
 int show_hosted_files();
