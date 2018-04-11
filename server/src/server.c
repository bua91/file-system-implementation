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
#include <errno.h>
#include "server.h"

/*
 * Insert metadata info(file-name and chunk number) to metadata table
 */
int insert_into_metadata(char *filenm, char *chunkid)
{
	struct metadata *temp = (struct metadata*) malloc(sizeof(struct metadata));
	int cunk_id = atoi(chunkid);
	if (cunk_id == 0){
		return 0;
	}
  //if metadata not in metadata table, then insert
  strcpy(temp->file_name, filenm);
	temp->chunk_id = cunk_id;
	temp->next = head;
	head = temp;

	return 1;
}

/*
 * server part of the server
 */
int server()
{
  int master_sock_fd, new_conn_fd, fd;
	struct sockaddr_in server_addr, cli_addr;
	socklen_t cli_len;
	int i = 0;
  int j;
	int max_fd, activity;
	char send_buffer[2048] = {0};
	char recv_buffer[2048] = {0};
  char rec_buf[30] = {0};
  char sen_buf[30] = {0};

  int file_server_fd = 0;

	//Open a socket connection
  if ((master_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "serversh(SERVER CODE): Error in socket creation!!\n");
		return 0;
	}

	//clear the server_addr structure
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(9000);

	//BIND the socket to localhost port 9000
	if (bind(master_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		fprintf(stderr, "serversh(SERVER CODE): Error in bind!!\n");
		return 0;
	}

	//Put the server in the listening passive mode with max pending connections as 10
	if (listen(master_sock_fd, 10)<0){
		fprintf(stderr, "serversh(SERVER CODE): Error in listen!!\n");
		return 0;
	}


	while(1){
		//clear the socket set and add the master socket to the set
		FD_ZERO(&readfds);
		FD_SET(master_sock_fd, &readfds);
		max_fd = master_sock_fd;
		cli_len = sizeof(cli_addr);

		//Add socket fds to the set
		for (i = 0; i < max_clients; i++){
			fd = conn_fds[i];
			if (fd > 0){
				//That means client/server socket exists
				FD_SET(fd, &readfds);
			}
			if ( fd > max_fd ){
				max_fd = fd;
			}
		}

		//wait for some new connection
		activity = select (max_fd+1, &readfds, NULL, NULL, NULL);

		if ( (activity < 0) && (errno != EINTR)){
			fprintf(stderr, "serversh(SERVER CODE): Error in select call!!\n");
		}

		//check if there is a new incoming client connection
		if (FD_ISSET(master_sock_fd, &readfds)){
			if ((new_conn_fd = accept(master_sock_fd, (struct sockaddr *)&cli_addr, &cli_len))<0){
				fprintf(stderr, "serversh(SERVER CODE): Error in accept!!\n");
			}

			if (read(new_conn_fd, recv_buffer, 2048) < 0){
				fprintf(stderr, "serversh(SERVER CODE): Error in reading hello from client!!\n");
			}

      if (!strcmp(recv_buffer, "client")){
        //store the connection fd in client_fds
        client_fds[client_fd_index] = new_conn_fd;
        client_fd_index++;
      }

      //send a reply saying server
      strcpy(send_buffer, "server");
      if (send(new_conn_fd, send_buffer, strlen(send_buffer), 0) < 0){
        fprintf(stderr,"serversh: error in sending reply message for the new connection!!\n");
      }

			//add the new connection fd to the connection fd list
			for (i = 0; i < max_clients; i++){
			 	//check for empty slot
				if (conn_fds[i] == 0){
					conn_fds[i] = new_conn_fd;
					break;
				}
			}
			// At last clear the recv and send buffer so that it can be used later
			memset(recv_buffer, 0, sizeof(recv_buffer));
			memset(send_buffer, 0, sizeof(send_buffer));
		}
		//else its some IO operation for already connected clients
		else{
			for (i = 0; i < max_clients; i++){
				if (FD_ISSET(conn_fds[i], &readfds)){
					if(read(conn_fds[i], recv_buffer, 2048) < 0){
					       fprintf(stderr, "serversh(SERVER CODE): Error in reading request message from client!!\n");
			    }

          //connection is coming from requesting client for (read/append)

					//parse the recv buffer
					char *token;
					int m =0;
          int cunk_id;
          char temp_chunk_id[2];
					char temp_filename[20];
					char temp_offset[5];
          char filnm[30];
          char buff[2048];
					token = strtok(recv_buffer, " ");

          //read part handler
          if (!strcmp(token, "read")){
            while(token != NULL){
							if (m == 1){
								strcpy(temp_filename, token);
							}
							else if (m == 2){
								strcpy(temp_chunk_id, token);
							}
              else if (m == 3){
                strcpy(temp_offset, token);
              }
							m++;
							token = strtok(NULL, " ");
						}
            //Open the file and read from it
            FILE *fp;
            strcpy(filnm, temp_filename);
            strcat(filnm, temp_chunk_id);
            fp = fopen(filnm, "r");
            fgets(buff, 2048, (FILE*)fp);
            //send the read string to the client
            if (send(conn_fds[i], buff, strlen(buff), 0) < 0){
              fprintf(stderr,"mServersh: error in sending read string to client!!\n");
            }
            fclose(fp);
          }

          //append part handler
          else if (!strcmp(token, "append")){
            while(token != NULL){
              if (m == 1){
                strcpy(temp_filename, token);
              }
              else if (m == 2){
                strcpy(temp_chunk_id, token);
              }
              else if (m == 3){
                strcpy(buff, token);
              }
              m++;
              token = strtok(NULL, " ");
            }
            //Open the file and append to it
            FILE *fp;
            strcpy(filnm, temp_filename);
            strcat(filnm, temp_chunk_id);
            fp = fopen(filnm, "a");
            fprintf(fp, "%s\n", buff);
            fclose(fp);
          }

					/*delete data from request queue
					if (!delete_from_request_queue(temp_hostname, temp_timestamp)){
						fprintf(stderr, "mServersh(SERVER CODE): error in inserting remote request to queue!!\n");
					}*/
          memset(recv_buffer, 0, sizeof(recv_buffer));
          memset(send_buffer, 0, sizeof(send_buffer));
				}
   		}
		}
	}
	return 1;
}

/*
 *  * Client functionality part of client node
 *   */
int peer_connect(char *ip_address)
{
	//to be written
}
