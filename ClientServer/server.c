#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>

#define SMALL_BUF 100
#define BUF_SIZE 1024

void* request_parser(int*);
void send_data(int* , char* , char*);
char* content_type(char*);

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char*argv[]){

    int server_socket;
    int opt = 1;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
    		&opt, sizeof(opt)))
    {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address, cli_addr;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8001);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    
    listen(server_socket, 5);
    int client_socket, clilen;
    while(1){
        clilen = sizeof(cli_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen);
        if(client_socket < 0) error("Error on accept");
	//send(client_socket, http_header, sizeof(http_header), 0);
	//close(client_socket);

	request_parser(&client_socket);
    }
    return 0;
}

void* request_parser(int* arg)
{
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

    if(strstr(req_line, "HTTP/")==NULL)
    {
        fclose(cli_read);
	fclose(cli_write);
	error("No HTTP.\n");
    }

    strcpy(method, strtok(req_line, " /"));
    strcpy(file_name, strtok(NULL, " /"));
    strcpy(ct, content_type(file_name));
    if(strcmp(method, "GET") != 0)
    {
        fclose(cli_read);
	fclose(cli_write);
	error("No GET method.\n");
    }
    fclose(cli_read);
    
    send_data(arg, ct, file_name);
}

void send_data(int* fp, char* ct, char* file_name)
{
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server: Linux Web Server \r\n";
    char cnt_len[] = "Content-length: 2048\r\n";
    char cnt_type[SMALL_BUF];
    char buf[BUF_SIZE];
    FILE* send_file;
    int m,n;

    sprintf(cnt_type, "Content-type: %s\r\n\r\n", ct);
    
    if(cnt_type == "text/html" || cnt_type == "text/plain")
    {
      send_file = fopen(file_name, "rt");
    }else
    {
      send_file = fopen(file_name, "rb");
    }

    strcat(protocol, server);
    strcat(protocol, cnt_len);
    strcat(protocol, cnt_type);
    printf("%s\n", protocol);
    n = fwrite(protocol, 1, BUF_SIZE, send_file);
    if (n < 0) error("ERROR writing to socket-1");
    while(1)
    {
      printf("%d\n", n);
      if (n < 0) error("ERROR reading from file");
      if (n==0) break;

      m = write(*fp, buf, BUF_SIZE);
      printf("%d\n", m);
      if (m < 0) error("ERROR writing to socket-2");
      memset(buf, 0, BUF_SIZE);
      
    }
    
    close(*fp);
    close(send_file);
}

char* content_type(char* file)
{
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
