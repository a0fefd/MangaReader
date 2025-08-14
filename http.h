//
// Created by nbigi0 on 14/08/2025.
//

#ifndef NB_HTTP
#define NB_HTTP

#ifndef NB_SOCKETUTIL
#include "socketutil.h"
#endif

#ifndef NB_HTTP_INCL
#define NB_HTTP_INCL
#include <fcntl.h>
#endif

#define GET 0
#define POST 1

struct HTTPWrapper_st {
    uint8_t method;
    char *host;
    char *page;
    char *payload;
};
typedef struct HTTPWrapper_st HTTPWrapper;

void HTTPHeaderGenerator(const HTTPWrapper info, char *out);
int create_socket(const uint16_t port, const HTTPWrapper header_wrapper);
SSL *create_ssl(SSL_CTX *ctx, const uint32_t socket_file_descriptor, const HTTPWrapper header_wrapper);
SSL_CTX *create_ssl_ctx(void);
int HTTPRequest(SSL_CTX *ssl_context, HTTPWrapper header_wrapper, const uint16_t target_port, char *output_buffer);
HTTPWrapper URLToWrapper(const uint16_t method, const char *url);
int _httpget(SSL_CTX *ssl_context, const char *target_url, const uint16_t target_port, char *output_buffer);

#endif