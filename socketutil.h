//
// Created by nbigi0 on 14/08/2025.
//

#ifndef NB_SOCKETUTIL
#define NB_SOCKETUTIL

#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <signal.h>

#ifndef NB_UTIL
#include "util.h"
#endif

void *get_in_addr(struct sockaddr *sa);

#endif