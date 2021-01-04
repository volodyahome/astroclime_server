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

#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

#include "logger.h"

enum Command {
    PING,       //0
    RANDOM,     //1
    CLOSE,      //2
    STAT,       //3
    FWINFO,     //4
    FWGET,      //5
    ANALYTICS,  //6
    OTHER       //7
};

enum Error {
    INVALID_COMMAND = 1,    //1
    NO_SUCH_FILE,           //2
    NOT_READ_FILE,          //3
    EMPTY_REQUEST,          //4
    COMMAND_NOT_FOUND,      //5
    DEFAULT_ERROR           //6
};

//Parse recv JSON
int parse_json(char * buff_recv);

//Create JSON answer
const char *create_json(void);

//Create JSON error
const char *error_json(int err);

int get_fw_count_bytes(void);

int get_fw_start_byte(void);

#endif /* JSON_H */
