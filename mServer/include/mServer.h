/*
 * File name: mServer.h
 * Functionality: contains data structures and function prototypes
 * used by the m-server.
 */
 #include <stdio.h>
 #include <sys/select.h>

//Metadata data structures
struct metadata{
  char host_name[30];
  char file_name[20];
  int chunk_id;
  //to keep track of which server hosts which chunk, we store the connection fd of that server connections
  int fd;
  struct metadata *next;
};
struct metadata *head;
fd_set readfds;
int conn_fds[10];
int server_fds[5];
int server_fd_index;
int client_fds[5];
int client_fd_index;
int max_clients;

//Function prototypes
int insert_into_metadata(char *hst_name, char *filenm, char *chunkid);
int get_chunk_id_from_offset(char *filename, char *offset);
void get_host_name(char *filename, int chunkid);
int server();
int show_hosted_files();
