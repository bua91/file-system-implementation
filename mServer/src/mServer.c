/*
 * FILE NAME: mServer.c
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
#include "mServer.h"

char temp_hostname[30];

/*
 * Insert metadata info(file_server name, file-name and chunk number) to metadata table
 */
int insert_into_metadata(char *hst_name, char *filenm, char *chunkid)
{
  struct metadata *current = head;
	struct metadata *temp = (struct metadata*) malloc(sizeof(struct metadata));
	int cunk_id = atoi(chunkid);
	if (cunk_id == 0){
		return 0;
	}
  //if metadata is already there do nothing
  while (current != NULL)
  {
      if ((!strcmp(current->host_name, hst_name)) && (!strcmp(current->file_name, filenm)) && (current->chunk_id == cunk_id)) {
          return 1;
      }
      current = current->next;
  }

  //if metadata not in metadata table, then insert
	strcpy(temp->host_name, hst_name);
  strcpy(temp->file_name, filenm);
	temp->chunk_id = cunk_id;
	temp->next = head;
	head = temp;

	return 1;
}

/*
 * delete data from request queue
 *
int delete_from_request_queue(char * t_hstnm, char * t_tmstmp)
{
	struct request *temp = head;
	struct request *prev = head;
	if (temp == NULL){
		fprintf(stderr, "mServersh: no nodes in the request list!!\n");
	}
	//if only one node or first node to be deleted
	else if ((temp->next == NULL) || (!strcmp(temp->cli_id, t_hstnm))){
		if (!strcmp(temp->cli_id, t_hstnm)){
			head = head->next;
			temp->next = NULL;
		}
	}
	else {
		temp = temp->next;
		while (temp != NULL){
			if (!strcmp(temp->cli_id, t_hstnm)){
				prev->next = temp->next;
				temp->next = NULL;
			}
			prev = temp;
			temp = temp->next;
		}
	}
	//update the local clock
	local_clock ++;

	return 1;
}*/

/*
 * Get the chunk id
 */
int get_chunk_id_from_offset(char *filename, char *offset)
{
  int offst;
  int chunk_no;
  int max_chunk = 0;
  struct metadata *current = head;
  if (!strcmp(offset, "max")){
    //apend part: return max chunk number for append
    while (current != NULL)
    {
        if (!strcmp(current->file_name, filename)) {
            if(max_chunk < current->chunk_id){
              max_chunk = current->chunk_id;
            }
        }
        current = current->next;
    }
    return max_chunk;
  }
  else{
    sscanf(offset, "%d", &offst);
    if (offst <= 8192){
      chunk_no = 1;
    }
    else{
      chunk_no = (offst/8192) +1;
    }
    while (current != NULL)
    {
        if ((!strcmp(current->file_name, filename)) && (current->chunk_id == chunk_no)){
            return chunk_no;
        }
        current = current->next;
    }
  }
  return 0;
}

/*
 *get the hostname of the file_server hosting the file and chunk
 */
void get_host_name(char *filename, int chunkid)
{
  struct metadata *current = head;
  while (current != NULL)
  {
      if ((!strcmp(current->file_name, filename)) && (current->chunk_id == chunkid)){
          strcpy(temp_hostname, current->host_name);
      }
      current = current->next;
  }
}

/*
 * server part of the client
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
		fprintf(stderr, "mServersh(SERVER CODE): Error in socket creation!!\n");
		return 0;
	}

	//clear the server_addr structure
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(9000);

	//BIND the socket to localhost port 9000
	if (bind(master_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		fprintf(stderr, "mServersh(SERVER CODE): Error in bind!!\n");
		return 0;
	}

	//Put the server in the listening passive mode with max pending connections as 10
	if (listen(master_sock_fd, 10)<0){
		fprintf(stderr, "mServersh(SERVER CODE): Error in listen!!\n");
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
			if ( fd > 0){
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
			fprintf(stderr, "mServersh(SERVER CODE): Error in select call!!\n");
		}

		//check if there is a new incoming client connection
		if (FD_ISSET(master_sock_fd, &readfds)){
			if ((new_conn_fd = accept(master_sock_fd, (struct sockaddr *)&cli_addr, &cli_len))<0){
				fprintf(stderr, "mServersh(SERVER CODE): Error in accept!!\n");
			}

			if (read(new_conn_fd, recv_buffer, 2048) < 0){
				fprintf(stderr, "mServersh(SERVER CODE): Error in reading hello from client!!\n");
			}

			if (!strcmp(recv_buffer, "server")){
				//store the connection fd in server_fds
        server_fds[server_fd_index] = new_conn_fd;
        server_fd_index++;
			}
      else if (!strcmp(recv_buffer, "client")){
        //store the connection fd in client_fds
        client_fds[client_fd_index] = new_conn_fd;
        client_fd_index++;
      }

      //send a reply saying mserver
      strcpy(send_buffer, "mserver");
      if (send(new_conn_fd, send_buffer, strlen(send_buffer), 0) < 0){
        fprintf(stderr,"mServersh: error in sending reply message for the new connection!!\n");
      }

      //hostname sending part
      if (read(new_conn_fd, rec_buf, 30) < 0){
				fprintf(stderr, "mServersh(SERVER CODE): Error in reading hostname request from client!!\n");
			}

      if(!strcmp(rec_buf, "hostname")){
        gethostname(sen_buf, 30);
        if (send(new_conn_fd, sen_buf, strlen(sen_buf), 0) < 0){
          fprintf(stderr,"mServersh: error in sending hostname message for the new connection!!\n");
        }
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
					       fprintf(stderr, "mServersh(SERVER CODE): Error in reading request message from client!!\n");
			    }
          //check if the connection is coming from file_server
          for (j = 0; j < 5; j++){
            if(server_fds[j] == conn_fds[i]){
              file_server_fd = 1;
              break;
            }
          }

          //If the connection is coming from file_server, then parse recv buffer and store the metadata info
					if (file_server_fd == 1){
						char *token;
						int m =0;
            char temp_hostname[30];
						char temp_filename[20];
						char temp_chunk_id[2];
						token = strtok (recv_buffer, " ");
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
              if (m > 2){
                if (!insert_into_metadata(temp_hostname, temp_filename, temp_chunk_id)){
                  fprintf(stderr, "mServersh(SERVER CODE): error in inserting metadata info!!\n");
                }
                m = 0;
              }
							token = strtok (NULL, " ");
						}
						file_server_fd = 0;
					}

          //Else connection is coming from requesting client for (create/read/append)
					else{
						//parse the recv buffer
						char *token;
						int m =0;
            int cunk_id;
            char temp_chunk_id[2];
						char temp_filename[20];
						char temp_offset[5];
						token = strtok(recv_buffer, " ");

            //read part handler
            if (!strcmp(token, "read")){
              while(token != NULL){
  							if (m == 1){
  								strcpy(temp_filename, token);
  							}
  							else if (m == 2){
  								strcpy(temp_offset, token);
  							}
  							m++;
  							token = strtok(NULL, " ");
  						}
              //get chunk id from the offset.
              cunk_id = get_chunk_id_from_offset(temp_filename,temp_offset);
              if (cunk_id != 0) {
                get_host_name(temp_filename,cunk_id);

                sprintf(temp_chunk_id, "%d", cunk_id);
                strcpy(send_buffer, temp_hostname);
                strcat(send_buffer, " ");
                strcat(send_buffer, temp_filename);
                strcat(send_buffer, " ");
                strcat(send_buffer, temp_chunk_id);
                if (send(conn_fds[i], send_buffer, strlen(send_buffer), 0) < 0){
                  fprintf(stderr,"mServersh: error in sending read metadata info to client!!\n");
                }
                //clear temp_hostname
                memset(temp_hostname, 0, sizeof(temp_hostname));
              }
              else {
                fprintf(stderr,"mServersh: no file chunk found !!\n");
              }
            }

            //append part handler
            else if (!strcmp(token, "append")){
              while(token != NULL){
  							if (m == 1){
  								strcpy(temp_filename, token);
  							}
  							m++;
  							token = strtok(NULL, " ");
  						}
              strcpy(temp_offset, "max");
              cunk_id = get_chunk_id_from_offset(temp_filename,temp_offset);
              if (cunk_id != 0) {
                get_host_name(temp_filename,cunk_id);
                sprintf(temp_chunk_id, "%d", cunk_id);
                strcpy(send_buffer, temp_hostname);
                strcat(send_buffer, " ");
                strcat(send_buffer, temp_filename);
                strcat(send_buffer, " ");
                strcat(send_buffer, temp_chunk_id);
                if (send(conn_fds[i], send_buffer, strlen(send_buffer), 0) < 0){
                  fprintf(stderr,"mServersh: error in sending append metadata info to client!!\n");
                }
                //clear temp_hostname
                memset(temp_hostname, 0, sizeof(temp_hostname));
              }
              else {
                fprintf(stderr,"mServersh: no file found to append!!\n");
              }
            }

            //create new file handler
            else if (!strcmp(token, "create")){
              while(token != NULL){
  							if (m == 1){
  								strcpy(temp_filename, token);
  							}
  							m++;
  							token = strtok(NULL, " ");
  						}
              //send request to random server for file creation
              //put that info in metadata table
              //send "success" to requesting client
            }

						/*delete data from request queue
						if (!delete_from_request_queue(temp_hostname, temp_timestamp)){
							fprintf(stderr, "mServersh(SERVER CODE): error in inserting remote request to queue!!\n");
						}*/
					}
          memset(recv_buffer, 0, sizeof(recv_buffer));
          memset(send_buffer, 0, sizeof(send_buffer));
				}
   		}
		}
	}
	return 1;
}
