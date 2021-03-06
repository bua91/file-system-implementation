/*
 * FILE NAME: serversh.c
 * OWNER: ARUNABHA CHAKRABORTY
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"

#define TOK_BUFF_SIZE 128
#define TOK_DELIMITER " \t\r\n\a"

/*
 *Function decleration for built in functions
 */
int serversh_cd(char **args);
int serversh_help(char **args);
int serversh_exit(char **args);

/*
 * Built in function implementations.
 * ************************************************************
 * ************************************************************
 * NAME: serversh_help
 * PURPOSE: Display help string for built in cmds.
 */
int serversh_help(char **args)
{
	printf("SERVERSH SUPPORTED BUILT IN COMANDS\n");
	printf("help                                   Displayes the helpline\n");
	printf("cd                                     Changes directory\n");
  printf("peer <IP_ADDRESS> <PORT>               To peer with m-server. Please use port 9000 for peering\n");
	printf("show hosted-files                      Shows the list of files and chunks hosted by the server\n");
	//printf("total connections                      show total number of connections\n");
	printf("exit                                   Exit serversh\n");
	return 1;
}

/*
 * NAME: serversh_cd
 * PURPOSE: change directory
 */
int serversh_cd(char **args)
{
	if (args[1] == NULL){
		fprintf(stderr, "serversh: Expected argument to cd!!\n");
	}
	else{
		if(chdir(args[1]) !=0){
			perror("serversh");
		}
	}
	return 1;
}

/*
 * NAME: serversh_exit
 * PURPOSE: To exit from serversh shell.
 * WARNING: All configurations will be gone.
 */
int serversh_exit(char **args)
{
	return 0;
}
/*
 * NAME: read_line
 * RETURN_TYPE: char *
 * PURPOSE: To read a cmd typed in the shell
 */
char * read_line()
{
        char *line;
	ssize_t buff_size = 0;
	getline(&line, &buff_size, stdin);
	return line;
}

/*
 * NAME: parse_line
 * RETURN_TYPE: array of \0 terminated strings
 * PURPOSE: To tokenize the input cmd string and return array of \0 terminated string
 *          arguments of the cmd.
 */
char ** parse_line(char *line)
{
	int buff_size = TOK_BUFF_SIZE, i = 0;
	char *token;
	char **cmd_arguments = malloc(buff_size * sizeof(char*));
	if(!cmd_arguments){
		fprintf(stderr, "serversh: allocation error!!\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, TOK_DELIMITER);
	while (token != NULL){
		cmd_arguments[i] = token;
		i++;

		if (i >= buff_size){
			buff_size += TOK_BUFF_SIZE;
			cmd_arguments = realloc(cmd_arguments, buff_size*sizeof(char*));
			if (!cmd_arguments){
			 	fprintf(stderr, "serversh: allocation error!!\n");
			 	exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, TOK_DELIMITER);
	}
	cmd_arguments[i] = NULL;
	return cmd_arguments;
}

/*
 * NAME: cmd_launch
 * RETURN_TYPE: integer
 * PURPOSE: To create a child process and execute the native linux shell cmds
 *          with exception of exit and cd.
 */
int cmd_launch(char **args)
{
	pid_t pid, wpid;
  	int status;

  	pid = fork();
  	if (pid < 0){
		 // Error forking
		 perror("serversh");
  	}
  	else if (pid == 0){
  		// Child process
    		if (execvp(args[0], args) == -1){
			perror("serversh");
		}
		exit(EXIT_FAILURE);
  	}
  	else{
  		// Parent process
		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		}while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
  	return 1;
}

/*
 * NAME: cmd_execute
 * RETURN_TYPE: integer
 * PURPOSE: To execute the serversh built in cmds and other native linux shell cmds.
 */
int cmd_execute (char **args)
{
	int i;
	if (args[0] == NULL)
		return 1;
	if (strcmp (args[0],"help") == 0){
		return (serversh_help(args));
	}
	else if (strcmp (args[0], "cd") == 0){
		return (serversh_cd(args));
	}
	else if (strcmp (args[0], "exit") == 0){
		return (serversh_exit(args));
	}
  else if (strcmp (args[0], "peer") == 0){
 		return (client_connect(args[1]));
 	}
	else if (strcmp (args[0], "show") == 0){
		return (show_hosted_files());
	}
	/*else if (strcmp (args[0], "total") == 0){
		printf("total connections = %d\n", no_of_conn);
		return 1;
	}*/
	return (cmd_launch(args));
}

/*
 * Thread handler for peer connection
 */
void * peer_conn(void * arg)
{
 peer_connect((char *)arg);
}

/*
 * Function to start a new thread for handling the peer mserver connection
 */
int client_connect( char *ip_addr)
{
 pthread_t local_ptid;
 int err;
 err = pthread_create(&local_ptid,NULL,peer_conn,(void *)ip_addr );
 if (err != 0){
   fprintf(stderr,"serversh: mserver client thread creation error!!\n");
 }
 //ptid[num] = local_ptid;
 //num++;
 return 1;
}

/*
 * Show hosted files by the server
 */
int show_hosted_files()
{
	//Fetching metadata information
	struct metadata *current = head;
	printf("METADATA TABLE :\n=======================\n\n");
	printf("%20s%10s\n", "File name", "Chunk id");
	while (current != NULL)
  {
      printf("%20s%10d\n", current->file_name, current->chunk_id);
      current = current->next;
  }
	return 1;
}

/*
 * Initialize the global variables.
 */
int initialize()
{
  head = NULL;
	int i;
	max_clients = 3;
  //Initialize all client socket fds to 0.
  for (i = 0; i < max_clients; i++){
    conn_fds[i] = 0;
  }
	client_fd_index = 0;
	//Initialize all the client_fds to 0.
	for (i =0; i<3; i++){
		client_fds[i] = 0;
	}
	mserver_fd =0;
	return 1;
}

/*
 * function for starting local server
 */
void *local_server(void * arg)
{
	int ret;
	ret = server();
	return ret;
}

/*
 * MAIN FUNCTION
 */
int main(int argc, char **argv)
{
  char *cmd_str;
  char **cmd_args;
	int status;
	pthread_t tid;
	int err;
	initialize();
	printf("=======================================================\n\n");
	printf("                  serversh                             \n\n");
	printf("       (CREATOR: ARUNABHA CHAKRABORTY                  \n\n");
	printf("=======================================================\n\n");
	/* Run local server in a new thread upon starting serversh*/
	err = pthread_create(&tid, NULL, local_server, NULL);
	if (err != 0 ){
		fprintf(stderr, "serversh: server thread creation error!!\n");
	}
	/* Start the shell and execute comands*/
  do{
          printf("serversh>>");
          cmd_str = read_line();
          cmd_args = parse_line(cmd_str);
          status = cmd_execute(cmd_args);
  }while(status);
  return EXIT_SUCCESS;
}
