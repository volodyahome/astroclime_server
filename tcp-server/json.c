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
#include <string.h>

#include "json.h"
#include "logger.h"

int parse_json(char * buff_recv) {
    struct json_object *obj;
    struct json_object *cmd = NULL;
    
    char response[9];
    
    int result = 7;
    
    obj = json_tokener_parse(buff_recv);
    
    if(obj != NULL) {
        
        json_object_object_get_ex(obj, "cmd", &cmd);
        
        sprintf(response, "%s", json_object_get_string(cmd));
        
        slog(response);
        
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
            result = 5;
        }
        if (strcmp("analytics", response) == 0) {
            result = 6;
        }
    } else {
        free(cmd);
        
        slog("{\"errco\":1,\"errdesc\":\"invalid command\"}");
    }
    
    free(obj);
    free(cmd);
    
    return result;
}
