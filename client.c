#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define PORT 9999
#define SERVER_IP "127.0.0.1" // 내 컴퓨터 주소

int main(void) {
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len;

    // 1. socket(): 전화기 구입
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("socket() error\n");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // IP 주소 변환 (문자열 -> 네트워크 숫자)
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(PORT);

    // 2. connect(): 서버에 전화걸기
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) {
        printf("connect() error\n");
        exit(1);
    }
    else {
        printf("Connected to Server! (Type'Q' to quit)\n");
    }

    while (1) {
        fputs("Input message: ", stdout);   // 사용자 입력
        fgets(message, BUF_SIZE, stdin);

        if (!strcmp(message, "q\n")|| !strcmp(message, "Q\n")) // Q 입력시 종료
           break;
        
           write(sock, message, strlen(message));

           str_len = read(sock, message, BUF_SIZE - 1);
           message[str_len] = 0;
           printf("Message from server: %s", message);
    }

    close(sock);
    return 0;
}