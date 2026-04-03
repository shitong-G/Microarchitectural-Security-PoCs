#define _GNU_SOURCE
/*
 * NetSpectre-style *local* teaching demo (Schwarz et al., USENIX Security 2018).
 * Loopback TCP: server adds secret-dependent work per request; client measures RTT.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 45124
#define SECRET_BIT 1

static uint64_t ns_diff(struct timespec *a, struct timespec *b) {
    return (uint64_t)(b->tv_sec - a->tv_sec) * 1000000000ull +
           (uint64_t)(b->tv_nsec - a->tv_nsec);
}

static void *server_thread(void *arg) {
    (void)arg;
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(srv, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("bind");
        return NULL;
    }
    listen(srv, 1);
    int fd = accept(srv, NULL, NULL);
    if (fd < 0) {
        perror("accept");
        close(srv);
        return NULL;
    }

    char buf[4];
    while (read(fd, buf, 1) == 1) {
        if (SECRET_BIT) {
            volatile uint64_t x = 0;
            for (int i = 0; i < 5000; i++) {
                x += (uint64_t)i;
            }
            (void)x;
        }
        if (write(fd, "x", 1) != 1) {
            break;
        }
    }
    close(fd);
    close(srv);
    return NULL;
}

int main(void) {
    pthread_t th;
    if (pthread_create(&th, NULL, server_thread, NULL) != 0) {
        fprintf(stderr, "pthread_create failed\n");
        return 1;
    }
    usleep(150000);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
        perror("connect");
        return 1;
    }

    const int trials = 2000;
    uint64_t total = 0;
    for (int t = 0; t < trials; t++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        if (write(fd, "q", 1) != 1) {
            break;
        }
        char r;
        if (read(fd, &r, 1) != 1) {
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        total += ns_diff(&t0, &t1);
    }
    close(fd);
    pthread_join(th, NULL);

    printf("NetSpectre local: avg request RTT over %d pings: %.0f ns\n", trials,
           (double)total / (double)trials);
    return 0;
}
