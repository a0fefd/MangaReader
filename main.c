#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>

#include "util.h"
#include "socketutil.h"
#include "http.h"

#define pow2n(n) (1 << n)

#define PORT 443

int main(void)
{
    SSL_library_init();

    SSL_CTX *ctx = create_ssl_ctx();

    char *buf = malloc(MAX_DATA_SIZE * sizeof(char));

#ifdef WIN32
    WSADATA wsa_data;
    if (WSAStartup((MAKEWORD(2,2)), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup() failed\n");
        exit(1);
    }
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2) {
        fprintf(stderr,"Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(2);
    }
#endif

    #define HTTPSGet(url, output_buffer) _httpget(ctx, url, PORT, output_buffer)

    char url[2048] = { '\000' };
    // fgets(url, 2048, stdin);
    // fscanf(stdin, "%s", url);
    strcpy(url, "https://www.mangapill.com/manga/7948/shin-no-seijo-de-aru-watashi-wa-tsuihou-saremashita-dakara-kono-kuni-wa-mou-owari-desu");

    char *pos;
    if ((pos = strstr(url, "\n")))
    {
        const uint32_t index = pos - url;
        memset(url+index*sizeof(char), '\000', sizeof(url)-(index-1)*sizeof(char));
    }

    int success = HTTPSGet(url, buf);
    FILE *offline_data = fopen("data.save", "wb+");
    if (offline_data && success)
    {
        fputs(buf, offline_data);
    }
    if (offline_data && success == -1)
    {
        offline_data = freopen("data.save", "rb", offline_data);
        read_until_eof(offline_data, NULL, buf);
        success = -2;
    }

    fclose(offline_data);

    if (success == -1)
    {
        fprintf(stderr,"No OK Response\n");
    } else
    {
        if (success == -2)
            fprintf(stderr,"Failed connection -> using saved data\n");

        char title[512], temp[512];
        strsplice(title, buf, "<title>"," Manga - Mangapill");
        memset(title, '\0', 7);
        memccpy(temp, title + 7, '\0', sizeof temp);
        memset(title, '\0', sizeof(title));
        memcpy(title, temp, sizeof temp - sizeof(char));

        strsplice_single(buf, "<div id=\"chapter", "</div>");
        int chaptercount = strtok_counter(buf, "href=");

        printf("Manga\nName: %s \nEff Chapter Count: %d", title, chaptercount);
    }

    // Cleanup
    free(buf);
    SSL_CTX_free(ctx);
    WSACleanup();

    return 0;
}
