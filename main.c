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
    
    //CONF
    ini_t *config = ini_load(argv[1]); // argv[1] - path config file
    
    const char* server_host = ini_get(config, "server", "host");
    const unsigned long server_port = strtoul(ini_get(config, "server", "port"), NULL, 10);
    const char* server_timeout_send = ini_get(config, "server", "timeout_send");
    const char* server_timeout_recv = ini_get(config, "server", "timeout_recv");
    
    const char *log_file_name = ini_get(config, "log", "file_name");
    const char *log_file_path = ini_get(config, "log", "file_path");
    
    firmware_file_name = ini_get(config, "firmware", "file_name");
    
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
    tv_recv.tv_sec = (int64_t)server_timeout_recv;
    tv_recv.tv_usec = 0;
    
    tv_send.tv_sec = (int64_t)server_timeout_send;
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
    
    if (listen(sockfd, 0) == -1) {
        slog_print(SLOG_FATAL, 1, "Listen socket error");
        exit(EXIT_FAILURE);
    }
    
    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_attr) != 0) {
        slog_print(SLOG_FATAL, 1, "pthread_attr_init");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        slog_print(SLOG_FATAL, 1, "pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }
    
    while(keep_run) {
        pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
        if (!pthread_arg) {
            perror("malloc");
            continue;
        }
        
        /* Accept connection to client. */
        client_address_len = sizeof pthread_arg->client_address;
        
        pthread_arg->connfd = accept(sockfd, (struct sockaddr*)&pthread_arg->client_address, &client_address_len);
        if(pthread_arg->connfd == -1) {
            sprintf(buff_log, "- PID: %i - Connection failed", pid);
            slog_print(SLOG_FATAL, 1, buff_log);
            
            free(pthread_arg);
            
            continue;
        } else {
            socklen_t len = sizeof(pthread_arg->client_address);
            getsockname(pthread_arg->connfd, (struct sockaddr*)&pthread_arg->client_address, &len);
            inet_ntop(AF_INET, &pthread_arg->client_address.sin_addr, client_ip, sizeof(client_ip));
            client_port = ntohs(pthread_arg->client_address.sin_port);
            
            count_conn++;
            
            sprintf(buff_log, "- PID: %i - Connection success. FD: %i, IP: %s, Port: %d", pid, pthread_arg->connfd, client_ip, client_port);
            slog_print(SLOG_INFO, 1, buff_log);
        }
        
        /* Create thread to serve connection to client. */
        if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
            slog_print(SLOG_FATAL, 1, "Error pthread create");
            
            free(pthread_arg);
            
            continue;
        }
    }
}

void *pthread_routine(void *arg) {
    
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int connfd = pthread_arg->connfd;
    int keep_run = 1;
    
    while (keep_run) {
        //Buffer for received data
        char buff_recv[BUFF_SIZE] = {0};
        
        ssize_t len_recv;
        len_recv = recv(connfd, buff_recv, sizeof(buff_recv), 0);
        
        if(len_recv == -1) {
            slog_print(SLOG_ERROR, 1, "Error recv data");
        }
        
        if(len_recv > 0) {
            
            int         resp;
            int         fw_cb;
            int         fw_sb;
            char        *md5_hash;
            struct stat ifw;
            char        *fw_receipt_time;
            char        *fp;
            
            //Resp command
            const char *resp_ping;
            const char *resp_close;
            const char *resp_other;
            const char *resp_stat;
            const char *resp_fwinfo;
            const char *resp_fwget;
            
            sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Recv data: %s", pid, client_ip, client_port, buff_recv);
            slog_print(SLOG_INFO, 1, buff_log);
            
            resp = parse_json(buff_recv);
            
            bzero(buff_recv, sizeof(buff_recv));
            
            switch (resp) {
                case PING:
                    
                    resp_ping = answer_json(PING);
                    
                    sprintf(buff_recv, "%s", resp_ping);
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Error: %s", pid, client_ip, client_port, "Error send ping");
                        slog_print(SLOG_ERROR, 1, buff_log);
                    }
                    
                    break;
                case STAT:
                    
                    getrusage(RUSAGE_SELF, &usage);
                    
                    resp_stat = answer_json(STAT);
                    
                    sprintf(buff_recv, resp_stat, count_conn, usage.ru_maxrss);
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Error: %s", pid, client_ip, client_port, "Error send stat");
                        slog_print(SLOG_ERROR, 1, buff_log);
                    }
                    
                    break;
                case FWINFO:
                    
                    resp_fwinfo = answer_json(FWINFO);
                    
                    ifw = info_fw(firmware_file_name);
                    
                    md5_hash = md5_fw(firmware_file_name);
                    
                    fw_receipt_time = time_fw(u);
                    
                    sprintf(buff_recv, resp_fwinfo, ifw.st_size, md5_hash, fw_receipt_time);
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Error: %s", pid, client_ip, client_port, "Error send fwinfo");
                        slog_print(SLOG_ERROR, 1, buff_log);
                    }
                    
                    break;
                case FWGET:
                    resp_fwget = answer_json(FWGET);
                    
                    fw_cb = get_fw_count_bytes();
                    fw_sb = get_fw_start_byte();
                    
                    fp = read_fw(firmware_file_name, fw_sb, fw_cb);
                    
                    sprintf(buff_recv, resp_fwget, fp);
                    
                    free(fp); //Freeing dynamically allocated memory(file_part)
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Error: %s", pid, client_ip, client_port, "Error send fwget");
                        slog_print(SLOG_ERROR, 1, buff_log);
                    }
                    
                    break;
                case CLOSE:
                    
                    resp_close = answer_json(CLOSE);
                    
                    sprintf(buff_recv, "%s", resp_close);
                    
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Error: %s", pid, client_ip, client_port, "Error send close");
                        slog_print(SLOG_ERROR, 1, buff_log);
                    }
                    
                    close(connfd);
                    
                    count_conn--;
                    
                    sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Msg: %s", pid, client_ip, client_port, "Connection with client closes");
                    slog_print(SLOG_INFO, 1, buff_log);
                    
                    keep_run = 0;
                                            
                    break;
                case OTHER:
                    
                    resp_other = error_json(INVALID_COMMAND);
                    
                    sprintf(buff_recv, "%s", resp_other);
                    if(send(connfd, buff_recv, sizeof(buff_recv) , 0) == -1) {
                        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Msg: %s", pid, client_ip, client_port, "Error send other command");
                        slog_print(SLOG_ERROR, 1, buff_log);
                    }
                    
                    sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Msg: %s", pid, client_ip, client_port, "Another command sent");
                    slog_print(SLOG_ERROR, 1, buff_log);
                    
                    break;
            }
        }
        
        bzero(buff_recv, sizeof(buff_recv));
    }
    
    return 0;
}
