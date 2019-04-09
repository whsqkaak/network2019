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
#include <signal.h>
#define SMALL_BUF 100
#define BUF_SIZE 1024

void *request_parser(int*);
void send_data(FILE*, char*, char*);
char *content_type(char *);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void cleanExit(){
    printf("I will exit~!\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGTERM,cleanExit);
    signal(SIGINT, cleanExit);

    int opt = 1;

    int sockfd, newsockfd; //descriptors return from socket and accept system calls
    int portno; // port number
    socklen_t clilen;
     
    char receive_buffer[256];
     
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
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
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
     
    while(1){
       clilen = sizeof(cli_addr);
       /*accept function: 
       1) Block until a new connection is established
       2) the new socket descriptor will be used for subsequent communication with the newly connected client.
       */
       newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
       if (newsockfd < 0) 
           error("ERROR on accept");
        
        request_parser(&newsockfd);
       //bzero(receive_buffer,256);
       //n = read(newsockfd,receive_buffer,255); //Read is a block function. It will read at most 255 bytes
       
       //if (n < 0) error("ERROR reading from socket");
       //printf("Here is the message: %s\n",receive_buffer);
         
       //n = write(newsockfd,"I got your message",18); //NOTE: write function returns the number of bytes actually sent out Ñ> this might be less than the number you told it to send
       //if (n < 0) error("ERROR writing to socket");
       if (strncmp(receive_buffer, "exit", 4) == 0) break;
    }
    close(sockfd);
    close(newsockfd);
     
     return 0; 
}

void* request_parser(int *arg){
    int cli_sockfd = *arg;
    char req_line[SMALL_BUF];
    FILE* cli_read;
    FILE* cli_write;

    char method[10];
    char ct[15];
    char file_name[30];

    cli_read = fdopen(cli_sockfd, "r");
    cli_write = fdopen(dup(cli_sockfd), "w");
    fgets(req_line, SMALL_BUF, cli_read);
    if(strstr(req_line, "HTTP/")==NULL){
        fclose(cli_read);
        fclose(cli_write);
        error("No HTTP.\n");
    }
    
    strcpy(method, strtok(req_line, " /"));
    strcpy(file_name, strtok(NULL, " /"));
    strcpy(ct, content_type(file_name));
    if(strcmp(method, "GET")!=0){
        fclose(cli_read);
        fclose(cli_write);
        error("No GET method.\n");
    }
    fclose(cli_read);
    send_data(cli_write, ct, file_name);
}

void send_data(FILE* fp, char* ct, char* file_name){
    char protocol[] = "HTTP/1.0 200 OK\r\n";
    char server[] = "Server: Linux Web Server \r\n";
    char cnt_len[] = "Content-length: 2048\r\n";
    char cnt_type[SMALL_BUF];
    char buf[BUF_SIZE];
    FILE* send_file;
    
    sprintf(cnt_type, "Content-type: %s\r\n\r\n", ct);

    send_file = fopen(file_name, "r");
    
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    printf("%s %s %s %s", protocol, server, cnt_len, cnt_type);
    int read;
    while(1){
        read = fread(buf, sizeof(char), BUF_SIZE, send_file); 
        if(read < 0){
	  printf("File Receive Error");
	  exit(1);
	}
	if(read == 0) break;
	printf("%s\n", buf);
	write(fp, buf, BUF_SIZE); 
	memset(buf, 0, BUF_SIZE);	
    }
    fflush(fp);
    fclose(fp);
    fflush(send_file);
    fclose(send_file);
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
    else if(!strcmp(extension, "pdf"))
        return "application/pdf";
    else if(!strcmp(extension, "png"))
        return "image/png";
    else
        return "text/plain";
}
