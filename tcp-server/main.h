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
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "logger.h"
#include "json.h"
#include "ini.h"
#include "utils.h"

#define SERVER_VERSION "0.1.1"

#define CLIENT_IP_SIZE  16
#define BUFF_SIZE       1024

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
char buff_log[BUFF_SIZE];

//Keep run
int keep_run = 1;

//Socket
int sockfd = 0;

//Count connects
int count_conn = 0;

//Server info and client info
struct sockaddr_in serv_addr, client_addr;

//Ip address of the connected client
char client_ip[CLIENT_IP_SIZE];

//Port of the connected client
unsigned int client_port;

//Firmware path
const char *firmware_file_name;

struct tm *u;

//Server statistics
struct rusage usage;

//Resp command ping
char resp_ping[BUFF_SIZE]   =   "{\"resp\":\"pong\",\"data\":[1,2,3]}";

//Resp command fwinfo
char resp_fwinfo[BUFF_SIZE] =   "{\"fwsize\":%llu,\"fwmd5\":\"%s\",\"dt\":\"%s\"}";

//Resp comand fwget
char resp_fwget[BUFF_SIZE]  =   "{\"buff\":\"%s\"}";

//Resp command stat
char resp_stat[BUFF_SIZE]   =   "{\"conn\":%d,\"mem_used\":%ld\"}";

pthread_attr_t pthread_attr;

typedef struct pthread_arg_t {
    int connfd;
    struct sockaddr_in client_address;
} pthread_arg_t;

pthread_arg_t *pthread_arg;

socklen_t client_address_len;

pthread_t pthread;

void *pthread_routine(void *arg);

#endif /* MAIN_H */
