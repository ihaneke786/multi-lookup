#include "util.h"
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int dnslookup(const char* hostname, char* firstIPstr, int maxSize) {

    if (!hostname || !firstIPstr) {
        return UTIL_FAILURE;
    }

    struct addrinfo hints;
    struct addrinfo* results = NULL;
    int addrError;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         // IPv4 only
    hints.ai_socktype = SOCK_STREAM;   // Avoid duplicate UDP entries
    hints.ai_flags = AI_ADDRCONFIG;    // Only return configured families

    addrError = getaddrinfo(hostname, NULL, &hints, &results);

    if (addrError == 0 && results != NULL) {

        struct sockaddr_in* addr =
            (struct sockaddr_in*) results->ai_addr;

        if (inet_ntop(AF_INET,
                      &addr->sin_addr,
                      firstIPstr,
                      maxSize) != NULL) {

            freeaddrinfo(results);
            return UTIL_SUCCESS;
        }

        freeaddrinfo(results);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;

    results = NULL;
    addrError = getaddrinfo(hostname, NULL, &hints, &results);

    if (addrError == 0 && results != NULL) {

        struct sockaddr_in6* addr6 =
            (struct sockaddr_in6*) results->ai_addr;

        if (inet_ntop(AF_INET6,
                      &addr6->sin6_addr,
                      firstIPstr,
                      maxSize) != NULL) {

            freeaddrinfo(results);
            return UTIL_SUCCESS;
        }

        freeaddrinfo(results);
    }

    return UTIL_FAILURE;
}
