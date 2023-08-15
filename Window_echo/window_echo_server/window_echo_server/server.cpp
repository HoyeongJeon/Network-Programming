#include <iostream>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib,"Ws2_32.lib")

int main(int argc, char * argv[]) {
	WSADATA wsa = { 0 };
	SOCKET servSock = INVALID_SOCKET;
	SOCKADDR_IN servAddr = { 0 };

	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cout << "ERROR : 윈속 초기화 실패" << std::endl;
		return 0;
	}
	
	servSock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (servSock == INVALID_SOCKET) {
		std::cout << "ERROR : 접속 대기 소켓 생성 실패" << std::endl;
		return 0;
	}
	// IP 주소와 포트 번호 bind
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9999);
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		std::cout << "ERROR : IP 주소와 포트번호 bind 실패" << std::endl;
		return 0;
	}

	if (::listen(servSock, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "ERROR : Listen 실패" << std::endl;
		return 0;
	}

	SOCKET clntSock = 0;
	SOCKADDR_IN clntAddr = { 0 };
	int clntAddrLen = sizeof(clntAddr);
	char szBuffer[128] = { 0 };
	int clntReceived = 0;

	while ((clntSock = ::accept(servSock, (SOCKADDR*)&clntAddr, &clntAddrLen)) != INVALID_SOCKET) {
		std::cout << "새로운 클라이언트와 연결되었습니다." << std::endl;
		std::cout << std::flush;
//		puts("새 클라이언트가 연결되었습니다."); 


		while ((clntReceived = ::recv(clntSock, szBuffer, sizeof(szBuffer), 0)) > 0 ) {
			::send(clntSock, szBuffer, sizeof(szBuffer), 0);
			std::cout << "From Client: " << szBuffer << std::endl;
			std::cout << std::flush;
			memset(szBuffer, 0, sizeof(szBuffer));
		}

		// 클라이언트 연결 종료
		::shutdown(clntSock, SD_BOTH);
		::closesocket(clntSock);
		std::cout << "클라이언트 연결 종료" << std::endl;
	}

	::closesocket(servSock);
	::WSACleanup();

	return 0;

}