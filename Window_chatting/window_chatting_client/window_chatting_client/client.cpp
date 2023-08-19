#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32")
#include <windows.h>
#include <thread>
#pragma warning(disable : 4996)

/*
만들어야 하는 기능
1. 클라이언트 기본 세팅 ㅇ
2. 메시지 보내기.o
3. 메시지 받는 거 thread로 구현 ㅇ 
4. Heartbeat 구현 (so_keepalive) ?
*/

void ReceiveMessageFromServer(SOCKET commSock) {
	char szBuffer[128];
	while(::recv(commSock, szBuffer, sizeof(szBuffer), 0) > 0) {
		std::cout << "->" << szBuffer << std::endl;
		memset(szBuffer, 0, sizeof(szBuffer));
	}
	puts("수신 스레드가 끝났습니다.");
	return ;
}

int main(int argc, char * argv[]) {
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		puts("윈속 초기화 실패");
		return 0;
	}
	SOCKET commSock = 0;
	commSock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (commSock == INVALID_SOCKET) {
		puts("ERROR: 클라이언트 통신 소켓 생성 실패");
		return 0;
	}

	int nOpt = 1;
	::setsockopt(commSock, SOL_SOCKET, SO_KEEPALIVE,(char *)&nOpt, sizeof(nOpt));


	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(9999);
	servAddr.sin_family = AF_INET;

	if (::connect(commSock, (SOCKADDR *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		puts("ERROR: 서버와 연결 실패");
		return 0;
	}

	
	std::thread th(ReceiveMessageFromServer, commSock);

	char szBuffer[128];
	puts("채팅을 시작합니다.(종료를 원하시면 \"EXIT\"을 입력해주세요.)");
	while (1)
	{
		//사용자로부터 문자열을 입력 받는다.
		memset(szBuffer, 0, sizeof(szBuffer));
		gets_s(szBuffer);
		if (strcmp(szBuffer, "EXIT") == 0)		break;

		//사용자가 입력한 문자열을 서버에 전송한다.
		::send(commSock, szBuffer, strlen(szBuffer) + 1, 0);

	}
	::closesocket(commSock);
	::Sleep(100);
	th.join();
	::WSACleanup();

	return 0;
}