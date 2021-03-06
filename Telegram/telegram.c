#include "telegram.h"

int telegram_init(struct telegram_chat *chat){
    chat->state = 0;
    return 1;
}

int telegram_check(struct telegram_chat *chat){
    return chat->state;
}

int telegram_mark_send(struct telegram_chat *chat){
    chat->state = 0;
    return 1;
}

int telegram_mark_new(struct telegram_chat *chat){
    chat->state = 1;
    return 1;
}

int telegram_get_me(char *destination){
    BIO *request_bio;
    char *header = (char *) malloc(256);
    char *response = (char *) malloc(2048);
    char *response_body = (char *) malloc(2048);

    memset(response_body, 0, sizeof(response_body));
    memset(response, 0, sizeof(response));

    struct http_request request_struct = {
        .method = "GET",
        .path = "/bot"TELEGRAM_TOKEN"/getMe",
        .host = "api.telegram.org"
    };

    create_request(header, &request_bio, &request_struct);
    
    if(write_request(&request_bio, header, strlen(header)) < 0){
        return -1;
    }

    strcpy(destination, response_body);

    free(response_body);
}

int telegram_set_webhook(char *url, char *public_key){
    BIO *request_bio;

    char *response = (char *) malloc(2048);
    char *response_body = (char *) malloc(2048);
    char *buffer = (char *) malloc(20480);
    char *temp = (char *) malloc(20480);

    struct http_request request_struct = {
        .method = "POST",
        .path = "/bot"TELEGRAM_TOKEN"/setWebhook",
        .host = "api.telegram.org",
        .content_type = "multipart/form-data",
    };

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    memset(response_body, 0, sizeof(response_body));

    add_post(buffer, url, "url");
    add_file_post(buffer, public_key, "certificate");
    end_post(&buffer);

    request_struct.content_length = strlen(buffer);

    create_request(temp, &request_bio, &request_struct);
    strcat(temp, buffer);
    strcpy(buffer, temp);

    if(write_request(&request_bio, buffer, strlen(buffer)) < 0){
        return -1;
    }
    get_response(&request_bio, response);
    get_body(response_body, response);

    free(temp);

    printf("\n\n[Response from Telegram]\n\n%s\n\n[End Telegram response]\n\n", response_body);

    return 1;
}

int telegram_send_msg(char *chat_id, char *text){
    BIO *request_bio;

    struct http_request request_struct = {
        .method = "POST",
        .path = "/bot"TELEGRAM_TOKEN"/sendMessage",
        .host = "api.telegram.org",
        .content_type = "multipart/form-data",
    };

    char *header = (char *) malloc(512);
    char *buffer = (char *) malloc(20480);
    char *response = (char *) malloc(20480);
    char *response_body = (char *) malloc(2048);

    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    memset(response_body, 0, sizeof(response_body));

    /* create HTTP body */
    add_post(buffer, chat_id, "chat_id");
    add_post(buffer, text, "text");
    add_post(buffer, "Markdown", "parse_mode");
    end_post(&buffer);

    request_struct.content_length = strlen(buffer);

    /* create HTTP Header message */
    create_request(header, &request_bio, &request_struct);
    if(write_request(&request_bio, header, strlen(header)) < 0)
    {
        return -1;
    }
    write_request(&request_bio, buffer, strlen(buffer));
    // get_response(&request_bio, response);
    // get_body(response_body, response);

    printf("[Telegram] send message to %s: %s\n", chat_id, text);
    // printf("\n\n[Response from Telegram]\n\n%s\n\n[End Telegram response]\n\n", response_body);
    return 1;
}

int telegram_send_act(char *chat_id, char *action){
    BIO *request_bio;

    struct http_request request_struct = {
        .method = "POST",
        .path = "/bot"TELEGRAM_TOKEN"/sendChatAction",
        .host = "api.telegram.org",
        .content_type = "multipart/form-data",
    };

    char *header = (char *) malloc(512);
    char *buffer = (char *) malloc(2048);
    char *response = (char *) malloc(2048);
    char *response_body = (char *) malloc(2048);

    add_post(buffer, chat_id, "chat_id");
    add_post(buffer, action, "action");
    end_post(&buffer);

    request_struct.content_length = strlen(buffer);

    create_request(header, &request_bio, &request_struct);
    if(write_request(&request_bio, header, strlen(header)) < 0){
        return -1;
    }
    if(write_request(&request_bio, buffer, strlen(buffer)) < 0){
        return -1;
    }

    printf("[Telegram] send action '%s' to %s\n", action, chat_id);
    return 1;
}

int telegram_send_file(char *chat_id, char *file_path){
    BIO *request_bio;
    FILE *fp;

    struct http_request request_struct = {
        .method = "POST",
        .path = "/bot"TELEGRAM_TOKEN"/sendDocument",
        .host = "api.telegram.org",
        .content_type = "multipart/form-data",
    };

    char *header = (char *) malloc(512);
    char *buffer = (char *) malloc(2048);
    char *temp = (char *) malloc(512);
    char *response = (char *) malloc(2048);
    char *response_body = (char *) malloc(2048);
    char *read_file = (char *) malloc(2048);

    memset(buffer, 0, sizeof(buffer));
    memset(header, 0, sizeof(header));
    memset(response, 0, sizeof(response));
    memset(response_body, 0, sizeof(response_body));
    memset(read_file, 0, sizeof(read_file));

    add_post(buffer, chat_id, "chat_id");
    create_file_boundary(&temp, file_path, "document");
    strcat(buffer, temp);

    free(temp);

    fp = fopen(file_path, "r");
    if(fp != NULL){
        fseek(fp, 0, SEEK_END);

        request_struct.content_length = 0;
        request_struct.content_length += strlen(buffer);
        request_struct.content_length += ftell(fp);

        bzero(buffer, sizeof(buffer));
        strcat(buffer, "\r\n");
        end_post(&buffer);
        request_struct.content_length += strlen(buffer);

        create_request(header, &request_bio, &request_struct);
        if(write_request(&request_bio, header, strlen(header)) < 0){
            return -1;
        }

        memset(buffer, 0, sizeof(buffer));
        add_post(buffer, chat_id, "chat_id");
        create_file_boundary(&temp, file_path, "document");
        strcat(buffer, temp);

        if(write_request(&request_bio, buffer, strlen(buffer)) < 0){
            return -1;
        }

        rewind(fp);
        while(!feof(fp)){
            memset(read_file, 0, sizeof(read_file));
            int bufflen = fread(read_file, 1, sizeof(read_file), fp);
            read_file[bufflen] = 0;
            if(write_request(&request_bio, read_file, bufflen) < 0){
                return -1;
            }
        }
        fclose(fp);
        bzero(buffer, sizeof(buffer));
        strcat(buffer, "\r\n");
        end_post(&buffer);
        if(write_request(&request_bio, buffer, strlen(buffer)) < 0){
            return -1;
        }
        printf("[Telegram] send file '%s' to %s\n", file_path, chat_id);
        return -1;
    }else{
        printf("[Telegram send file] file \"%s\" not found\n", file_path);
        return 1;
    }
}