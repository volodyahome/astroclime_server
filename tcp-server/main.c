/*
 * Copyright (c) 2020 Vladimir Smirnov
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

int main(int argc, char *argv[]) {
    //Change exist two arg
    if(argc < 2) {
        printf("Enter the path to the config file .ini");
        exit(EXIT_SUCCESS);
    }
    
    //PROC ID
    pid = getpid();
    
    //SIGNAL
    signal(SIGINT, signal_int);
    
    //CONF
    ini_t *config = ini_load(argv[1]); // argv[1] - path config file
    
    const char* server_host = ini_get(config, "server", "host");
    const unsigned long server_port = strtoul(ini_get(config, "server", "port"), NULL, 10);
    const int server_timeout_send = (int)ini_get(config, "server", "timeout_send");
    const int server_timeout_recv = (int)ini_get(config, "server", "timeout_recv");
    
    const char *log_file_name = ini_get(config, "log", "file_name");
    const char *log_file_path = ini_get(config, "log", "file_path");
    
    const char *firmware_file_name = ini_get(config, "firmware", "file_name");
    
    //LOG CFG
    SLogConfig slgCfg;
    slog_init(NULL, SLOG_INFO, 0);
    slgCfg.eColorFormat = SLOG_COLOR_TAG;
    strcpy(slgCfg.sFilePath, log_file_path);
    strcpy(slgCfg.sFileName, log_file_name);
    slgCfg.nTraceTid = 0;
    slgCfg.nToScreen = 0;
    slgCfg.nToFile = 1;
    slgCfg.nFlush = 0;
    slgCfg.nFlags = SLOG_FLAGS_ALL;
    slog_config_set(&slgCfg);
    
    //SOCKET SERVER
    tv_recv.tv_sec = server_timeout_recv;
    tv_recv.tv_usec = 0;
    
    tv_send.tv_sec = server_timeout_send;
    tv_send.tv_usec = 0;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_recv, sizeof(struct timeval));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_send, sizeof(struct timeval));
    
    if (sockfd == -1) {
        sprintf(buff_log, "- PID: %i - Socket creation failed", pid);
        slog_print(SLOG_FATAL, 1, buff_log);
        exit(1);
    } else {
        sprintf(buff_log, "- PID: %i - Socket successfully created", pid);
        slog_print(SLOG_INFO, 1, buff_log);
    }
    
    bzero(&serv_addr, sizeof(serv_addr));
    bzero(&client_addr, sizeof(client_addr));
    bzero(&serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    inet_aton(server_host, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(server_port);
    
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        sprintf(buff_log, "- PID: %i - Port reservation failed", pid);
        slog_print(SLOG_FATAL, 1, buff_log);
        exit(EXIT_FAILURE);
    }
    
    listen(sockfd, 0);
    
    connfd = accept(sockfd, (struct sockaddr*)NULL, NULL);
    
    if(connfd == -1) {
        sprintf(buff_log, "- PID: %i - Connection failed", pid);
        slog_print(SLOG_FATAL, 1, buff_log);
        
        close(connfd);
        
        exit(EXIT_FAILURE);
    } else {
        socklen_t len = sizeof(client_addr);
        getsockname(connfd, (struct sockaddr*)&client_addr, &len);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        client_port = ntohs(client_addr.sin_port);
        
        sprintf(buff_log, "- PID: %i - Connection success. FD: %i, IP: %s, Port: %d", pid, connfd, client_ip, client_port);
        slog_print(SLOG_INFO, 1, buff_log);
    }
    
    while (keep_run) {
        ssize_t len_recv;
        len_recv = recv(connfd, buff_recv, sizeof(buff_recv), 0);
        
        if(len_recv == -1) {
            slog_print(SLOG_ERROR, 1, "Error recv data");
            break;
        }
        
        if(len_recv > 0) {
            int resp;
            int fw_cb;
            int fw_sb;
            resp = parse_json(buff_recv);
            
            switch (resp) {
                case PING:
                    if(send(connfd, resp_ping, sizeof(resp_ping) , 0) == -1) {
                        slog_print(SLOG_ERROR, 1, "Error send ping");
                    }
                    
                    break;
                case STAT:
                    getrusage(RUSAGE_SELF, &usage);
                    
                    sprintf(buff_log, "CPU time: \n1. %ld.%061d sec user,\n2. %ld.%061d sec system\n3. mem %ld", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec, usage.ru_stime.tv_sec, usage.ru_stime.tv_usec, usage.ru_maxrss);

                    slog_print(SLOG_INFO, 1, buff_log);
                    
                    if(send(connfd, resp_stat, sizeof(resp_stat) , 0) == -1) {
                        slog_print(SLOG_ERROR, 1, "Error send fwinfo");
                    }
                    
                    break;
                case FWINFO:
                    info_fw(firmware_file_name);
                    md5_fw(firmware_file_name);
                    
                    time_fw(u);
                    
                    sprintf(buff_recv, resp_fwinfo, &file_info.st_size, &md5_str, &time_get_file);
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        slog_print(SLOG_ERROR, 1, "Error send fwinfo");
                    }
                    
                    break;
                case FWGET:
                    bzero(buff_recv, sizeof(buff_recv));
                    
                    fw_cb = get_fw_count_bytes();
                    fw_sb = get_fw_start_byte();
                    
                    read_fw(firmware_file_name, fw_sb, fw_cb);
                    
                    sprintf(buff_recv, resp_fwget, file_part);
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        slog_print(SLOG_ERROR, 1, "Error send fwget");
                    }
                    
                    break;
                case CLOSE:
                    if(send(connfd, resp_close, sizeof(resp_close) , 0) == -1) {
                        slog_print(SLOG_ERROR, 1, "Error send close");
                    }
                    
                    close(connfd);
                    
                    slog_print(SLOG_INFO, 1, "Connection with client closes");
                            
                    exit(EXIT_SUCCESS); //BAD
                    
                    break;
                case OTHER:
                    slog_print(SLOG_INFO, 1, "Another command sent");

                    break;
            }
        }
        
        bzero(buff_recv, sizeof(buff_recv));
    }
    
    return 0;
}

void read_fw(const char * firmware_file_name, int start, int count) {
    FILE        *fp;
    char        buff[count];
            
    fp = fopen(firmware_file_name, "rb");
    if (fp == NULL) {
        slog_print(SLOG_ERROR, 1, "Error occured while opening file");
    }
    
    if (fseek(fp, start, SEEK_SET) == 0) {
        if (fgets(buff, count, fp) != NULL ) {
            file_part = bin2hex((unsigned char *)buff, sizeof(buff));
        }
    } else {
        slog_print(SLOG_ERROR, 1, "Сould not set the position pointer in file");
    }
    
    fclose(fp);
}

void info_fw(const char * firmware_file_name) {
    FILE    *fp;
    
    if((fp= fopen(firmware_file_name, "rb"))==NULL)
    {
        slog_print(SLOG_ERROR, 1, "Error occured while opening file");
    }
    
    fstat(fileno(fp), &file_info);
    
    fclose(fp);
}

void md5_fw(const char * firmware_file_name){
    FILE    *fp;
    MD5_CTX md_context;
    
    unsigned long bytes;
    
    unsigned char data[1024];
    
    if((fp= fopen(firmware_file_name, "rb"))==NULL)
    {
        slog_print(SLOG_ERROR, 1, "Error occured while opening file");
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

void signal_int(int sig_num) {
    keep_run = 0;
    
    sprintf(buff_log, "INTERUPT signal - ^c (break): %i", sig_num);
    slog_print(SLOG_INFO, 1, buff_log);
}
