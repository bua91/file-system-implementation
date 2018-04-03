/*
 * File name: client.h
 * Functionality: contains data structures and function prototypes 
 * used by the client.
 */
#include <stdio.h>

//Data structures
struct files_hosted{
	char * filename;
	char * last_line;
	struct files_hosted * next;
};

struct timestamp{
	int socket_fd;
	char cli_id[30];
	long int t_stamp;
	//char filename[20];
	struct timestamp * next;
};

int server_fds[3];
struct timestamp *head;
long int local_clock;

// Function prototypes
int server();
int peer_connect( char *ip_address);
int show_hosted_files();
void * peer_conn(void * arg);
int connect( char *ip_addr);
void client_send_request(int sock_fd);
