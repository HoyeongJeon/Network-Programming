#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32")
#include <windows.h>
#include <list>
#include <thread>
#include <iterator>
#include <mutex>
#include <vector>

/*
만들어야 하는 기능
1. 서버 기본 세팅 ㅇ 
2. 클라이언트 연결 시 관리 list에 넣기 ㅇ 
3. 클라이언트 메시지 받는 거 thread로 분리 ㅇ 
4. 클라이언트에게서 받은 메시지 다른 클라이언트에게 전송 ㅇ
5. CTRL_C 눌렀을 경우 종료 ㅇ
6. HB 구현 
*/


SOCKET g_listenSocket = 0;
std::list<SOCKET> g_clntList;
std::mutex g_m;  // mutex 객체


BOOL AddClnt(SOCKET clntSock) {
	g_m.lock();
	g_clntList.push_back(clntSock);
	std::cout << "ok" << std::endl;
	g_m.unlock();
	return TRUE;
}

void SendMessageToClient(char* szBuffer) {
	std::list<SOCKET>::iterator it;
	g_m.lock();
	for (it = g_clntList.begin(); it != g_clntList.end(); ++it) {
		::send(*it, szBuffer, strlen(szBuffer) + 1, 0);
	}
	g_m.unlock();
}

void ReceiveMessageFromClient(SOCKET clntSock) {
	char szBuffer[128];
	std::cout << "클라이언트가 접속했습니다." << std::endl;
	while (::recv(clntSock, szBuffer, sizeof(szBuffer), 0) > 0) {
		std::cout << szBuffer << std::endl;
		SendMessageToClient(szBuffer);
		memset(szBuffer, 0, sizeof(szBuffer));
	}
	
	puts("Client 연결을 끊었습니다.");
	g_m.lock();
	g_clntList.remove(clntSock);
	g_m.unlock();


	::closesocket(clntSock);

}

BOOL WINAPI IsItControlC(DWORD fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT) {
		std::list<SOCKET>::iterator it;
		// 연결된 모든 클라이언트 및 리슨 소켓 닫고 종료
		::shutdown(g_listenSocket, SD_BOTH);
		g_m.lock();
		for (it = g_clntList.begin(); it != g_clntList.end(); ++it) {
			closesocket(*it);
		}
		g_clntList.clear();
		g_m.unlock();;
		puts("모든 클라이언트 연결을 종료했습니다.");
		::Sleep(100);
		::closesocket(g_listenSocket);

		// 윈속 해제
		::WSACleanup();
		exit(0);
	}
	return TRUE;
};

int main(int argc, char* argv[]) {
	WSADATA wsa = { 0 };
	::WSAStartup(MAKEWORD(2, 2), &wsa);

	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)IsItControlC, TRUE) == FALSE) {
		puts("ERROR: Ctrl+C 처리기 등록 실패");
	}

	g_listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (g_listenSocket == INVALID_SOCKET) {
		puts("ERROR: 소켓 생성 실패");
		return 0;
	}

	SOCKADDR_IN servAddr = { 0 };
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9999);
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	
	
	if (::bind(g_listenSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		puts("ERROR : 서버 소켓에 IP와 PORT 번호 바인딩 실패");
		return 0;
	}

	if (::listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		puts("ERROR : 서버 소켓 리슨 실패");
		return 0;
	}
	
	puts("*** 채팅서버를 시작합니다. ***");

	std::vector<std::thread> threads;
	// 클라이언트 접속 처리
	SOCKET clntSock = 0;
	char szBuffer[128];
	SOCKADDR_IN clntAddr;
	int clntAddrLen = sizeof(clntAddr);
	
	while ((clntSock = ::accept(g_listenSocket, (SOCKADDR*)&clntAddr, &clntAddrLen)) != INVALID_SOCKET) {

		if (AddClnt(clntSock) == FALSE) {
			puts("유저 연결에 실패했습니다.	");
			break;
		}


		std::thread th(ReceiveMessageFromClient, clntSock);
		threads.push_back(std::move(th));
	}
	for (auto& thread : threads) {
		thread.join();
	}

	puts("*** 채팅서버를 종료합니다. ***");
	return 0;

}