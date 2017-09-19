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

/* this tiny web server just serves a file called */
/* 'index.html' which has to be in the same directory */
/* as this program's executable, on specified port. */
/* This could probably have been done better. */


#define OUT_FILE "index.html"
#define MAX_CON 64

const char *response =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n";

void help(void) {
    puts("usage: [Port No.]");
    exit(1);
}

char *buf;
FILE *f;

void T800(int i) {
    free(buf);
    fclose(f);
    printf("\nExiting... [%d]\n", i);
    exit(1);
}

int main(int argc, char *argv[]) {
    
    unsigned int port, sz_sockaddr;
    int hsock, csock, err;
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
        puts("Error: file not found.");
        exit(1);
    }

    fstat(fileno(f), &st);
    buf = malloc(st.st_size + 1);
    if (!buf) {
        puts("R.I.P");
        exit(1);
    }
    
    fread(buf, st.st_size, 1, f);

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if (hsock < 0) {
        puts("Error: Could not create socket.");
        exit(1);
    }

    memset(&server, 0, sizeof server);
    memset(&client, 0, sizeof client);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    err = bind(hsock, (struct sockaddr *)&server, sizeof server);
    if (err < 0) {
        puts("Error: Bind failed.");
        exit(1);
    }

    err = listen(hsock, MAX_CON);
    if (err < 0) {
        puts("Error: Listen failed.");
        exit(1);
    }

    sz_sockaddr = sizeof (struct sockaddr_in);
A:
    csock = accept(hsock, (struct sockaddr *)&client,
                   (socklen_t *)&sz_sockaddr);
    if (csock < 0) {
        puts("Error: Accept failed.");
        exit(1);
    }

    write(csock, response, strlen(response));
    write(csock, buf, strlen(buf));
    write(csock, "\r\n", strlen("\r\n"));

goto A;

    free(buf);
    fclose(f);
    return 0;
}
