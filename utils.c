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

#include "utils.h"

//Firmware info
struct stat file_info;

//Firmware time
char time_get_file[TIME_GET_FILE] = {0};

//Firmware file hash
unsigned char file_hash[MD5_DIGEST_LENGTH];

//Firmware part
char *file_part;

char *bin2hex(const unsigned char *bin, size_t len)
{
    char  *out;
    size_t  i;

    if (bin == NULL || len == 0)
        return NULL;

    out = malloc(len*2+1);
    for (i=0; i<len; i++) {
        out[i*2]   = "0123456789ABCDEF"[bin[i] >> 4];
        out[i*2+1] = "0123456789ABCDEF"[bin[i] & 0x0F];
    }
    out[len*2] = '\0';

    return out;
}

int hexchr2bin(const char hex, char *out)
{
    if (out == NULL)
        return 0;

    if (hex >= '0' && hex <= '9') {
        *out = hex - '0';
    } else if (hex >= 'A' && hex <= 'F') {
        *out = hex - 'A' + 10;
    } else if (hex >= 'a' && hex <= 'f') {
        *out = hex - 'a' + 10;
    } else {
        return 0;
    }

    return 1;
}

size_t hexs2bin(const char *hex, unsigned char **out)
{
    size_t len;
    char   b1;
    char   b2;
    size_t i;

    if (hex == NULL || *hex == '\0' || out == NULL)
        return 0;

    len = strlen(hex);
    if (len % 2 != 0)
        return 0;
    len /= 2;

    *out = malloc(len);
    memset(*out, 'A', len);
    for (i=0; i<len; i++) {
        if (!hexchr2bin(hex[i*2], &b1) || !hexchr2bin(hex[i*2+1], &b2)) {
            return 0;
        }
        (*out)[i] = (b1 << 4) | b2;
    }
    return len;
}

char read_fw(const char * firmware_file_name, int start, int count) {
    FILE    *fp;
    char    buff[count];
            
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
    
    return *file_part;
}

struct stat info_fw(const char * firmware_file_name) {
    FILE    *fp;
    
    fp = fopen(firmware_file_name, "rb");
    if(fp == NULL)
    {
        slog_print(SLOG_ERROR, 1, "Error occured while opening file");
    }
    
    fstat(fileno(fp), &file_info);
    
    fclose(fp);
    
    return file_info;
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

char time_fw(struct tm *u) {
    
    bzero(time_get_file, TIME_GET_FILE);
    
    const time_t timer = time(NULL);
    
    u = localtime(&timer);
    
    strftime(time_get_file, TIME_GET_FILE, "%d%m%Y%H%M", u);
    
    return *time_get_file;
}
