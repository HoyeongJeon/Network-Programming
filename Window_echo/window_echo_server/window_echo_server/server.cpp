#include <iostream>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib,"Ws2_32.lib")

int main(int argc, char * argv[]) {
	WSADATA wsa = { 0 };
	SOCKET servSock = INVALID_SOCKET;
	SOCKADDR_IN servAddr = { 0 };

	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cout << "ERROR : ���� �ʱ�ȭ ����" << std::endl;
		return 0;
	}
	
	servSock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (servSock == INVALID_SOCKET) {
		std::cout << "ERROR : ���� ��� ���� ���� ����" << std::endl;
		return 0;
	}
	// IP �ּҿ� ��Ʈ ��ȣ bind
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9999);
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(servSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		std::cout << "ERROR : IP �ּҿ� ��Ʈ��ȣ bind ����" << std::endl;
		return 0;
	}

	if (::listen(servSock, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "ERROR : Listen ����" << std::endl;
		return 0;
	}

	SOCKET clntSock = 0;
	SOCKADDR_IN clntAddr = { 0 };
	int clntAddrLen = sizeof(clntAddr);
	char szBuffer[128] = { 0 };
	int clntReceived = 0;

	while ((clntSock = ::accept(servSock, (SOCKADDR*)&clntAddr, &clntAddrLen)) != INVALID_SOCKET) {
		std::cout << "���ο� Ŭ���̾�Ʈ�� ����Ǿ����ϴ�." << std::endl;
		std::cout << std::flush;
//		puts("�� Ŭ���̾�Ʈ�� ����Ǿ����ϴ�."); 


		while ((clntReceived = ::recv(clntSock, szBuffer, sizeof(szBuffer), 0)) > 0 ) {
			::send(clntSock, szBuffer, sizeof(szBuffer), 0);
			std::cout << "From Client: " << szBuffer << std::endl;
			std::cout << std::flush;
			memset(szBuffer, 0, sizeof(szBuffer));
		}

		// Ŭ���̾�Ʈ ���� ����
		::shutdown(clntSock, SD_BOTH);
		::closesocket(clntSock);
		std::cout << "Ŭ���̾�Ʈ ���� ����" << std::endl;
	}

	::closesocket(servSock);
	::WSACleanup();

	return 0;

}