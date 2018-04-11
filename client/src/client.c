/*
 * FILE NAME: client.c
 * OWNER: ARUNABHA CHAKRABORTY
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "client.h"

void insert_node_info(char *hstnm, int fd)
{
	struct node_info *temp = (struct node_info*) malloc(sizeof(struct node_info));

  //if metadata not in metadata table, then insert
	strcpy(temp->hostname, hstnm);
	temp->fd = fd;
	temp->next = head;
	head = temp;

	return 1;
}
/*
 * Client functionality part of client node
 */
int peer_connect(char *ip_address)
{
  int sock_fd;
	struct sockaddr_in server_addr;
	char send_buffer[2048] = {0};
	char recv_buffer[2048] = {0};
  char sen_buf[30] = {0};
  char rec_buf[30] = {0};
	int i, m, rd;
	int random = 0;
	int check = 0;
	int release_check = 0;
	strcpy(send_buffer, "client");
  strcpy(sen_buf, "hostname");

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    fprintf(stderr, "clientsh(CLIENT PART): Error in socket creation!!\n");
    return 0;
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(9000);

	if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0){
		fprintf(stderr, "clientsh(CLIENT PART): Error in IP address!!\n");
		return 0;
	}

	if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		fprintf(stderr, "clientsh(CLIENT PART): Error in socket connect!!\n");
		return 0;
	}

	//send hello to the new mserver/server
	if (send(sock_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending hello to other client!!\n");
	}

	//read the greeting reply from the other servers and mserver
	rd = read(sock_fd, recv_buffer, 2048);
	if ((strncmp(recv_buffer, "server", 6) == 0)){
		for ( m = 0; m<5; m++){
			if(server_fds[m] == 0){
				server_fds[m] = sock_fd;
        server_fd_index++;
			}
		}
	}
  else if ((strncmp(recv_buffer, "mserver", 7) == 0)){
    mserver_fd = sock_fd;
  }

  //Ask for hostname
  if (send(sock_fd, sen_buf, strlen(sen_buf), 0) < 0){
		fprintf(stderr,"clientsh: error in sending hostname request to other client!!\n");
	}
  //get hostname
  rd = read(sock_fd, rec_buf, 100);
  //insert node info
  insert_node_info(rec_buf, sock_fd);


	while (1){
    //infinite loop to let the client socket open untill the program ends
	}

	close(sock_fd);
	return 1;
}

/*
 * get file server socket fd from the hostname
 */
int get_file_server_fd(char *hstnm){
  struct node_info *current = head;
  while (current != NULL)
  {
      if (!strcmp(current->hostname, hstnm)){
          return current->fd;
      }
      current = current->next;
  }
}

/*
 * Read Functionality
 */
int read_operation(char *filename, char *offset){
  int temp_fd = mserver_fd;
  int m = 0;
  char *token;
  char temp_hostname[30];
  char temp_filename[20];
  char temp_chunk_id[2];
  char send_buffer[2048] = {0};
  char recv_buffer[2048] = {0};

  //construct the message to send to mServer
  strcpy(send_buffer, "read");
  strcat(send_buffer, " ");
  strcat(send_buffer, filename);
  strcat(send_buffer, " ");
  strcat(send_buffer, offset);
  if (send(temp_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending read request to mserver!!\n");
	}
  if(read(temp_fd, recv_buffer, 2048) < 0){
         fprintf(stderr, "mServersh(SERVER CODE): Error in reading reply from mserver!!\n");
  }

  token = strtok(recv_buffer, " ");
  while(token != NULL){
    if (m == 0){
      strcpy(temp_hostname, token);
    }
    else if (m == 1){
      strcpy(temp_filename, token);
    }
    else if (m == 2){
      strcpy(temp_chunk_id, token);
    }
    m++;
    token = strtok(NULL, " ");
  }

  memset(recv_buffer, 0, sizeof(recv_buffer));
  memset(send_buffer, 0, sizeof(send_buffer));
  temp_fd = get_file_server_fd(temp_hostname);
  //construct the read message to send to file server
  strcpy(send_buffer, "read");
  strcat(send_buffer, " ");
  strcat(send_buffer, filename);
  strcat(send_buffer, " ");
  strcat(send_buffer, temp_chunk_id);
  strcat(send_buffer, " ");
  strcat(send_buffer, offset);
  if (send(temp_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending read request to mserver!!\n");
	}
  if(read(temp_fd, recv_buffer, 2048) < 0){
         fprintf(stderr, "mServersh(SERVER CODE): Error in reading reply from mserver!!\n");
  }
  printf("The string read is:\n%s", recv_buffer);

}


/*
 * Append Functionality
 */
int append_operation(char *filename){
  int temp_fd = mserver_fd;
  int m = 0;
  char *token;
  char temp_hostname[30];
  char temp_filename[20];
  char temp_chunk_id[2];
  char send_buffer[2048] = {0};
  char recv_buffer[2048] = {0};

  //construct the message to send to mServer
  strcpy(send_buffer, "append");
  strcat(send_buffer, " ");
  strcat(send_buffer, filename);
  if (send(temp_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending append request to mserver!!\n");
	}
  if(read(temp_fd, recv_buffer, 2048) < 0){
         fprintf(stderr, "mServersh(SERVER CODE): Error in reading append reply from mserver!!\n");
  }

  token = strtok(recv_buffer, " ");
  while(token != NULL){
    if (m == 0){
      strcpy(temp_hostname, token);
    }
    else if (m == 1){
      strcpy(temp_filename, token);
    }
    else if (m == 2){
      strcpy(temp_chunk_id, token);
    }
    m++;
    token = strtok(NULL, " ");
  }

  memset(recv_buffer, 0, sizeof(recv_buffer));
  memset(send_buffer, 0, sizeof(send_buffer));
  temp_fd = get_file_server_fd(temp_hostname);
  //construct the append message to send to file server
  strcpy(send_buffer, "append");
  strcat(send_buffer, " ");
  strcat(send_buffer, filename);
  strcat(send_buffer, " ");
  strcat(send_buffer, temp_chunk_id);
  strcat(send_buffer, " ");
  strcat(send_buffer, "append_operation_from_client");
  if (send(temp_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending append request to mserver!!\n");
	}

}

/*
 * Create Functionality
 */
int append_operation(char *filename){
  int temp_fd = mserver_fd;
  char send_buffer[2048] = {0};
  char recv_buffer[2048] = {0};

  //construct the message to send to mServer
  strcpy(send_buffer, "create");
  strcat(send_buffer, " ");
  strcat(send_buffer, filename);
  if (send(temp_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending create request to mserver!!\n");
	}
}
