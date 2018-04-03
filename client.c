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
#include "client.h"

//Global variables
fd_set readfds;
int max_clients = 10;
int cli_conn_fds[10];
int max_fd;
int no_of_conn = 0;
int peer_replies = 0;

/*
 * server part of the client
 */
int server()
{
        int master_sock_fd, new_conn_fd, fd;
	struct sockaddr_in server_addr, cli_addr;
	socklen_t cli_len;
	int i = 0;
	
	//Inotialize all client socket fds to 0.
	for (i = 0; i < max_clients; i++){
		cli_conn_fds[i] = 0;
	}
	
	//Open a socket connection
	if ((master_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "clientsh(SERVER CODE): Error in socket creation!!\n");
		return 0;
	}

	//clear the server_addr structure
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(9000);
	
	//BIND the socket to localhost port 9000
	if (bind(master_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		fprintf(stderr, "clientsh(SERVER CODE): Error in bind!!\n");
		return 0;
	}

	//Put the server in the listening passive mode with max pending connections as 10
	if (listen(master_sock_fd, 10)<0){
		fprintf(stderr, "clientsh(SERVER CODE): Error in listen!!\n");
		return 0;
	}
	

	while(1){
		//clear the socket set and add the master socket to the set
		FD_ZERO(&readfds);
		FD_SET(master_sock_fd, &readfds);
		max_fd = master_sock_fd;
		cli_len = sizeof(cli_addr);
		
		//Add client specific socket fds to the set
		for (i = 0; i < max_clients; i++){
			fd = cli_conn_fds[i];
			if ( fd > 0){
				//That means client socket exists
				FD_SET(fd, &readfds);
			}
			if ( fd > max_fd ){
				max_fd = fd;
			}
		}

		//wait for some new connection 
		activity = select (max_fd+1, &readfds, NULL, NULL, NULL);

		if ( (activity < 0) && (errno != EINTR)){
			fprintf(stderr, "clientsh(SERVER CODE): Error in select call!!\n");
		}
		
		//check if there is a new incoming client connection
		if (FD_ISSET(master_sock_fd, &readfds)){
			if ((new_conn_fd = accept(master_sock_fd, (struct sockaddr *)&cli_addr, &cli_len))<0){
				fprintf(stderr, "clientsh(SERVER CODE): Error in accept!!\n");
				return 0;
			}

			no_of_conn++;
			 
			 //add the new connection fd to the client connection fd list
			 for (i = 0; i < max_clients; i++){
			 	//check for empty slot
				if (cli_conn_fds[i] == 0){
					cli_conn_fds[i] = new_conn_fd;
					break;
				}
			}
		}
		//else its some IO operation for already connected clients
		else{
			for (i = 0; i < max_clients; i++){
				if (FD_ISSET(cli_conn_fds[i], &readfds)){
					//read write data to file
				}
			}

		}
	}

	return 1;
}

int is_client_server_socket( int sock_fd)
{
	int m = 0;
	int ret = 0;
	for (m=0; m<3; m++){
		if(server_fds[m] == sock_fd){
			ret = 1;
		}
	}
	return ret;
}

/*
 * Client functionality part of client node
 */
int peer_connect(char *ip_address)
{
        int sock_fd;
	struct sockaddr_in server_addr;
	char send_buffer[1024];
	char recv_buffer[1024];
	int i;
	int m;
	int random = 0;
	strcpy(send_buffer, "hello");

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
	
	//send hello to the new client/server
	if (send(sock_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending request to other client!!\n");
	}

	//read the greeting reply from the other clients and the servers
	rd = read(sock_fd, recv_buffer, 1024);
	if ((strncmp(recv_buffer, "server", 6) == 0)){
		for ( m = 0; m<3; m++){
			if(server_fds[m] == 0){
				server_fds[m] = sock_fd;
			}
		}
	}

	no_of_conn++;
	srand(time(0));

	while (1){
		//infinite loop to let the client socket open untill the program ends
		random = (rand() % 10) +1;
		sleep (random);
		if (no_of_conn >= 7) {
			if(cli_replies >= 4){
				if (is_client_server_socket(sock_fd)){
					request_to_server_send(sock_fd);
				}
			}
			else {
				if (!is_client_server_socket(sock_fd)){
					cli_replies++;
					client_send_request(sock_fd);
				}
			}
		}

	}
	
	close(sock_fd);
	return 1;
}

/*
 * send request to server for writing to a file hosted by the server 
 */
int request_to_server_send(int sock_fd)
{
	struct timestamp * temp = head;
	int random;
	char send_buffer[1024];
	char recv_buffer[1024];
	int fd = temp->socket_fd;
	long int min = temp->t_stamp;
	char hst_nm[30];
	char cli_id_temp[30];
	
	gethostname(hst_nm, 30);
	
	//get the min timestamp from the queue
	while (temp != NULL){
		if (min > temp->t_stamp){
			if (!strncmp(hst_nm, temp->cli_id, 10)){
				min = temp->t_stamp;
				fd = temp->socket_fd;
				strcpy(cli_id_temp, temp->cli_id);
			}
		}
		temp = temp->next;
	}

	//chech if the client request is at the head of the queue
	if (!strncmp(cli_id_tmp, hst_nm)){
		srand(time(0));
		random = (rand() % 2);

		
	}

}
/*
 * Lamports mutual exclusion implementation. REQUEST send to other clients
 */
void client_send_request(int sock_fd)
{
	struct timestamp *temp = (struct timestamp*) malloc(sizeof(struct timestamp));
	char send_buffer[1024];
	char recv_buffer[1024];
	char ck[25];
	int rd;

	temp->next = head;
	temp->socket_fd = sock_fd;
	//fetching the local clock time
	//time_t t = time(NULL);
	//long int t_temp = (long int) t;
	temp->t_stamp = local_clock;

	//fetching the hostname
	char hostname_temp[30];
	gethostname(hostname_temp, 30);
	strcpy(temp->cli_id, hostname_temp);

	//copy filename to the structure.
	head = temp;

	//construct the message to be sent
	strcpy(send_buffer, hostname_temp);
	strcat(send_buffer, " ");
	sprintf(ck, "%ld", local_clock);
	strcat(send_buffer, ck);
	
	//Send request to other clients.
	if (send(sock_fd, send_buffer, strlen(send_buffer), 0) < 0){
		fprintf(stderr,"clientsh: error in sending request to other client!!\n");
	}

	rd = read(sock_fd, recv_buffer, 1024);
	if ((strncmp(recv_buffer, "reply", 5) != 0)){
		fprintf(stderr, "something else is being replied by the other client!!\n");
	}
}

