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
    if(argc < 3) {
        printf("Usage ./astroclime_server -d for daemon or ./astroclime_server -i for interactive\nEnter the path to the config file .ini\n");
        exit(EXIT_SUCCESS);
    }

    if (strcmp(argv[2],"-i")==0) {
        daemon_server(argv[1]);
    } else if (strcmp(argv[2],"-d")==0) {
        pid_t parpid;

        parpid=fork();

        if(parpid < 0) {
            printf("\ncan't fork");
            exit(EXIT_FAILURE);
        }
        else if(parpid != 0) {
            exit(EXIT_SUCCESS);
        }

        setsid();

        daemon_server(argv[1]);
    }

    return 0;
}


int daemon_server(char *cnf_path) {
    //PROC ID
    pid = getpid();

    //CONF
    ini_t *config = ini_load(cnf_path); //path config file
    
    const char *server_host         = ini_get(config, "server", "host");
    const unsigned long server_port = strtoul(ini_get(config, "server", "port"), NULL, 10);
    const char *server_timeout_send = ini_get(config, "server", "timeout_send");
    const char *server_timeout_recv = ini_get(config, "server", "timeout_recv");
    const int server_buff_log       = atoi(ini_get(config, "server", "buff_log"));
    
    const char *log_file_name       = ini_get(config, "log", "file_name");
    const char *log_file_path       = ini_get(config, "log", "file_path");
    
    firmware_file_name              = ini_get(config, "firmware", "file_name");

    buff_log = malloc(server_buff_log * sizeof(char));
    
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
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_host);
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
            slog_print(SLOG_FATAL, 1, "Error malloc pthread_arg");
            continue;
        }

        pthread_arg->buff_recv_size = atoi(ini_get(config, "server", "buff_recv_size"));
        pthread_arg->buff_send_size = atoi(ini_get(config, "server", "buff_send_size"));
        //Redis conf
        pthread_arg->redis_host = ini_get(config, "redis", "redis_host");
        pthread_arg->redis_port = atoi(ini_get(config, "redis", "redis_port"));

        //Accept connection to client
        client_address_len = sizeof pthread_arg->client_address;
        
        pthread_arg->connfd = accept(sockfd, (struct sockaddr*)&pthread_arg->client_address, &client_address_len);
        
        if(pthread_arg->connfd == -1) {
            sprintf(buff_log, "- PID: %i - Connection failed", pid);
            slog_print(SLOG_FATAL, 1, buff_log);
            
            free(pthread_arg);
            
            continue;
        } else {
            client_ip = inet_ntoa(pthread_arg->client_address.sin_addr);
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

    free(buff_log);
    buff_log = NULL;
    
    ini_free(config);

    return 0;
}

void *pthread_routine(void *arg) {
    pthread_arg_t *pthread_arg  = (pthread_arg_t *)arg;
    int keep_run                = 1;
    size_t memory_size          = pthread_arg->buff_recv_size * sizeof(char); 
    char *buff_recv             = NULL; //Request
    int connfd = pthread_arg->connfd;

    buff_recv = (char *)malloc(memory_size);

    if(buff_recv == NULL) {
        log_error(buff_log, "Error malloc");
        keep_run = 0;
        close(connfd);
    }

    while (keep_run) {

        ssize_t len_recv;
        struct stat ifw;
        int fw_cb;
        int fw_sb;
        redisContext *c;
        redisReply *reply;
        char buff_tmp[BUFF_SIZE]    = {0}; 
        char *resp                  = NULL; //JSON
        char *md5_hash              = NULL; //MD5 hash file
        char *fw_receipt_time       = NULL; //Receipt time
        char *fp                    = NULL;
        char *err_msg               = NULL;
                    
        
        len_recv = recv(connfd, buff_recv, memory_size, 0);
        
        if(len_recv == -1) {
            slog_print(SLOG_ERROR, 1, "Error recv data");
        }

        if(len_recv == 0) {
            free(buff_recv);
            buff_recv = NULL;

            free(pthread_arg);
            pthread_arg = NULL;

            keep_run = 0;

            close(connfd);

            sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Msg: %s", pid, client_ip, client_port, "Connection with client closes");
            slog_print(SLOG_INFO, 1, buff_log);
        }
        
        if(len_recv > 0) {            
            sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Recv data: %s, Recv_len: %lu", pid, client_ip, client_port, buff_recv, len_recv);
            slog_print(SLOG_INFO, 1, buff_log);
            
            switch (parse_json(buff_recv)) {
                case PING://{"cmd":"ping"}
                    
                    resp = answer_json(PING, memory_size);
                    
                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }
                     
                    err_msg = "Error send answer command ping";
                    send_messange(connfd, resp, err_msg, pthread_arg->buff_send_size);
                    break;
                case RANDOM://{"cmd":"random","len":120}                    
                    resp = answer_json(RANDOM, memory_size);

                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }

                    err_msg = "Error send answer command random";
                    send_messange(connfd, resp, err_msg, pthread_arg->buff_send_size);
                    break;
                case STAT://{"cmd":"stat"}
                    
                    resp = answer_json(STAT, memory_size);
                    
                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }
                    
                    getrusage(RUSAGE_SELF, &usage);
                    
                    sprintf(buff_tmp, resp, count_conn, usage.ru_maxrss);
                    
                    err_msg = "Error send stat";
                    send_messange(connfd, buff_tmp, err_msg, pthread_arg->buff_send_size);
                                        
                    break;
                case FWINFO://{"cmd":"fwinfo"}
                    
                    resp = answer_json(FWINFO, memory_size);
                    
                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }
                    
                    ifw = info_fw(firmware_file_name);

                    if(ifw.st_size == 0) {
                        resp = error_json(NO_SUCH_FILE);
                    
                        if(resp == NULL) {
                            err_msg = "Error malloc";
                            log_error(buff_log, err_msg);
                            
                            break;
                        }

                        err_msg = "Error send fwinfo";

                        send_messange(connfd, resp, err_msg, pthread_arg->buff_send_size);
                        break;
                    }
                    
                    md5_hash = md5_fw(firmware_file_name);
                    
                    fw_receipt_time = time_fw(u);
                    
                    sprintf(buff_tmp, resp, ifw.st_size, md5_hash, fw_receipt_time);
                    
                    err_msg = "Error send fwinfo";
                    send_messange(connfd, buff_tmp, err_msg, pthread_arg->buff_send_size);
                                        
                    break;
                case FWGET://{"cmd":"fwget","count":40,"start":0}
                    resp = answer_json(FWGET, memory_size);
                    
                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }
                    
                    fw_cb = get_fw_count_bytes();
                    fw_sb = get_fw_start_byte();
                    
                    fp = read_fw(firmware_file_name, fw_sb, fw_cb);
                    
                    sprintf(buff_tmp, resp, fp);
                    
                    free(fp); //Freeing dynamically allocated memory(file_part)
                    
                    err_msg = "Error send fwget";
                    send_messange(connfd, buff_tmp, err_msg, pthread_arg->buff_send_size);
                                        
                    break;
                case CLOSE://{"cmd":"close"}
                    
                    resp = answer_json(CLOSE, memory_size);
                    
                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }
                              
                    err_msg = "Error send close";
                    send_messange(connfd, resp, err_msg, pthread_arg->buff_send_size);
                    
                    close(connfd);
                    
                    count_conn--;
                    
                    sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Msg: %s", pid, client_ip, client_port, "Connection with client closes");
                    slog_print(SLOG_INFO, 1, buff_log);
                    
                    keep_run = 0;
                                                                
                    break;

                case ANALYTICS://{"cmd":"analytics","data":"[1,2,3,4]", "uid":"xxx"}
                    c = redisConnect(pthread_arg->redis_host, pthread_arg->redis_port);
                    if (c == NULL || c->err) {
                        if (c) {
                            err_msg = "Error connection redis";
                            log_error(buff_log, err_msg);
                            redisFree(c);
                        } else {
                            err_msg = "Connection redis error: can't allocate redis context";
                            log_error(buff_log, err_msg);
                        }

                        break;
                    }
                    reply = redisCommand(c, "SET test %s", "test");
                    
                    redisFree(c);
                    
                    break;
                case OTHER://Invalid command
                    
                    resp = error_json(INVALID_COMMAND);
                    
                    if(resp == NULL) {
                        err_msg = "Error malloc";
                        log_error(buff_log, err_msg);
                        
                        break;
                    }
                    
                    err_msg = "Error send other command";
                    send_messange(connfd, resp, err_msg, pthread_arg->buff_send_size);
                    
                    break;
            }
            
            free(resp); //Freeing dynamically allocated memory(resp)
            err_msg = NULL;
        }   
    }

    free(buff_recv); //Freeing dynamically allocated memory(buff_recv)

    return 0;
}

void send_messange(int connfd, char *resp, char *error_message, int buff_send_size) {
    int len_resp    = strlen(resp) + 1;
    char *buff_send = (char *)malloc(len_resp * sizeof(char));

    strcpy(buff_send, resp);
    
    if(send(connfd, buff_send, len_resp , 0) == -1) {
        sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Error: %s", pid, client_ip, client_port, error_message);
        slog_print(SLOG_ERROR, 1, buff_log);
    }
    
    free(buff_send);
    buff_send = NULL;
}

void log_error(char *buff_log, char *message) {
    sprintf(buff_log, "- PID: %i - IP: %s, Port: %d, Msg: %s", pid, client_ip, client_port, message);
    slog_print(SLOG_ERROR, 1, buff_log);
}
