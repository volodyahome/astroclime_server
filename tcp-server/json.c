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
    }
    
    free(parsed_json);
    free(cmd);
    
    return result;
}

const char *create_json() {
    
    const char *response;
        
    json_object * jobj = json_object_new_object();
    
    json_object *resp = json_object_new_string("bay");
    
    json_object_object_add(jobj,"resp", resp);
            
    response = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
    
    json_object_put(jobj);
        
    return response;
}

const char *error_json(int err) {
    
    const char *response;
    
    json_object *jobj = json_object_new_object();
    
    json_object *errco;
    json_object *errdesc;
    
    switch (err) {
        case INVALID_COMMAND:
            errco = json_object_new_int(INVALID_COMMAND);
            errdesc = json_object_new_string("invalid command");
            break;
        case NO_SUCH_FILE:
            errco = json_object_new_int(NO_SUCH_FILE);
            errdesc = json_object_new_string("no such file");
            break;
        case NOT_READ_FILE:
            errco = json_object_new_int(NOT_READ_FILE);
            errdesc = json_object_new_string("not read file");
            break;
        case EMPTY_REQUEST:
            errco = json_object_new_int(EMPTY_REQUEST);
            errdesc = json_object_new_string("empty request");
            break;
        case COMMAND_NOT_FOUND:
            errco = json_object_new_int(COMMAND_NOT_FOUND);
            errdesc = json_object_new_string("command not found");
            break;
        default:
            errco = json_object_new_int(DEFAULT_ERROR);
            errdesc = json_object_new_string("unknown error");
    }
    
    json_object_object_add(jobj, "errco", errco);
    json_object_object_add(jobj, "errdesc", errdesc);
    
    response = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
    
    json_object_put(jobj);
        
    return response;
}

int get_fw_count_bytes(void) {
    return fw_count_bytes;
}

int get_fw_start_byte(void) {
    return fw_start_byte;
}
