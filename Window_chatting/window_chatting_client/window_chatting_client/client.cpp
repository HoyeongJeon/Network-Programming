#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32")
#include <windows.h>
#include <thread>
#pragma warning(disable : 4996)

/*
������ �ϴ� ���
1. Ŭ���̾�Ʈ �⺻ ���� ��
2. �޽��� ������.o
3. �޽��� �޴� �� thread�� ���� �� 
4. Heartbeat ���� (so_keepalive) ?
*/

void ReceiveMessageFromServer(SOCKET commSock) {
	char szBuffer[128];
	while(::recv(commSock, szBuffer, sizeof(szBuffer), 0) > 0) {
		std::cout << "->" << szBuffer << std::endl;
		memset(szBuffer, 0, sizeof(szBuffer));
	}
	puts("���� �����尡 �������ϴ�.");
	return ;
}

int main(int argc, char * argv[]) {
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		puts("���� �ʱ�ȭ ����");
		return 0;
	}
	SOCKET commSock = 0;
	commSock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (commSock == INVALID_SOCKET) {
		puts("ERROR: Ŭ���̾�Ʈ ��� ���� ���� ����");
		return 0;
	}

	int nOpt = 1;
	::setsockopt(commSock, SOL_SOCKET, SO_KEEPALIVE,(char *)&nOpt, sizeof(nOpt));


	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(9999);
	servAddr.sin_family = AF_INET;

	if (::connect(commSock, (SOCKADDR *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		puts("ERROR: ������ ���� ����");
		return 0;
	}

	
	std::thread th(ReceiveMessageFromServer, commSock);

	char szBuffer[128];
	puts("ä���� �����մϴ�.(���Ḧ ���Ͻø� \"EXIT\"�� �Է����ּ���.)");
	while (1)
	{
		//����ڷκ��� ���ڿ��� �Է� �޴´�.
		memset(szBuffer, 0, sizeof(szBuffer));
		gets_s(szBuffer);
		if (strcmp(szBuffer, "EXIT") == 0)		break;

		//����ڰ� �Է��� ���ڿ��� ������ �����Ѵ�.
		::send(commSock, szBuffer, strlen(szBuffer) + 1, 0);

	}
	::closesocket(commSock);
	::Sleep(100);
	th.join();
	::WSACleanup();

	return 0;
}