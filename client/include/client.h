/*
 * File name: client.h
 * Functionality: contains data structures and function prototypes
 * used by the client.
 */
 #include <stdio.h>
 #include <pthread.h>

 //data structures and global variables
struct node_info{
  char hostname[30];
  int fd;
  struct node_info *next;
};

struct node_info *head;
pthread_t ptid[10];
int num;
int server_fds[5];
int server_fd_index;
int mserver_fd;


//Function prototypes
int peer_connect( char *ip_address);
void insert_node_info(char *hstnm, int fd);
int read_operation(char *filename, char *offset);
int append_operation(char *filename);
int create_operation(char *filename);
int get_file_server_fd(char *hstnm);
