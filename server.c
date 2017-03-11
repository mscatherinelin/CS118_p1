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
#include <string.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <unistd.h>

void error(char *msg)
{
	perror(msg);
	exit(1);
}

char* file_type(char* filename){
	char* response;
	if(strstr(filename , ".jpg") != NULL) {
		
	response =  "HTTP/1.1 200 OK\n"
		    "Content-Type: image/jpeg\n"
		    "\n";
	
	}
	else if(strstr(filename , ".gif") != NULL){
	
	response =  "HTTP/1.1 200 OK\n"
		    "Content-Type: image/gif\n"
		    "\n";
	}
	else{
	
	response = "HTTP/1.1 200 OK\n"
		   "Content-Type: text/html\n"
		   "\n";
	}

	return response;
}



void serveFile(int sockfd, char *file){

	//if(strncmp(file,"\0",1)){
	//	error("Error: no file specified!\n");
	//}

	int n;
	char *buf = NULL;
	int length = 0;
	char * response;

	FILE *fp = fopen(file, "rb");

	if (fp == NULL){
		 response = "HTTP/1.1 404 Not Found\n"
                   	    "Content-Type: text/html\n"
                            "\n"
			    "<html><body><h1> 404 Not Found </h1></body><html>";			
		 n = write(sockfd,response,strlen(response));
		 // error("Error: file could not be opened.\n");
	}
	
	//place file pointer at the end of the file

	else if(fseek(fp, 0, SEEK_END) == 0){
		//obtain the size of the file
		length = ftell(fp);
		buf = malloc(sizeof(char) * (length + 1));

		//read the data from the file into buf
		
		fseek(fp, SEEK_SET, 0);

		size_t read_length = fread(buf, sizeof(char), length, fp);
		
		if (read_length == 0)
			error("Error: could not read file.\n");

		//set the null byte at the end of the buffer
		buf[read_length] = '\0';
		
		response = file_type(file);
		//generate the html response headers
	
		n = write(sockfd,response,strlen(response));

		if (n < 0) error("ERROR writing to socket");

		n = write(sockfd, buf, read_length);

		if (n < 0) error("ERROR writing to socket");

		free(buf);
	}
	
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
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
	while(1){
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
	if (newsockfd < 0) 
		error("ERROR on accept");

	int n;
	char buffer[256];

	memset(buffer, 0, 256);	//reset memory

	//read client's message
	n = read(newsockfd,buffer,255);
	if (n < 0) error("ERROR reading from socket");
	
	printf("%s\n",buffer);

	// get filename 

	char filename[256];

	sscanf(buffer, "GET /%s HTTP/1.1 /n /*",filename);
	
	// reply the file

	serveFile(newsockfd, filename);

	
	close(newsockfd);	//close connection 
	}
	close(sockfd);

	return 0; 
}
