/*
 * Copyright (c) 2020 volodyahome
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "main.h"
#include "logger.h"

char *level_strings[] = {
    "DEBUG", "INFO", "ERROR",
};

int main(int argc, char *argv[]) {
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (listenfd == -1) {
        logger(level_strings[LOG_ERROR], "socket creation failed...\n");
        exit(0);
    } else {
        logger(level_strings[LOG_INFO], "Socket successfully created..\n");
    }
    
    //clear memory
    bzero(&serv_addr, sizeof(serv_addr));
    bzero(&client_addr, sizeof(client_addr));
    bzero(&serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);
    
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        logger(level_strings[LOG_ERROR], "Port reservation failed...\n");
        exit(1);
    }
    
    listen(listenfd, 0);
    
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    
    if(connfd == -1) {
        logger(level_strings[LOG_ERROR], "Connection failed...\n");
        close(connfd);
    } else {
        
        sprintf(buff_log, "Connection success %i...\n", connfd);
        logger(level_strings[LOG_INFO], buff_log);
        
        socklen_t len = sizeof(client_addr);
        getsockname(connfd, (struct sockaddr*)&client_addr, &len);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        client_port = ntohs(client_addr.sin_port);
        
        sprintf(buff_log, "Client ip address: %s, port: %d\n", client_ip, client_port);
        logger(level_strings[LOG_INFO], buff_log);
    
    }
    
    while (1) {
        recv(connfd, buff_recv, sizeof(buff_recv), 0);
        
        if (strncmp("ping", buff_recv, 4) == 0) {
            
            logger(level_strings[LOG_INFO], resp_ping);
            
            send(connfd, &resp_ping, sizeof(resp_ping), 0);
        }
        
        if (strncmp("stat", buff_recv, 4) == 0) {
            getrusage(RUSAGE_SELF, &usage);
            
            sprintf(buff_log, "CPU time: \n1. %ld.%061d sec user,\n2. %ld.%061d sec system\n3. mem %ld", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec, usage.ru_stime.tv_sec, usage.ru_stime.tv_usec, usage.ru_maxrss);
            logger(level_strings[LOG_INFO], buff_log);
            
            if(send(connfd, resp_stat, sizeof(resp_stat) , 0) == -1) {
                perror("Error send fwinfo");
            }
        }
        
        if (strncmp("fwinfo", buff_recv, 6) == 0) {
            printf("%s\n", buff_recv);
            
            info_fw(file_name);
            
            md5_fw(file_name);
            
            time_fw(u);
            
            sprintf(buff_recv, resp_fwinfo, &file_info.st_size, &md5_str, &time_get_file);
            
            logger(level_strings[LOG_INFO], buff_recv);
            
            if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                perror("Error send fwinfo");
            }
        }
        
        if (strncmp("fwget", buff_recv, 5) == 0) {
            
        }
        
        if (strncmp("close", buff_recv, 5) == 0) {
            
            if(send(connfd, resp_close, sizeof(resp_close) , 0) == -1) {
                perror("Error send close");
            }
            
            sleep(1);
            
            close(connfd);
            
            logger(level_strings[LOG_INFO], "Connection with client closes.\n");
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
