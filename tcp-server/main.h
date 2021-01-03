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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#include "logger.h"
#include "json.h"
#include "ini.h"
#include "utils.h"

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
char buff_log[SLOG_BUFF_LOG];

//Keep run
int keep_run = 1;
//Signal handler
void signal_int(int sig_num);

int sockfd = 0;

int connfd = 0;

int count_conn = 0;

//Server info and client info
struct sockaddr_in serv_addr, client_addr;

//Ip address of the connected client
char client_ip[16];

//Port of the connected client
unsigned int client_port;

//Buffer for received data
char buff_recv[512] = {0};

//Firmware path
const char *firmware_file_name;

//Firmware info
struct stat file_info;

//Firmware file hash
unsigned char file_hash[MD5_DIGEST_LENGTH];
char md5_str[33] = {0};

//Firmware time
char time_get_file[12] = {0};
struct tm *u;

//Firmware part
char *file_part;

//Server statistics
struct rusage usage;

//Resp command ping
char resp_ping[512] = "{\"resp\":\"pong\",\"data\":[1,2,3]}";

//Resp command fwinfo
char resp_fwinfo[512] = "{\"fwsize\":%llu,\"fwmd5\":\"%s\",\"dt\":\"%s\"}";

//Resp comand fwget
char resp_fwget[512] = "{\"buff\":\"%s\"}";

//Resp command stat
char resp_stat[512] = "{\"conn\":1,\"mem_used\":7.4707794189453125}";

//Resp command close
char resp_close[512] = "{\"resp\":\"bay\"}";

//Reading firmware
void read_fw(const char * firmware_file_name, int offset, int origin);

//Get info firmware
void info_fw(const char * firmware_file_name);

//Get md5 firmware file
void md5_fw(const char * firmware_file_name);

//Get firmware request time
void time_fw(struct tm *u);

#endif /* MAIN_H */
