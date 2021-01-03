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

#include "json.h"

//Received bytes
int fw_count_bytes = 0;

//Byte to start reading
int fw_start_byte = 0;

int parse_json(char * buff_recv) {
    
    struct json_object *parsed_json;
    struct json_object *cmd = NULL;
    
    char response[9] = {0};
    
    int result = 7;
        
    parsed_json = json_tokener_parse(buff_recv);
    
    if(parsed_json != NULL) {
        json_object_object_get_ex(parsed_json, "cmd", &cmd);
        
        sprintf(response, "%s", json_object_get_string(cmd));
        
        slog_print(SLOG_INFO, 1, buff_recv);
        
        if (strcmp("ping", response) == 0) {
            result = 0;
        }
        if (strcmp("random", response) == 0) {
            result = 1;
        }
        if (strcmp("close", response) == 0) {
            result = 2;
        }
        if (strcmp("stat", response) == 0) {
            result = 3;
        }
        if (strcmp("fwinfo", response) == 0) {
            result = 4;
        }
        if (strcmp("fwget", response) == 0) {
            struct json_object *count; //Received bytes
            struct json_object *start; //Byte to start reading
            
            json_object_object_get_ex(parsed_json, "count", &count);
            fw_count_bytes = json_object_get_int(count);
            
            json_object_object_get_ex(parsed_json, "start", &start);
            fw_start_byte = json_object_get_int(start);
            
            free(count);
            free(start);
            
            result = 5;
        }
        if (strcmp("analytics", response) == 0) {
            result = 6;
        }
    } else {
        free(cmd);
        slog_print(SLOG_ERROR, 1, "{\"errco\":1,\"errdesc\":\"invalid command\"}");
    }
    
    free(parsed_json);
    free(cmd);
    
    return result;
}

int get_fw_count_bytes(void) {
    return fw_count_bytes;
}

int get_fw_start_byte(void) {
    return fw_start_byte;
}
