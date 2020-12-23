//
//  main.c
//  tcp-server
//
//  Created by BS on 19.12.2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#include "main.h"

int main(int argc, char *argv[]) {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created..\n");
    }
    
    //clear memory
    bzero(&serv_addr, sizeof(serv_addr));
    bzero(&client_addr, sizeof(client_addr));
    
    //fill memory with a constant byte
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);
    
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("Port reservation failed...\n");
        exit(1);
    }
    
    listen(listenfd, 0);
    
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    
    if(connfd == -1) {
        printf("Connection failed...\n");
        close(connfd);
    } else {
        printf("Connection success %i...\n", connfd);
        socklen_t len = sizeof(client_addr);
        getsockname(connfd, (struct sockaddr*)&client_addr, &len);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        client_port = ntohs(client_addr.sin_port);
        printf("Client ip address: %s, port: %d\n", client_ip, client_port);
    }
    
    while (1) {
        recv(connfd, buff_recv, sizeof(buff_recv), 0);
        
        if (strncmp("ping", buff_recv, 4) == 0) {
            send(connfd, &resp_ping, sizeof(resp_ping), 0);
        }
        
        if (strncmp("stat", buff_recv, 4) == 0) {
            getrusage(RUSAGE_SELF, &usage);
            
            printf("CPU time: \n 1. %ld.%061d sec user,\n 2. %ld.%061d sec system\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec, usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
            
            if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                perror("Error send fwinfo");
            }
        }
        
        if (strncmp("fwinfo", buff_recv, 6) == 0) {
            printf("%s\n", buff_recv);
            
            info_fw(file_name);
            
            md5_fw(file_name);
            
            time_fw(u);
            
            sprintf(buff_recv, resp_fwinfo, &file_info.st_size, &md5_str, &time_get_file);
            
            if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                perror("Error send fwinfo");
            }
        }
        
        if (strncmp("fwget", buff_recv, 5) == 0) {
            
        }
        
        if (strncmp("close", buff_recv, 5) == 0) {
            close(connfd);
            sleep(1);
            printf("Connection with client closes.\n");
            break;
        }
        
        bzero(buff_recv, sizeof(buff_recv));
    }
    
    return 0;
}

int read_fw(char * file_name) {
    FILE * fp;
    char c;
    
    info_fw(file_name);
    
    if((fp= fopen(file_name, "rb"))==NULL)
    {
        perror("Error occured while opening file");
        return 1;
    }
    
    while((c=getc(fp))!= EOF)
    {
        printf("%02x\n", c);
    }
    
    fclose(fp);
    
    return 0;
}

void info_fw(char * file_name) {
    FILE * fp;
    
    if((fp= fopen(file_name, "rb"))==NULL)
    {
        perror("Error occured while opening file");
    }
    
    fstat(fileno(fp), &file_info);
    
    fclose(fp);
}

void md5_fw(char * file_name){
    FILE *fp;
    
    MD5_CTX md_context;
    
    unsigned long bytes;
    
    unsigned char data[1024];
    
    if((fp= fopen(file_name, "rb"))==NULL)
    {
        perror("Error occured while opening file");
    }
    
    MD5_Init(&md_context);
    while ((bytes = fread(data, 1, 1024, fp)) != 0) {
        MD5_Update(&md_context, data, bytes);
    }
    
    MD5_Final(file_hash, &md_context);
    
    for(int i = 0; i < MD5_DIGEST_LENGTH; ++i){
        sprintf(&md5_str[i*2], "%02x", (unsigned int)file_hash[i]);
    }
    
    fclose (fp);
}

void time_fw(struct tm *u) {
    
    bzero(time_get_file, 12);
    
    const time_t timer = time(NULL);
    
    u = localtime(&timer);
    
    strftime(time_get_file, 12, "%d%m%Y%H%M", u);
}
