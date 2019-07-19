/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
/* whsqkaak.LSH
   Develop Environment : Ubuntu 16.04
   Test Browser : Chrome
*/

#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>

#define SMALL_BUF 100 // Small Buffer Size (ex : content name, content type)
#define BUF_SIZE 1024 // Data Buffer Size

int file_size(FILE*); // Input file, return file size
char *content_type(char*); // Input content name, return content type 

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
    FILE* send_file;
    char buffer[BUF_SIZE]; // BUF_SIZE 1024
    /*sockaddr_in: Structure Containing an Internet Address*/
    struct sockaddr_in serv_addr, cli_addr;
    
    int m, n; // temp variant m : sprintf cursor, n : error detector

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
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); //atoi converts from String to Integer
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
    serv_addr.sin_port = htons(portno); //convert from host to network byte order
     
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
             error("ERROR on binding");
     
    listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5
    
    // accept request repeatedly 
    while(1){
      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0) 
           error("ERROR on accept");
      
      bzero(buffer, BUF_SIZE); 
      n = read(newsockfd,buffer,255); //Read is a block function. It will read at most 255 bytes
      if (n < 0) error("ERROR reading from socket");
      
      printf("%s\n", buffer); // print request message at console
      
      // Request message parsing
      char method[SMALL_BUF]; // request message
      char file_name[SMALL_BUF]; // request file name
      char ct[SMALL_BUF]; // request content type
      char* response_header; // response header
      int size_file; // for size file

      if(strstr(buffer, "HTTP/") == NULL){
        error("No Http.");
      }

      // Request message parse and copy
      strcpy(method, strtok(buffer, " /"));
      strcpy(file_name, strtok(NULL, " /"));
      strcpy(ct, content_type(file_name));
      if(strcmp(method, "GET")!=0)
        error("No GET method.");
      
      // Make response message
      response_header = (char*)malloc(sizeof(char)*1024);

      send_file = fopen(file_name, "r");
      size_file = file_size(send_file);
      
      // Use sprintf for make response header
      m = sprintf(response_header, "HTTP/1.1 200 OK\n");
      m += sprintf(response_header + m, "Server : LSH Server\n");
      m += sprintf(response_header + m, "Content-Type: %s\n", ct);
      m += sprintf(response_header + m, "Content-Length: %d\n\n", size_file);
      
      // write response header
      n = write(newsockfd, response_header, strlen(response_header));
      if(n<0) error("File Header Write Error");
      
      bzero(buffer, BUF_SIZE);
      // read and write repeatedly using buffer
      while((n=fread(buffer, 1, BUF_SIZE, send_file) > 0)){
        // read send_file 1 by 1 BUF_SIZE times, and load to buffer
	n = write(newsockfd, buffer, BUF_SIZE);
	// write buffer to newsockfd as much as BUF_SIZE
	if(n < 0) error("File Write Error");
      }

      if(n < 0) error("File Read Error");
      // clear buffer, response_header, send_file
      bzero(buffer, BUF_SIZE);
      free(response_header);
      fclose(send_file);
    }
    // close socket
    close(sockfd);
    close(newsockfd);
    return 0; 
}

// To get a file size
int file_size(FILE* file){
  int size;
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return size;
}

// To get a content type
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
  else if(!strcmp(extension, "ico"))
    return "image/x-icon";
  else
    return "text/plain";
}
