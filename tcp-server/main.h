//
//  main.h
//  tcp-server
//
//  Created by BS on 21.12.2020.
//

#ifndef main_h
#define main_h

struct statm_t{
    unsigned long   size;      //total program size (same as VmSize in /proc/[pid]/status)
    unsigned long   resident;  //resident set size (same as VmRSS in /proc/[pid]/status)
    unsigned long   share;     //shared pages (from shared mappings)
    unsigned long   text;      //text (code)
    unsigned long   lib;       //library (unused in Linux 2.6)
    unsigned long   data;      //data + stack
    unsigned long   dt;        //dirty pages (unused in Linux 2.6)
};

int listenfd = 0, connfd = 0;

//Server info and client info
struct sockaddr_in serv_addr, client_addr;

//Ip address of the connected client
char client_ip[16];

//Port of the connected client
unsigned int client_port;

//Buffer for received data
char buff_recv[512];

//Firmware path
char * file_name = "/Users/bs/Desktop/tcp-server/tcp-server/fw/firmware";

//Firmware info
struct stat file_info;

//Firmware file hash
unsigned char file_hash[MD5_DIGEST_LENGTH];
char md5_str[33];

//Firmware time
char time_get_file[12];
struct tm *u;

struct rusage usage;

//Resp command ping
char resp_ping[] = "{\"resp\":\"pong\",\"data\":[1,2,3]}\n";

//Resp command fwinfo
char resp_fwinfo[512] = "{\"fwsize\":%llu,\"fwmd5\":\"%s\",\"dt\":\"%s\"}\n";


//Reading firmware
int read_fw(char * file_name);

//Get info firmware
void info_fw(char * file_name);

//Get md5 firmware file
void md5_fw(char * file_name);

//Get firmware request time
void time_fw(struct tm *u);

#endif /* main_h */
