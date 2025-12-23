#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h> //  스레드 사용을 위한 헤더

#define PORT 9999
#define BUF_SIZE 1024

// 스레드가 실행할 함수 선언
void *handle_client(void *arg);

int main(void) {
    int server_sock;
    int client_sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;
    pthread_t t_id; // 스레드 ID (직원 명찰)

    // 1. 소켓 생성
    server_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // 2. bind
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("Bind Error\n");
        exit(1);
    }

    // 3. listen
    if (listen(server_sock, 5) == -1) {
        printf("Listen Error\n");
        exit(1);
    }

    printf("Multi-Threaded Server Started on port %d...\n",PORT);

    client_addr_size = sizeof(client_addr);

    while(1) {
        // 4. accept
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_sock == -1) {
            continue;
        }

        printf("New Client Connected! (Sock: %d)\n", client_sock);

        /* [변경 핵심] fork() 대신 스레드 생성
        1. &t_id: 스레드 ID를 저장할 변수 주소
        2. NULL: 기본 설정
        3. handle_client: 스레드가 수행할 함수
        4. (void*)&client_sock: 함수에 전달할 인자 
        주소를 바로 넘기면 동시 접속 시 덮어써질 위험 => 실무에서는 malloc으로 메모리 따로 할당 */
        
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        pthread_create(&t_id, NULL, handle_client, (void*)new_sock);

        pthread_detach(t_id);
        // 스레드가 일이 끝나면 알아서 메모리 소멸( 안하면 좀비 프로세스/ 메모리 누수 )
    }
    
    close(server_sock);
    return 0;
}

// [직원 업무 메뉴얼] 스레드가 실행하는 함수
void *handle_client(void *arg) {
    int clnt_sock = *((int*)arg);  // 전달받은 소켓 번호 꺼내기
    free(arg);                     // 할당받은 메모리 해제

    int str_len = 0;
    char message[BUF_SIZE];

    while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0) {
        write(clnt_sock, message, str_len);

        message[str_len] = 0;                    // 서버 화면 출력
        printf("[Thread] Message: %s", message); // printf는 스레드에서 출력이 섞일 위험 but 테스트용
    }

    close(clnt_sock);
    printf("Client Disconnected.\n");
    return NULL;
}