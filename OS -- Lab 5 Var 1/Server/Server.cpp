#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET Connections[100];
int Counter = 0;

const char IP_SERV[] = "127.0.0.1";
const int PORT_NUM = 1111;
const short BUFF_SIZE = 1024;

HANDLE access = CreateMutex(0, 0, 0);


string morseToText(string morseCode) {
	map<string, char> morseMap = {
	  {".-", 'A'},
	  {"-...", 'B'},
	  {"-.-.", 'C'},
	  {"-..", 'D'},
	  {".", 'E'},
	  {"..-.", 'F'},
	  {"--.", 'G'},
	  {"....", 'H'},
	  {"..", 'I'},
	  {".---", 'J'},
	  {"-.-", 'K'},
	  {".-..", 'L'},
	  {"--", 'M'},
	  {"-.", 'N'},
	  {"---", 'O'},
	  {".--.", 'P'},
	  {"--.-", 'Q'},
	  {".-.", 'R'},
	  {"...", 'S'},
	  {"-", 'T'},
	  {"..-", 'U'},
	  {"...-", 'V'},
	  {".--", 'W'},
	  {"-..-", 'X'},
	  {"-.--", 'Y'},
	  {"--..", 'Z'},
	  {"-----", '0'},
	  {".----", '1'},
	  {"..---", '2'},
	  {"...--", '3'},
	  {"....-", '4'},
	  {".....", '5'},
	  {"-....", '6'},
	  {"--...", '7'},
	  {"---..", '8'},
	  {"----.", '9'}
	};

	string result = "";
	string current = "";

	for (char c : morseCode) {
		if (c == ' ') {
			if (morseMap.count(current) > 0) {
				result += morseMap[current];
			}
			current = "";
		}
		else {
			current += c;
		}
	}

	if (morseMap.count(current) > 0) {
		result += morseMap[current];
	}

	return result;
}

void ClientHandler(int index) {
	WaitForSingleObject(access, INFINITE);
	int ind = index;
	//cout << index;
	vector<char> msg(1024);
	while (true) {
		recv(Connections[index], msg.data(), 1024, 0);
		if (msg[0] == '!') {
			shutdown(Connections[index], SD_BOTH);
			closesocket(Connections[index]);
			ReleaseMutex(access);
			//WSACleanup();
			return;
		}

		//HANDLE endEvent = OpenEvent(0, 1, L"endSession");
		//WaitForSingleObject(endEvent, INFINITE);
		//if (endEvent) {
		//	ResetEvent(endEvent);
		//	shutdown(Connections[index], SD_BOTH);
		//	closesocket(Connections[index]);
		//	ReleaseMutex(access);
		//	//CloseHandle(endEvent);
		//	//WSACleanup();
		//	return;
		//}

		
		cout << "[" << ind << "] " << "Client's code: " << msg.data() << endl;

		string s = msg.data();
		string s_decode = morseToText(s);
		cout << "[" << ind << "] Client: " << s_decode << endl;


		/*for (int i = 0; i < msg.size(); ++i) {
			if (msg[i] >= 'a' && msg[i] <= 'z') {
				msg[i] = ('a' + (msg[i] - 'a' - 3 + 26) % 26);
			}
			else if (msg[i] >= 'A' && msg[i] <= 'Z') {
				msg[i] = ('A' + (msg[i] - 'A' - 3 + 26) % 26);
			}
		}*/

		//cout << "Client's message: " << msg.data() << endl;
		// 
		//cout << '[' << IP_SERV << "]: " << msg.data() << endl;
	}
	ReleaseMutex(access);
}

int main() {
	// 1. WinSock initialization
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		cout << WSAGetLastError() << endl;
		exit(EXIT_FAILURE);
	}
	else
		cout << "WinSock initialization is OK" << endl;

	// 2. Socket initialization
	SOCKET ServSock = socket(AF_INET, SOCK_STREAM, NULL);

	if (ServSock == INVALID_SOCKET) {
		cout << "Error initialization socket # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << "Server socket initialization is OK" << endl;

	// 3. Bind the socket to a local address and port
	SOCKADDR_IN servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));
	int sizeofaddr = sizeof(servInfo);
	in_addr ip_to_num;
	if (inet_pton(AF_INET, IP_SERV, &ip_to_num) <= 0) {
		cout << "Error in IP translation to special numeric format" << endl;
		closesocket(ServSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(PORT_NUM);
	servInfo.sin_family = AF_INET;

	if (bind(ServSock, (SOCKADDR*)&servInfo, sizeof(servInfo)) != 0) {
		cout << "Error Socket binding to server info. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << "Binding socket to Server info is OK" << endl;

	// 4. Listening to the linked port
	if (listen(ServSock, 2) != 0) {
		cout << "Can't start to listen to. Error # " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << "Listening..." << endl;

	// 5. Accepting of connection
	sockaddr_in clientInfo;
	ZeroMemory(&clientInfo, sizeof(clientInfo));
	int clientInfo_size = sizeof(clientInfo);

	SOCKET ClientConn;

	for (int i = 0; i < 100; ++i) {
		//access = CreateMutex(0, 0, 0);
		//WaitForSingleObject(access, INFINITE);

		ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

		if (ClientConn == INVALID_SOCKET) {
			cout << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
			closesocket(ServSock);
			closesocket(ClientConn);
			WSACleanup();
			exit(EXIT_FAILURE);
		}
		else {
			cout << "Connection to a client established successfully" << endl;
			char clientIP[22];
			inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
			cout << "Client connected with IP address " << clientIP << endl;
		}

		Connections[i] = ClientConn;
		Counter++;

		//access = CreateMutex(0, 0, 0);
		//WaitForSingleObject(access, INFINITE);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
		//ReleaseMutex(access);
	}

	//while (true) {
	//	packet_size = recv(ClientConn, servBuff.data(), servBuff.size(), 0);

	//	if (servBuff[0] == '1' && servBuff[1] == '2' && servBuff[2] == '3') {
	//		shutdown(ClientConn, SD_BOTH);
	//		closesocket(ClientConn);
	//		closesocket(ServSock);
	//		WSACleanup();
	//		return 0;
	//	}

	//	decryption(servBuff);

	//	cout << "Client's message: " << servBuff.data() << endl;

	//	/*cout << "Your (host) message: ";
	//	fgets(clientBuff.data(), clientBuff.size(), stdin);*/

	//	// Check whether server would like to stop chatting 
	//	/*if (servBuff[0] == '1' && servBuff[1] == '2' && servBuff[2] == '3') {
	//		shutdown(ClientConn, SD_BOTH);
	//		closesocket(ClientConn);
	//		closesocket(ServSock);
	//		WSACleanup();
	//		return 0;
	//	}*/

	//	/*packet_size = send(ClientConn, clientBuff.data(), clientBuff.size(), 0);

	//	if (packet_size == SOCKET_ERROR) {
	//		cout << "Can't send message to Client. Error # " << WSAGetLastError() << endl;
	//		closesocket(ServSock);
	//		closesocket(ClientConn);
	//		WSACleanup();
	//		return EXIT_FAILURE;
	//	}*/

	//}

	closesocket(ServSock);
	for (int i = 0; i < Counter; ++i) {
		closesocket(Connections[i]);
	}
	WSACleanup();

	return 0;
}