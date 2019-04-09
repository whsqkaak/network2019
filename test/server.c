/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#define SMALL_BUF 100
#define BUF_SIZE 1024

char *request_parser(char*);
char *content_type(char *);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd; //descriptors rturn from socket and accept system calls
    int portno; // port number
    socklen_t clilen;
     
    char buffer[BUF_SIZE];
    int opt = 1;     
    /*sockaddr_in: Structure Containing an Internet Address*/
    struct sockaddr_in serv_addr, cli_addr;
     
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
     
    /*Create a new socket
      AF_INET: Address Domain is Internet 
      SOCK_STREAM: Socket Type is STREAM Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        		&opt, sizeof(opt)))
    {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }
					    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); //atoi converts from String to Integer
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
    serv_addr.sin_port = htons(portno); //convert from host to network byte order
     
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
             error("ERROR on binding");
     
    listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5
     
    clilen = sizeof(cli_addr);
    while(1){
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0) 
           error("ERROR on accept");
      memset(buffer, 0, BUF_SIZE); 
      n = read(newsockfd,buffer,255); //Read is a block function. It will read at most 255 bytes
      if (n < 0) error("ERROR reading from socket");
       
      //Request parsing
      char method[10];
      char file_name[30];
      char ct[15];
       
      if(strstr(buffer, "HTTP/") == NULL){
        error("No Http.");
      }

      strcpy(method, strtok(buffer, " /"));
      strcpy(file_name, strtok(NULL, " /"));
      strcpy(ct, content_type(file_name));
      if(strcmp(method, "GET")!=0)
        error("No GET method.");
       
      printf("method : %s\n", method);
      printf("file name : %s\n", file_name);
      printf("content-type : %s\n\n", ct);       
       
      //Make Response header line
       
      char response_header[SMALL_BUF];
      memset(response_header, 0, SMALL_BUF);
      sprintf(response_header, "HTTP/1.0 200 OK\r\nServer: Linux Web Server \r\nfile name: %s \r\ncontent-type: %s \r\n\r\n", file_name, ct);
       
      printf("%s\n", response_header);
       
      //Make response data
      FILE* send_file;
       
      if(strstr(ct, "text")==NULL){
        send_file = fopen(file_name, "rb");
      }else{
        send_file = fopen(file_name, "r");
      }
       
      bzero(buffer, BUF_SIZE); 
      n = write(newsockfd, response_header, sizeof(response_header));
       
      while(feof(send_file) == 0){
        n = fread(buffer, 1, BUF_SIZE, send_file);
	if(n < 0) error("File Receive Error");
        if(n == 0) break;
	 
        printf("%s\n", buffer);
	 
        n = write(newsockfd, buffer, BUF_SIZE);
        if(n < 0) error("File Write Error");
       
        bzero(buffer, BUF_SIZE);
      }
      bzero(buffer, BUF_SIZE);
      if (n < 0) error("Write error");
      fclose(send_file);
    }
    close(sockfd);
    close(newsockfd);
    return 0; 
}

char *content_type(char *file){
  char extension[SMALL_BUF];
  char file_name[SMALL_BUF];
  strcpy(file_name, file);
  strtok(file_name, ".");
  strcpy(extension, strtok(NULL, "."));

  if(!strcmp(extension, "html") || !strcmp(extension, "htm"))
    return "text/html";
  else if(!strcmp(extension, "gif"))
    return "image/gif";
  else if(!strcmp(extension, "jpeg") || !strcmp(extension, "jpg"))
    return "image/jpeg";
  else if(!strcmp(extension, "mp3"))
    return "audio/mpeg3";
  else if(!strcmp(extension, "png"))
    return "image/png";
  else if(!strcmp(extension, "pdf"))
    return "application/pdf";
  else
    return "text/plain";
}
