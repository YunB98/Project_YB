#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>  //
#include <sys/wait.h>

#define PORT 9999 //  (내선 번호)
#define BUF_SIZE 1024 // (메시지 버퍼 크기)

int main(void) {
    int server_sock;
    int client_sock;
    pid_t pid;

    struct sockaddr_in server_addr; // 서버 주소 정보
    struct sockaddr_in client_addr; // 접속한 클라이언트 주소 정보
    socklen_t client_addr_size;

    char message[BUF_SIZE];
    int str_len;

    // 1. 전화기 구입
    signal(SIGCHLD, SIG_IGN);
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("Socket Error\n");
        exit(1);
    }

    // 주소 정보 초기화 (구조체 비우기)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                 // IPv4 사용
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 내 컴퓨터의 모든 IP 허용
    server_addr.sin_port = htons(PORT);               // 포트 번호 설정 (9999)

    // 2. bind(): 전화번호 부여 (소켓에 IP와 PORT 할당)
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))== -1) {
        printf("Bind Error\n");
        exit(1);
    }

    // 3. listen(): 연결 대기 (5는 대기열, 동시에 5명까지 대기가능)
    if (listen(server_sock, 5) == -1) {
        printf("Listen Error\n");
        exit(1);
    }
    
    printf("Server is waiting on port %d...\n", PORT);

    client_addr_size = sizeof(client_addr);

    // 4. accept(): 연결 수락, 연결 요청이 올 때까지 여기서 대기!
    // [수정] while 문 전체를 이걸로 바꾸세요
    while(1) {
        // 4. accept(): 연결 수락
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_sock == -1) {
            continue; // 에러 나면 다시 대기 (서버 안 꺼짐)
        }
        
        printf("New Client Connected!\n");

        // !!!! 여기서 분신술(fork) 사용 !!!!
        pid = fork(); 

        if (pid == -1) { // 분신술 실패 시
            close(client_sock);
            continue;
        }

        // [자식 프로세스 구역] : 실제 손님 담당 (pid가 0)
        if (pid == 0) {
            close(server_sock); // 자식은 '가게 전화기(listen)' 필요 없음
            
            // 대화 루프
            while ((str_len = read(client_sock, message, BUF_SIZE)) != 0) {
                // 1. 클라이언트에게 다시 보내기 (Echo)
                write(client_sock, message, str_len);
                
                // 2. 서버 화면에 출력 (누가 처리하는지 pid 출력)
                message[str_len] = 0;
                printf("[Child %d] Message: %s", getpid(), message);
            }
            
            close(client_sock); // 대화 끝나면 전화 끊기
            printf("Client Disconnected (Child %d finished).\n", getpid());
            exit(0); // 자식 프로세스 소멸 (매우 중요!)
        }
        
        // [부모 프로세스 구역] : 카운터 담당
        else {
            close(client_sock); // 부모는 '손님 전화기' 필요 없음 (자식 줬으니까)
            // 부모는 즉시 루프 위로 올라가서 다른 손님을 기다림 (accept)
        }
    }
    // 6. close(): 전화 끊기
    close(server_sock);
    return 0;
}