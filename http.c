//
// Created by nbigi0 on 14/08/2025.
//

#include <http.h>

void HTTPHeaderGenerator(const HTTPWrapper info, char *out) {
    sprintf(out, "%s %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n"
                     "%s"
                     "\r\n", info.method ? "POST" : "GET", info.page, info.host, info.payload ? info.payload : "");
}

int create_socket(const uint16_t port, const HTTPWrapper header_wrapper)
{
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    char strport[5]; itoa(port, strport, 10);
    int getaddrinfo_ret = getaddrinfo(header_wrapper.host, strport, &hints, &res);
    if (getaddrinfo_ret != 0)
    {
        puts(gai_strerror(getaddrinfo_ret));
        perror("getaddrinfo()");
        exit(EXIT_FAILURE);
    }
    SOCKET sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int connect_ret = connect(sockfd,res->ai_addr,(int)res->ai_addrlen);
    if (connect_ret < 0)
    {
        perror("connect(socket)");
        exit(1);
    }

    return (int)sockfd;
}

SSL *create_ssl(SSL_CTX *ctx, const uint32_t socket_file_descriptor, const HTTPWrapper header_wrapper)
{
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, (int32_t)socket_file_descriptor);
    char *host = header_wrapper.host;
    // strcat(host, header_wrapper.page);
    SSL_set_tlsext_host_name(ssl, host);
    SSL_connect(ssl);

    return ssl;
}

SSL_CTX *create_ssl_ctx(void)
{
    const SSL_METHOD* client_method = SSLv23_client_method();
    if (client_method == NULL)
    {
        fprintf(stderr, "SSLv23_client_method() failed\n");
        exit(1);
    }
    SSL_CTX *ctx = SSL_CTX_new(client_method);
    if (ctx == NULL)
    {
        fprintf(stderr, "SSL_CTX_new() failed\n");
        exit(1);
    }
    return ctx;
}

int HTTPRequest(SSL_CTX *ssl_context, HTTPWrapper header_wrapper, const uint16_t target_port, char *output_buffer)
{
    if (!(header_wrapper.host) || strlen(header_wrapper.host) < 1)
        return -3;
    const uint64_t socket_file_descriptor = create_socket(target_port, header_wrapper);

    char header_string[1024];
    HTTPHeaderGenerator(header_wrapper, header_string);

    SSL *ssl = create_ssl(ssl_context, socket_file_descriptor, header_wrapper);
    free(header_wrapper.host);
    // free(header_wrapper.page);

    SSL_write(ssl, header_string, (int)strlen(header_string));

    char tempbuf[256] = {0}; char tempstore[MAX_DATA_SIZE] = {0};
    signed ossl_ssize_t bytes, culm_bytes = 0;
    while ((bytes = SSL_read(ssl, tempbuf, sizeof(tempbuf)-1)) > 0)
    {
        tempbuf[bytes] = '\0';
        // puts(tempbuf);
        memccpy(tempstore+culm_bytes, tempbuf, '\0', bytes+1);
        culm_bytes += bytes;
    }

    memccpy(output_buffer, tempstore, '\0', culm_bytes);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close((int)socket_file_descriptor);

    if (tempstore[0] == 0 || tempstore[0] == '\0')
    {
        return -3;
    }

    if (strcmp(output_buffer, "HTTP/1.1 200") < 0)
    {
        return -1;
    }
    return 0;


}

HTTPWrapper URLToWrapper(const uint16_t method, const char *url)
{
    if (!url || strlen(url) < 1)
        return (HTTPWrapper){0, 0 ,0, 0};

    char new_url[1024];
    memcpy(new_url, url, strlen(url));

    uint8_t url_offset = 0;
    if (strcmp(new_url, "http") > 0)
    {
        switch (new_url[4])
        {
        case 's' :
            url_offset = 8;
            break;
        default :
            url_offset = 7;
            break;
        }
    }

    memset(new_url, '\0', strlen(url)+1);
    memcpy(new_url, url + url_offset*sizeof(char), strlen(url));

    HTTPWrapper header_wrapper = {
        .method = method,
        .host = malloc(strlen(url) + 1),
        .page = NULL
    };

    char *token = strtok(new_url, "/");
    if (token)
    {
        memccpy(header_wrapper.host, new_url, '\0', sizeof(new_url));
        token = strtok(NULL, "/");
        if (token)
        {
            memccpy(new_url, url + url_offset*sizeof(char) + sizeof(header_wrapper.host), '\0', sizeof(url)-(url_offset*sizeof(char) + sizeof(header_wrapper.host)));
            token = strtok(new_url, "\0");
            header_wrapper.page = strchr(token, '/');
        } else
        {
            header_wrapper.page = "/";
        }
    }
    return header_wrapper;
}

int _httpget(SSL_CTX *ssl_context, const char *target_url, const uint16_t target_port, char *output_buffer)
{
    HTTPWrapper header_wrapper = URLToWrapper(GET, target_url);
    return HTTPRequest(ssl_context, header_wrapper, target_port, output_buffer);
}
