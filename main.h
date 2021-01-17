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
#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <hiredis/hiredis.h>

#include "logger.h"
#include "json.h"
#include "ini.h"
#include "utils.h"

#define SERVER_VERSION "0.1.1"

#define BUFF_SIZE       2048

//Process id
pid_t pid;

//Timeout send and recv data
struct timeval tv_send;
struct timeval tv_recv;

struct statm_t{
    unsigned long   size;      //total program size (same as VmSize in /proc/[pid]/status)
    unsigned long   resident;  //resident set size (same as VmRSS in /proc/[pid]/status)
    unsigned long   share;     //shared pages (from shared mappings)
    unsigned long   text;      //text (code)
    unsigned long   lib;       //library (unused in Linux 2.6)
    unsigned long   data;      //data + stack
    unsigned long   dt;        //dirty pages (unused in Linux 2.6)
};

//Buffer log
char *buff_log = NULL;

//Keep run
int keep_run = 1;

//Socket
int sockfd = 0;

//Count connects
int count_conn = 0;

//Server info and client info
struct sockaddr_in serv_addr, client_addr;

//Ip address of the connected client
char *client_ip = NULL;

//Port of the connected client
unsigned int client_port;

//Firmware path
const char *firmware_file_name;

struct tm *u;

//Server statistics
struct rusage usage;

pthread_attr_t pthread_attr;

typedef struct pthread_arg_t {
    int                 connfd;
    int                 buff_recv_size;
    int                 buff_send_size;
    struct sockaddr_in  client_address;
    char                redis_host;
    int                 redis_port;
} pthread_arg_t;

pthread_arg_t *pthread_arg;

socklen_t client_address_len;

pthread_t pthread;

int daemon_server(char *cnf_path);

void *pthread_routine(void *arg);

//Send data
void send_messange(int connfd, char *resp, char *error_message, int buff_send_size);

//Log error
void log_error(char *buff_log, char *message);

#endif /* MAIN_H */
