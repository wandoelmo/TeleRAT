#include "http_praser.h"

void prase_request(struct http_request *request, char *http_request_str){
    char *http_header = (char *) malloc(strlen(http_request_str));
    char *splited = NULL;
    char temp[256];
    int j = 0, i = 0;
    strcpy(http_header, http_request_str);

    memset(temp, 0, sizeof(temp));
    for(i = 0, j = 0; http_header[i] != ' '; i++, j++){
        temp[j] = http_header[i];
    }
    strcpy(request->method, temp);

    splited = strtok(http_header, "\r\n");
    while(splited != NULL){
        if(strncmp(splited, request->method, strlen(request->method)) == 0){
            //get path
            memset(temp, 0, sizeof(temp));
            for(i = strlen(request->method) + 1, j=0; splited[i] != ' '; i++, j++){
                temp[j] = splited[i];
            }
            strcpy(request->path, temp);

            memset(temp, 0, sizeof(temp));
            for(i += 1, j=0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
            }
            strcpy(request->version, temp);
        }

        if(strncmp(splited, "Host: ", 6) == 0){
            memset(temp, 0, sizeof(temp));
            for(i = 6, j = 0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
            }
            strcpy(request->host, temp);
        }

        if(strncmp(splited, "Content-Type: ", 14) == 0){
            memset(temp, 0, sizeof(temp));
            for(i = 14, j = 0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
            }
            strcpy(request->content_type, temp);
        }

        if(strncmp(splited, "Content-Length: ", 16) == 0){
            memset(temp, 0, sizeof(temp));
            for(i = 16, j = 0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
            }
            request->content_length = atoi(temp);
        }

        splited = strtok(NULL, "\r\n");
    }

    free(http_header);
}

void prase_response(struct http_response *response, char *http_response_str){
    char *http_header = (char *) malloc(strlen(http_response_str));
    char *splited = NULL;
    char temp[256];
    int j = 0, i = 0;
    strcpy(http_header, http_response_str);

    memset(temp, 0, sizeof(temp));
    for(i = 0, j = 0; http_header[i] != ' '; i++, j++){
        temp[j] = http_header[i];
    }
    strcpy(response->version, temp);

    splited = strtok(http_header, "\r\n");
    while(splited != NULL){
        if(strncmp(splited, response->version, strlen(response->version)) == 0){
            memset(temp, 0, sizeof(temp));
            for(i += 1, j=0; splited[i] != ' '; i++, j++){
                temp[j] = splited[i];
            }
            response->status_code = atoi(temp);

            memset(temp, 0, sizeof(temp));
            for(i += 1, j=0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
            }
            strcpy(response->status, temp);
        }

        if(strncmp(splited, "Content-Type: ", 14) == 0){
            int has_charset = 0;
            memset(temp, 0, sizeof(temp));
            for(i = 14, j = 0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
                if(splited[i] == ';'){
                    has_charset = 1;
                    break;
                }
            }
            strcpy(response->content_type, temp);

            if(has_charset){
            memset(temp, 0, sizeof(temp));
                for(i += 1, j = 0; splited[i] != '\0'; i++, j++){
                    if(splited[i] == ' '){
                        continue;
                    }
                    temp[j] = splited[i];
                }
                strcpy(response->charset, temp);
            }
        }

        if(strncmp(splited, "Content-Length: ", 16) == 0){
            memset(temp, 0, sizeof(temp));
            for(i = 16, j = 0; splited[i] != '\0'; i++, j++){
                temp[j] = splited[i];
            }
            response->content_length = atoi(temp);
        }

        splited = strtok(NULL, "\r\n");
    }

    free(http_header);
    printf("debug ==> %s\n", response->content_type);
}