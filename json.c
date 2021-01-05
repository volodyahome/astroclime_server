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

// "{\"resp\":\"pong\",\"data\":[1,2,3]}";
//  {"resp":"random","data":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187]}

// {"conn":1,"mem_used":7.4707794189453125}
// {"fwsize":94567,"fwmd5":"1843b1367728caf598b0499872ba9a12","dt":"050320111748"}
// {"buff":"0a2020226e616d65223a"}
const char *answer_json(int answer) {
    
    const char *response;
    
    json_object *jobj = json_object_new_object();
    json_object *jarray = json_object_new_array();

    json_object *resp;
    json_object *conn;
    json_object *mem_used;
    json_object *fwsize;
    json_object *fwmd5;
    json_object *dt;
    
    
    switch (answer) {
        case PING:
            resp = json_object_new_string("pong");

            json_object *int1 = json_object_new_int(1);
            json_object *int2 = json_object_new_int(2);
            json_object *int3 = json_object_new_int(3);

            json_object_array_add(jarray, int1);
            json_object_array_add(jarray, int2);
            json_object_array_add(jarray, int3);

            json_object_object_add(jobj, "resp", resp);
            json_object_object_add(jobj, "data", jarray);
            break;
        case RANDOM:
            
            break;
        case CLOSE:
            resp = json_object_new_string("bay");
            json_object_object_add(jobj, "resp", resp);
            break;
        case STAT:
            conn = json_object_new_string("%d");
            mem_used = json_object_new_string("%ld");
            
            json_object_object_add(jobj, "conn", conn);
            json_object_object_add(jobj, "mem_used", mem_used);
            break;
        case FWINFO:
            fwsize = json_object_new_string("%llu");
            fwmd5 = json_object_new_string("%s");
            dt = json_object_new_string("%s");
            
            json_object_object_add(jobj, "fwsize", fwsize);
            json_object_object_add(jobj, "fwmd5", fwmd5);
            json_object_object_add(jobj, "dt", dt);
            break;
        case FWGET:
            
            break;
        case ANALYTICS:
            
            break;
            
    }
    
    response = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PLAIN);
        
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
            
    return response;
}

int get_fw_count_bytes(void) {
    return fw_count_bytes;
}

int get_fw_start_byte(void) {
    return fw_start_byte;
}
