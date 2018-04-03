/*
 * FILE NAME: server.c
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
#include "server.h"

/*
 * server code
 */
int server()
{
        int master_sock_fd, new_conn_fd, fd, cli_conn_fds[6];
	int max_clients = 6;
	struct sockaddr_in server_addr, cli_addr;
	socklen_t cli_len;
	fd_set readfds;
	int max_fd;
	int i = 0;
	
	//Inotialize all client socket fds to 0.
	for (i = 0; i < max_clients; i++){
		cli_conn_fds[i] = 0;
	}
	
	//Open a socket connection
	if ((master_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "serversh: Error in socket creation!!\n");
		return 0;
	}

	//clear the server_addr structure
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(9000);
	
	//BIND the socket to localhost port 9000
	if (bind(master_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		fprintf(stderr, "serversh: Error in bind!!\n");
		return 0;
	}

	//Put the server in the listening passive mode with max pending connections as 10
	if (listen(master_sock_fd, 10)<0){
		fprintf(stderr, "serversh: Error in listen!!\n");
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
			fprintf(stderr, "serversh: Error in select call!!\n");
		}
		
		//check if there is a new incoming client connection
		if (FD_ISSET(master_sock_fd, &readfds)){
			if ((new_conn_fd = accept(master_sock_fd, (struct sockaddr *)&cli_addr, &cli_len))<0){
				fprintf(stderr, "serversh: Error in accept!!\n");
				return 0;
			}
			 //send list of files hosted to the client
			
			 //add the new connection fd to the client connection fd list
			 for (i = 0; i < max_clients; i++){
			 	//check for empty slot
				if (cli_conn_fds[i] == 0){
					cli_conn_fds[i] = new_conn_fd;
					break;
				}
		}
		//else its some IO operation for already connected clients
		else{
			for (i = 0; i < max_clients; i++){
				if (FD_ISSET(cli_conn_fds[i], &readfds)){
					rd = read(cli_conn_fds[i], recv_buffer, 1024);
					parse(recv_buffer);
					send(cli_conn_fds[i], send_buffer, strlen(sennd_buffer), 0);
					//read write data to file
				}
			}

		}
	}

}
