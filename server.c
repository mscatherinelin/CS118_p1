/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
 */
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */


void error(char *msg)
{
	perror(msg);
	exit(1);
}

void serveFile(int sockfd, char *file){
	if(file=="\0")
		error("Error: no file specified!\n");

	int n;
	char *buf = NULL;
	int length = 0;
	FILE *fp = fopen(file, "r");

	if (fp == NULL)
		error("Error: file could not be opened.\n");

	//place file pointer at the end of the file
	if (fseek(fp, 0, SEEK_END) == 0){
		//obtain the size of the file
		length = ftell(fp);
		printf(" the length of the file is %d\n", length);
		buf = malloc(sizeof(char) * (length + 1));

		//read the data from the file into buf
		size_t read_length = fread(buf, sizeof(char), length, fp);
		printf(" the number of bytes read is %d\n", read_length);
		if (read_length == 0)
			error("Error: could not read file.\n");

		//set the null byte at the end of the buffer
		buf[read_length] = '\0';

		//generate the html response headers
		char *response = 
		"HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"\n";

		n = write(sockfd,response,strlen(response));

		if (n < 0) error("ERROR writing to socket");

		n = write(sockfd, buf, read_length);

		if (n < 0) error("ERROR writing to socket");

		free(buf);

	}
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//create socket
	if (sockfd < 0) 
		error("ERROR opening socket");
	memset((char *) &serv_addr, 0, sizeof(serv_addr));	//reset memory
	//fill in address info
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		error("ERROR on binding");

	listen(sockfd,5);	//5 simultaneous connection at most

	//accept connections
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0) 
		error("ERROR on accept");

	int n;
	char buffer[256];

	memset(buffer, 0, 256);	//reset memory

	//read client's message
	n = read(newsockfd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	
	printf("Here is the message: %s\n",buffer);

	// get filename 

	char filename[256];

	sscanf(buffer, "GET /%s HTTP/1.1 /n \*",filename);
	
	printf("%s \n",filename);

	serveFile(newsockfd, filename);
	
	//reply to client
	/*
	char *response = 
		"HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"\n";

	n = write(newsockfd,response,strlen(response));
	
	response = "<h1> hello <\h1>";

	n = write(newsockfd ,response,strlen(response));

	if (n < 0) error("ERROR writing to socket");
	*/

	close(newsockfd);//close connection 
	close(sockfd);

	return 0; 
}


