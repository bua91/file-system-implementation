/*
 * FILE NAME: metadata.c
 * OWNER: ARUNABHA CHAKRABORTY
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <dirent.h>
#include "p2psh.h"

/*
 * Generate the hash of the file contents pointed to by fptr)
 */
int hash_generate(FILE *fptr, char *hash)
{
    	unsigned char data[11] = {0};
      	unsigned char hashhex[SHA_DIGEST_LENGTH+1];
        unsigned char temp[50];
	int numread=0;
    	int i;

	SHA1(data,10, hash);
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	
	bzero(&temp,50);
	while(feof(fptr) == 0)
	{
	      numread = fread(data, sizeof(unsigned char), 10, fptr);
	      if(numread > 0)
	      {
	            //printf("%s",data);
	            SHA1_Update(&ctx,data, 10);
		    bzero(&data, 10);
	      }
	}
	SHA1_Final(hashhex, &ctx);
	for(i=0;i<SHA_DIGEST_LENGTH;i++) 
		sprintf(temp+strlen(temp),"%x",hashhex[i]);
        strcpy(hash,temp);
	return 1;
}

/*
 * Add metadata info to the list
 */
int metadata_add (int file_size, char *hash, char *file_name, char *filetype)
{
	struct meta_info *temp = (struct meta_info*) malloc(sizeof(struct meta_info));
	if (temp == NULL){
		return 0;
	}
	temp->file_len = file_size;
	strcpy(temp->filename, file_name);
	strcpy(temp->file_type, filetype);
	strcpy(temp->hash, hash);
	temp->next = meta_head;
	meta_head = temp;
	return 1;
}

/*
 * Generate the local metadata list
 */

int metadata()
{
	char hash[SHA_DIGEST_LENGTH+1];
	DIR *FD;
	char file[50];
	struct dirent* in_file;
	FILE *fptr;
	struct stat st;
	const char *dot;
	char file_type[5];

	bzero(hash,SHA_DIGEST_LENGTH);
	if (NULL == (FD = opendir("../demo_files"))){
		fprintf(stderr, "p2psh: directory opening failed!!\n");
		return 1;
	}
	while (in_file = readdir(FD)){
		if (!strcmp (in_file->d_name, "."))
			continue;
		if (!strcmp (in_file->d_name, ".."))    
		        continue;
		fptr = fopen(in_file->d_name, "r");
		if (fptr == NULL){
			fprintf(stderr, "p2psh: failed to open demo file!!\n");
			return 1;
		}

		/* Generate the file hash*/
		hash_generate(fptr, hash);

		/* Get the file name from in_file structure*/
		strcpy (file, in_file->d_name);

		/* To get the file size in bytes*/
		fstat(fptr, &st);

		/* Identifies the file type*/
		dot = strrchr(in_file->d_name, '.');
		if (!strncmp(*(dot+1), "p", 1)){
			strcpy(file_type, "png");
		}
		else if (!strncmp(*(dot+1), "h", 1)){
			strcpy(file_type, "html");
		}
		else {
			strcpy(file_type, "txt");
		}

		/* Add the metadata info to the meta data list*/
		if (metadata_add((int)st.st_size, hash, in_file->d_name, file_type) == 0){
			fprintf(stderr, "p2psh: Failure while adding metadata!!\n");
		}

		/* Close the file*/
		fclose(fptr);

		/* Clear the hash for the next hash storage*/
		bzero(hash, SHA_DIGEST_LENGTH);
	}
	return 1;
}
