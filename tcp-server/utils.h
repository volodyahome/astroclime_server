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

#ifndef utils_h
#define utils_h

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>

#include <openssl/md5.h>

#include "logger.h"

#define MD5_STR_SIZE    33
#define TIME_GET_FILE   12

char *bin2hex(const unsigned char *bin, size_t len);

int hexchr2bin(const char hex, char *out);

size_t hexs2bin(const char *hex, unsigned char **out);

//Reading firmware
char read_fw(const char *firmware_file_name, int offset, int origin);

//Get info firmware
struct stat info_fw(const char *firmware_file_name);

//Get md5 firmware file
char md5_fw(const char *firmware_file_name);

//Get firmware request time
char time_fw(struct tm *u);

#endif /* utils_h */
