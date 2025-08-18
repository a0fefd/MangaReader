// External
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

// Local
#include <util.h>
#include <http.h>

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
    sprintf(info_buffer, "Title: \"%s\"\nChapter #: %d\n", manga.title, manga.chapter_count);
}

MangaInfo __httpget_manga(const int (*HTTPSGet)(SSL_CTX *, const char *, char *), SSL_CTX *ctx, const char url[512],
                          char store[MAX_DATA_SIZE])
{
    MangaInfo info = {
        .title = "",
        .chapter_count = 0
    };
    strcpy_s(info.url, strlen(url), url);
    info.url[0] = 'h';

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

MangaInfo get_manga_cp_urls(SSL_CTX *ctx, const char *url, char (*output_buffer)[512][512], char *buf)
{
    if (strlen(url) <= 0)
    {
        return (MangaInfo){.url="",.title= "", .chapter_count = 0};
    }

    char *tmpbuf = malloc(MAX_DATA_SIZE * sizeof(char));
    char *tmpbuf2 = malloc(MAX_DATA_SIZE * sizeof(char));
    memset(buf, '\0', MAX_DATA_SIZE);
    memset(tmpbuf, '\0', MAX_DATA_SIZE);
    memset(tmpbuf2, '\0', MAX_DATA_SIZE);

    MangaInfo manga = get_manga((const int (*)(SSL_CTX *, const char *, char *)) HTTPSGet_ABST, ctx, url, buf);
    // manga_list[i] = get_manga((const int (*)(SSL_CTX*, const char*, char*))NULL, ctx, urls[i], buf);


    
    char *a = strstr(buf, "<a");
    memccpy(tmpbuf, a, '\0', strlen(a));

    strsplice(tmpbuf2, tmpbuf, "f=\"", "\" title");
    char *_chapter_path = strstr(tmpbuf2, "/chap");
    char *chapter_path = malloc(strlen(_chapter_path) * sizeof(char));
    memcpy(chapter_path, _chapter_path, strlen(_chapter_path));
    chapter_path[strlen(_chapter_path)-1] = '\0';

    const char host[] = "https://mangapill.com";
    for (int i = manga.chapter_count - 1; i > -1; i--)
    {
        memset(output_buffer[0][i], 0, 512);

        for (int j = 0; j < strlen(chapter_path); j++)
        {
            if (j < 21)
                output_buffer[0][i][j] = host[j];
            output_buffer[0][i][j+21] = chapter_path[j];
        }

        itoa(i+1, (output_buffer[0][i]+strlen(chapter_path)+21), 10);
    }
    free(chapter_path);

    free(tmpbuf);
    free(tmpbuf2);

    return manga;
}

void main_download(MangaInfo manga_list[MANGACOUNT], const char urls[MANGACOUNT][512])
{
    SSL_CTX *ctx = create_ssl_ctx();
    char *buf = malloc(MAX_DATA_SIZE * sizeof(char));

    char (*manga_cp_urls)[MANGACOUNT][512][512] = malloc(MANGACOUNT * 512 * 512 * sizeof(char));
    memset(manga_cp_urls, '\0', MANGACOUNT * 512 * 512 * sizeof(char));
    for (int i = 0; i < MANGACOUNT; i++)
    {
        const char *url = urls[i];
        manga_list[i] = get_manga_cp_urls(ctx, url, &manga_cp_urls[0][i], buf);
    }

    char (*manga_cp_img_urls)[512][1][512] = malloc(512 * 512 * sizeof(char));
    for (int i = 0; i < MANGACOUNT; i++)
    {
        if (strlen(manga_list[i].title) < 1)
            continue;
        char info[512];
        get_manga_info(manga_list[i], info);
        printf("%s", info);
        for (int j = 0; j < manga_list[i].chapter_count; j++)
        {
            
        }
        printf("\n");
    }

    free(buf);
    free(manga_cp_urls);
    SSL_CTX_free(ctx);
}

int main(void)
{
    SSL_library_init();
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

    char urls[MANGACOUNT][512] = {0};

    FILE *url_list; 
    fopen_s(&url_list, "url_list.txt", "a");
    fputc('\n', url_list); // append newline to file due to failed recognition of link when no newline

    freopen_s(&url_list, "url_list.txt", "r", url_list);
    int indexpointer = 0; int c;
    char line[512] = {0}; int lines = 0;
    while ((c = fgetc(url_list)) != EOF)
    {
        if (c == '\n' && strlen(line) > 20)
        {
            // line[indexpointer] = '\0';
            strcpy_s(urls[lines], sizeof(line), line);
            lines++;
            indexpointer = 0;
            memset(line, '\0', 512);
            continue;
        }
        line[indexpointer] = (char)c;
        indexpointer++;
    }
    fclose(url_list);

    MangaInfo manga_list[MANGACOUNT];

    /// ONLY FOR DOWNLOADING MANGA ///
    main_download(manga_list, urls);
    /// ^ ONLY FOR DOWNLOADING MANGA ^ ///

    // Cleanup
    WSACleanup();

    return 0;
}
