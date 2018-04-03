/*
 * File name: server.h
 * Functionality: contains data structures and function prototypes 
 * used by the server.
 */

//Data structures
#include <stdio.h>
struct files_hosted{
	char * filename;
	char * last_line;
	struct files_hosted * next;
}
// Function prototypes
int server();

