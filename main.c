// External
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

// Local
#include "util.h"
#include "http.h"

#define PORT_HTTP 80
#define PORT_HTTPS 443

#define MANGACOUNT 32

struct MangaInfo_st
{
    char url[512];
    char title[512];
    uint16_t chapter_count;
};

typedef struct MangaInfo_st MangaInfo;

void get_manga_info(MangaInfo manga, char *info_buffer)
{
    sprintf(info_buffer, "Title: %s\nChapter #: %d\n", manga.title, manga.chapter_count);
}

MangaInfo __httpget_manga(const int (*HTTPSGet)(SSL_CTX *, const char *, char *), SSL_CTX *ctx, const char url[512],
                          char store[MAX_DATA_SIZE])
{
    MangaInfo info = {
        .url = "",
        .title = "",
        .chapter_count = 0
    };
    strcpy_s(info.url, strlen(url), url);

    char *buf = store;
    int success = HTTPSGet(ctx, info.url, buf);
    if (success == -1)
    {
        fprintf(stderr, "No OK Response\n");
    } else
    {
        if (success == -2)
            fprintf(stderr, "Failed connection -> using saved data\n");
        if (success == -3)
            return (MangaInfo){"", "", 0};

        char *title = info.title;
        char /*title[512],*/ temp[512];
        strsplice(title, buf, "<title>", " Manga - Mangapill");
        memset(title, '\0', 7);
        _memccpy(temp, title + 7, '\0', sizeof temp);
        memset(title, '\0', sizeof info.title);
        memcpy(title, temp, sizeof temp - sizeof(char));

        strsplice_single(buf, "<div id=\"chapter", "</div>");
        info.chapter_count = strtok_counter(buf, "href=");
    }

    return info;
}

MangaInfo __fget_manga(FILE *file, char store[MAX_DATA_SIZE])
{
    MangaInfo info = {
        .url = "",
        .title = "",
        .chapter_count = 0
    };

    char *buf = store;

    read_until_eof(file, NULL, buf);

    char *title = info.title;
    char /*title[512],*/ temp[512];
    strsplice(title, buf, "<title>", " Manga - Mangapill");
    memset(title, '\0', 7);
    _memccpy(temp, title + 7, '\0', sizeof temp);
    memset(title, '\0', sizeof info.title);
    memcpy(title, temp, sizeof temp - sizeof(char));

    strsplice_single(buf, "<div id=\"chapter", "</div>");
    info.chapter_count = strtok_counter(buf, "href=");

    return info;
}

MangaInfo get_manga(const int (*HTTPSGet)(SSL_CTX *, const char *, char *), SSL_CTX *ctx, const char data[512],
                    char store[MAX_DATA_SIZE])
{
    MangaInfo info = {0};
    // char store[MAX_DATA_SIZE] = {0};

    if (!HTTPSGet)
    {
        FILE *file = fopen(data, "r+");
        info = __fget_manga(file, store);
    } else
        info = __httpget_manga(HTTPSGet, ctx, data, store);

    if (strlen(info.title) > 0)
    {
        for (size_t i = 0; i < strlen(info.title); i++)
        {
            char c = info.title[i];
            if (c == '&')
            {
                char nc = 0;
                switch (info.title[i+1])
                {
                    case '#':
                    {
                        char str_code[2] = {info.title[i+2], info.title[i+3]};
                        int code = atoi(str_code);
                        switch (code)
                        {
                            case 39:
                            {
                                nc = '\'';
                                break;
                            }

                            default:
                                {break;}
                        }
                        break;
                    }

                    default:
                    {
                        char n3c[3] = {info.title[i+1], info.title[i+2], info.title[i+3]};
                        if (strcmp(n3c, "amp") >= 0)
                        {
                            nc = '&';
                        }
                        break;
                    }
                }
                char replaceval[4] = {0};
                replaceval[0] = nc;
                memcpy(info.title+i, replaceval, sizeof replaceval);
                memcpy(info.title+i+1, info.title+i+5, sizeof (info.title) - i-5);
            }
        }
    }

    return info;
}

int HTTPSGet_ABST(SSL_CTX *ctx, const char *url, char *buf)
{
    return _httpget(ctx, url, PORT_HTTPS, buf);
}

MangaInfo get_manga_cp_urls(SSL_CTX *ctx, const char url[512], char **output_buffer, char *buf)
{
    if (strlen(url) <= 0)
    {
        return (MangaInfo){.url="",.title= "", .chapter_count = 0};
    }

    char *tmpbuf = malloc(MAX_DATA_SIZE * sizeof(char));
    memset(buf, '\0', MAX_DATA_SIZE);
    memset(tmpbuf, '\0', MAX_DATA_SIZE);

    MangaInfo manga = get_manga((const int (*)(SSL_CTX *, const char *, char *)) HTTPSGet_ABST, ctx, url, buf);
    // manga_list[i] = get_manga((const int (*)(SSL_CTX*, const char*, char*))NULL, ctx, urls[i], buf);

    strsplice(tmpbuf, buf, "<div id=\"chapter", "</div>");
    
    char *a = strstr(tmpbuf, "<a");
    strcpy_s(tmpbuf, strlen(a), a);

    for (int i = manga.chapter_count - 1; i > -1; i--)
    {
        char *next = strstr(tmpbuf + 1, "<a");
        strsplice_single(tmpbuf, "f=\"", "\" title");
        char *chapter_path = strstr(tmpbuf, "/");

        strcpy_s(output_buffer[i], 22, "https://mangapill.com");
        size_t len = strlen(chapter_path);
        strcat_s(output_buffer[i], len, chapter_path);
        strcpy_s(tmpbuf, sizeof(next), next);
    }
    free(tmpbuf);

    return manga;
}

int main(void)
{
    SSL_library_init();

    SSL_CTX *ctx = create_ssl_ctx();

    char *buf = malloc(MAX_DATA_SIZE * sizeof(char));

#ifdef WIN32
    WSADATA wsa_data;
    if (WSAStartup((MAKEWORD(2, 2)), &wsa_data) != 0)
    {
        fprintf(stderr, "WSAStartup() failed\n");
        exit(1);
    }
    if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
    {
        fprintf(stderr, "Version 2.2 of Winsock not available.\n");
        WSACleanup();
        exit(2);
    }
#endif

#define HTTPGet(url, output_buffer) _httpget(ctx, url, PORT_HTTP, output_buffer)

    char urls[MANGACOUNT][512] = {0};

    FILE *url_list;
    fopen_s(&url_list, "url_list.txt", "r");
    int indexpointer = 0; int c;
    char line[512] = {0}; int lines = 0;
    while ((c = fgetc(url_list)) != EOF)
    {
        if (c == '\n' || c+1 == EOF)
        {
            line[indexpointer] = '\0';
            strcpy_s(urls[lines], sizeof(line), line);
            lines++;
            indexpointer = 0;
            memset(line, '\0', 512);
            continue;
        }
        line[indexpointer] = (char)c;
        indexpointer++;
    }

    MangaInfo manga_list[MANGACOUNT];
    
    // for (int i = 0; i < MANGACOUNT; i++)
    // {
    //     memset(buf, '\0', MAX_DATA_SIZE * sizeof(char));
    //     manga_list[i] = get_manga((const int (*)(SSL_CTX *, const char *, char *)) HTTPSGet_ABST, ctx, urls[i], buf);
    //     // manga_list[i] = get_manga((const int (*)(SSL_CTX*, const char*, char*))NULL, ctx, urls[i], buf);
    // }

    char (*manga_cp_urls)[MANGACOUNT][512][512] = malloc(MANGACOUNT * 512 * 512 * sizeof(char));
    memset(manga_cp_urls, 0, MANGACOUNT * 512 * 512 * sizeof(char));
    for (int i = 0; i < MANGACOUNT; i++)
    {
        char **temp_buffer = malloc(512 * sizeof(char*));  // Array of 512 char* pointers
        for (int j = 0; j < 512; j++) {
            temp_buffer[j] = malloc(512 * sizeof(char));   // Each string has space for 512 characters
            memset(temp_buffer[j], '\0', 512);
        }
        manga_list[i] = get_manga_cp_urls(ctx, urls[i], temp_buffer, buf);
        _memccpy(manga_cp_urls[0][i], temp_buffer, '\0', 512 * 512);
        for (int j = 0; j < 512; j++) {
            free(temp_buffer[j]);
        }
        free(temp_buffer);
    }

    for (int i = 0; i < MANGACOUNT; i++)
    {
        if (strlen(manga_list[i].title) < 1)
            continue;
        char info[512];
        get_manga_info(manga_list[i], info);
        printf("%s", info);
        for (int j = 0; j < manga_list[i].chapter_count; j++)
        {
            printf("%s\n", manga_cp_urls[0][i][j]);
        }
    }
    free(manga_cp_urls);

    // Cleanup
    free(buf);
    SSL_CTX_free(ctx);
    WSACleanup();

    return 0;
}
