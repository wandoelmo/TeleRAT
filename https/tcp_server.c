#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

// openSSL
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/bio.h"

#define BUFFER_SIZE 20480

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void init_openssl(){
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
    SSL_library_init();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

/* Config SSL context */
void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

/* Get request and response HTTPS */
int response(int client_socket, FILE *file, SSL **ssl){
    unsigned int bufflen, readlen;
    char buffer[BUFFER_SIZE], readBuffer[BUFFER_SIZE];

    /* Buffer IO Init*/
    BIO *out;
    BIO *ssl_bio;
    ssl_bio = BIO_new(BIO_f_ssl());
    out = BIO_new(BIO_s_connect());

    BIO_set_ssl(ssl_bio, *ssl, BIO_CLOSE);
    BIO_set_nbio(out, 1);

    out = BIO_push(ssl_bio, out);

    readlen = BIO_read(ssl_bio, readBuffer, sizeof(readBuffer));
    if(readlen > 0){
        printf("[HTTP Request] | readlen = %d\n", readlen);
        printf("%s\n\n", readBuffer);
        printf("=== End Request ===\n");
    }
    else{
        return -1;
    }
    bzero(readBuffer, sizeof(readBuffer));

    // HTTP 1.1 Header
    char http_header[] = 
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";

    if(file){
        unsigned int content_length = 0;

        // Content-Length
        while((bufflen = fread(buffer, 1, BUFFER_SIZE, file)) > 0){
            content_length += bufflen;
        }

        char header_content_length[1024];
        sprintf(header_content_length, "Content-Length: %d\r\n\r\n", content_length);

        strcpy(buffer, http_header);
        strcat(buffer, header_content_length);

        printf("[HTTP Response]\n %s\n", buffer);
        BIO_write(out, &buffer, strlen(buffer));
        rewind(file);

        // read file and sent response
        while((bufflen = fread(buffer, 1, BUFFER_SIZE, file)) > 0){
            BIO_write(out, &buffer, bufflen);
        }

        printf("==== End Response ===\n");

        return 1;
    }
}

int main(){
    SSL *ssl;
    SSL_CTX *ctx;
    init_openssl();
    ctx = create_context();
    configure_context(ctx);

    FILE *fp;

    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8443);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if(listen(server_socket, 5) < 0){
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    while(1){
        struct sockaddr_in client_address;
        unsigned int len = sizeof(client_address);

        int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &len);
        if (client_socket < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        fp = fopen("htdocs/index.html", "r");

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
            response(client_socket, fp, &ssl);
        }
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }

    SSL_CTX_free(ctx);
    close(server_socket);
    cleanup_openssl();
}