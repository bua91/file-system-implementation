/*
 * FILE NAME: clientsh.c
 * OWNER: ARUNABHA CHAKRABORTY
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "client.h"

#define TOK_BUFF_SIZE 128
#define TOK_DELIMITER " \t\r\n\a"

/*
 * global variables
 */
pthread_t ptid[10];
int num = 1;

/* 
 *Function decleration for built in functions
 */
int clientsh_cd(char **args);
int clientsh_help(char **args);
int clientsh_exit(char **args);

/*
 * Built in function implementations.
 * ************************************************************
 * ************************************************************
 * NAME: clientsh_help
 * PURPOSE: Display help string for built in cmds.
 */
int clientsh_help(char **args)
{
	printf("CLIENTSH SUPPORTED BUILT IN COMANDS\n");
	printf("help                                   Displayes the helpline\n");
	printf("cd                                     Changes directory\n");
	printf("show hosted-files                      To show the list of files hosted by the servers\n");
	printf("peer <IP_ADDRESS> <PORT>               To peer with clients as well as server. Please use port 9000 for peering\n");
	printf("start                                  To start the sending of information to the server files considering lamport mutual exclusion\n");
	printf("exit                                   Exit clientsh\n");
	return 1;
}

/*
 * NAME: clientsh_cd
 * PURPOSE: change directory
 */
int clientsh_cd(char **args)
{
	if (args[1] == NULL){
		fprintf(stderr, "clientsh: Expected argument to cd!!\n");
	}
	else{
		if(chdir(args[1]) !=0){
			perror("clientsh");
		}
	}
	return 1;
}

/*
 * NAME: clientsh_exit
 * PURPOSE: To exit from clientsh shell.
 * WARNING: All configurations will be gone.
 */
int clientsh_exit(char **args)
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
		fprintf(stderr, "clientsh: allocation error!!\n");
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
			 	fprintf(stderr, "clientsh: allocation error!!\n");
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
		 perror("clientsh");
  	}
  	else if (pid == 0){
  		// Child process
    		if (execvp(args[0], args) == -1){
			perror("clientsh");
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
 * Thread handler for send
 */
void * start_send(void * arg)
{
	client_send();
}

/*
 * Start the sending process
 */
int start()
{
	pthread_t stid;
	int err;
	err = pthread_create(&stid,NULL,start_send,NULL);
	if (err != 0){
		fprintf(stderr,"clientsh: client send thread creation error!!\n");
	}
	return 1
}

/*
 * NAME: cmd_execute
 * RETURN_TYPE: integer
 * PURPOSE: To execute the clientsh built in cmds and other native linux shell cmds.
 */
int cmd_execute (char **args)
{
	int i;
	if (args[0] == NULL)
		return 1;
	if (strcmp (args[0],"help") == 0){
		return (clientsh_help(args));
	}
	else if (strcmp (args[0], "cd") == 0){
		return (clientsh_cd(args));
	}
	else if (strcmp (args[0], "exit") == 0){
		return (clientsh_exit(args));
	}
	else if (strcmp (args[0], "peer")){
		return (connect(args[1]));
	}
	else if (strcmp (args[0], "start")){
		return (start());
	}
	else if (strcmp (args[0], "show") == 0){
		return (show_hosted_files());
	}
	return (cmd_launch(args));
}

/*
 * Show hosted files by the server
 */
int show_hosted_files()
{
	printf("\ttest1.txt\n\ttest2.txt");
	return 1;
}

/*
 * Thread handler for peer connection
 */
void * peer_conn(void * arg)
{
	peer_connect((char *)arg);
}

/*
 * Function to start a new thread for handling the peer client connection
 */
int connect( char *ip_addr)
{
	pthread_t local_ptid = ptid[num];
	int err;
	err = pthread_create(&local_ptid,NULL,peer_conn,(void *)ip_addr );
	if (err != 0){
		fprintf(stderr,"clientsh: client thread creation error!!\n");
	}
	num++;
	return 1;
}

/*
 * Initialize the global variables.
 */
int initialize()
{
	head = NULL;
	local_clock = 1;
	int n =0;
	for (i =0; i<3; i++){
		server_fds[i] = 0;
	}
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
	printf("                  CLIENTSH                             \n\n");
	printf("       (CREATOR: ARUNABHA CHAKRABORTY                  \n\n");
	printf("=======================================================\n\n");
	/* Run local server in a new thread upon starting clientsh*/
	err = pthread_create(&tid, NULL, local_server, NULL);
	if (err != 0 ){
		fprintf(stderri, "clientsh: server thread creation error");
	}
	/* Start the shell and execute comands*/
        do{
                printf("CLIENTSH>>");
                cmd_str = read_line();
                cmd_args = parse_line(cmd_str);
                status = cmd_execute(cmd_args);
        }while(status);
        return EXIT_SUCCESS;
}

