#include <iostream>
#include <vector>
#include <string>
#include <WinSock2.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma warning(disable: 4996)


using namespace std;

SOCKET connection;


void ClientHandler() {
	int msg_size;
	while (true) {
		recv(connection, (char*)&msg_size, sizeof(int), NULL);
		char *msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(connection, msg, msg_size, NULL);
		std::cout << msg << std::endl;
		delete[] msg;
	}
}

std::string getLocalIP() {
	char ac[INET_ADDRSTRLEN];
	ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
	PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);

	if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAddresses);
		pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
	}

	if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen) == NO_ERROR) {
		for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next) {

			if (pCurrAddresses->OperStatus == IfOperStatusUp) {
				if (pCurrAddresses->FirstUnicastAddress != NULL) {
					inet_ntop(AF_INET, &((struct sockaddr_in*)pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr)->sin_addr, ac, sizeof(ac));
					free(pAddresses);
					return std::string(ac);
				}
				else {
					std::cout << "No unicast address found for this adapter.\n";
				}
			}
		}
	}
	else {
		std::cerr << "GetAdaptersAddresses failed with error: " << GetLastError() << "\n";
	}

	free(pAddresses);
	return "";
}


int main() {
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);

	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		cout << "Error" << endl;
		exit(1);
	}

	SOCKADDR_IN addr;

	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8080);
	addr.sin_family = AF_INET;

	connection = socket(AF_INET, SOCK_STREAM, NULL);

	if (connect(connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		cout << "Error:failed connect to server!\n";
		return 1;
	}
	cout << "Connect to server!\n";
	

	send(connection, getLocalIP().c_str(), getLocalIP().size(), 0);
	
	


	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);


	string msg1;
	while (true) {
		getline(cin, msg1);
		int msg_size = msg1.size();
		send(connection, (char*)&msg_size, sizeof(int), NULL);
		send(connection, msg1.c_str(), msg_size, NULL);
		Sleep(10);
	}

	system("pause");
}
