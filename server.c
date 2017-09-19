#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

/* this tiny web server just serves a file called */
/* 'index.html' which has to be in the same directory */
/* as this program's executable, on specified port. */
/* This could probably have been done better. */


#define OUT_FILE "index.html"
#define MAX_CON 64

#define ERROR_DIE(c) do {            \
                        perror(c);   \
                        exit(errno); \
                     } while(0)

const char *response =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n";

volatile int run = 1;

void T800(int);
void help(void);

int main(int argc, char *argv[]) {
    
    unsigned int port, sz_sockaddr;
    int hsock, csock, err;
    char *buf;
    FILE *f;
    struct stat st;
    struct sockaddr_in server, client;

    signal(SIGINT, &T800);
    signal(SIGTSTP, &T800);

    if (argc < 2) {
        help();
    }

    port = atoi(argv[1]);
    if (!port) {
        help();
    }

    f = fopen(OUT_FILE, "rb");
    if (!f) {
        ERROR_DIE("File not found.");
    }

    fstat(fileno(f), &st);
    buf = malloc(st.st_size + 1);
    if (!buf) {
        ERROR_DIE("R.I.P");
    }
    
    fread(buf, st.st_size, 1, f);

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if (hsock < 0) {
        ERROR_DIE("Error: Could not create socket.");
    }

    memset(&server, 0, sizeof server);
    memset(&client, 0, sizeof client);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    err = bind(hsock, (struct sockaddr *)&server, sizeof server);
    if (err < 0) {
        ERROR_DIE("Bind failed.");
    }

    err = listen(hsock, MAX_CON);
    if (err < 0) {
        ERROR_DIE("Listen failed.");
    }

    sz_sockaddr = sizeof (struct sockaddr_in);

    while(run) {
        csock = accept(hsock, (struct sockaddr *)&client, (socklen_t *)&sz_sockaddr);
        if (csock < 0 && run) {
            ERROR_DIE("Accept failed.");
        }

        write(csock, response, strlen(response));
        write(csock, buf, strlen(buf));
        write(csock, "\r\n", strlen("\r\n"));

        close(csock);
    }

    free(buf);
    fclose(f);
    return 0;
}

void T800(int i) {
    run = 0;
    printf("\nExiting.. (%d)\n", i);
}

void help(void) {
    puts("usage: [Port No.]");
    exit(1);
}

