#include <iostream>
#include <cstring>
#include <string>
#include <winsock2.h>

#pragma comment(lib,"Ws2_32.lib")

int main(int argc, char * argv[]) {
	WSADATA wsa = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET commSock = ::socket(AF_INET, SOCK_STREAM, 0);
	if(commSock == INVALID_SOCKET) {
		std::cerr << "ERROR: ���� ���� ����" << std::endl;
		return 0;
	}
	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9999);
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if(::connect(commSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		std::cerr << "ERROR: ���� ���� ����" << std::endl;
		return 0;
	}
	char szBuffer[128] = { 0 };
	while (1) {
		std::cout << "To Server(���Ḧ ���Ͻø� \"EXIT\"�� �Է��ϼ���.) : ";
		gets_s(szBuffer); // std::getline ����� ��������, stirng ����� �ؾ���. ����? string ����� �ʹ� �۾Ƽ� szBuffer�� ����Ϸ��� gets_s()�� �� ������
		if (strcmp(szBuffer, "EXIT") == 0) {
			std::cout << "EXIT �Ǿ����ϴ�." << std::endl;
			break;
		}

		::send(commSock, szBuffer, strlen(szBuffer)+1, 0);
		memset(szBuffer, 0, sizeof(szBuffer));
		::recv(commSock, szBuffer, sizeof(szBuffer), 0);
		std::cout << "From Server: " << szBuffer << std::endl;
	}

	::shutdown(commSock, SD_BOTH);
	::closesocket(commSock);
	WSACleanup();

}