#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

SOCKET ClientSock;

string toMorseCode(const string& input) {
	// Define a map of characters to Morse code
	map<char, string> morseCode{
	  {'A', ".-"},
	  {'B', "-..."},
	  {'C', "-.-."},
	  {'D', "-.."},
	  {'E', "."},
	  {'F', "..-."},
	  {'G', "--."},
	  {'H', "...."},
	  {'I', ".."},
	  {'J', ".---"},
	  {'K', "-.-"},
	  {'L', ".-.."},
	  {'M', "--"},
	  {'N', "-."},
	  {'O', "---"},
	  {'P', ".--."},
	  {'Q', "--.-"},
	  {'R', ".-."},
	  {'S', "..."},
	  {'T', "-"},
	  {'U', "..-"},
	  {'V', "...-"},
	  {'W', ".--"},
	  {'X', "-..-"},
	  {'Y', "-.--"},
	  {'Z', "--.."},
	  {'0', "-----"},
	  {'1', ".----"},
	  {'2', "..---"},
	  {'3', "...--"},
	  {'4', "....-"},
	  {'5', "....."},
	  {'6', "-...."},
	  {'7', "--..."},
	  {'8', "---.."},
	  {'9', "----."},
	  {' ', " "},
	  {'!', "!"}
	};

	string output;
	for (char c : input) {
		// Convert the character to uppercase
		c = toupper(c);

		// Look up the Morse code for the character
		auto it = morseCode.find(c);
		if (it != morseCode.end()) {
			output += it->second;
			output += " ";
		}
	}

	return output;
}

int main() {
	const char SERVER_IP[] = "127.0.0.1";
	const short SERVER_PORT_NUM = 1111;
	const short BUFF_SIZE = 1024;

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
	ClientSock = socket(AF_INET, SOCK_STREAM, NULL);

	if (ClientSock == INVALID_SOCKET) {
		cout << "Error initialization socket # " << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << "Client socket initialization is OK" << endl;

	// 4. Connecting to Server
	SOCKADDR_IN servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));
	in_addr ip_to_num;
	cout << "New client connecting..." << endl;
	if (inet_pton(AF_INET, SERVER_IP, &ip_to_num) <= 0) {
		cout << "Error in IP translation to special numeric format" << endl;
		closesocket(ClientSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(SERVER_PORT_NUM);
	servInfo.sin_family = AF_INET;

	if (connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo)) != 0) {
		cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	else
		cout << "Connected! Ready to send a message to Server" << endl;

	vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);
	short packet_size = 0;

	while (true) {
		cout << "Your (Client) message to Server: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		string msg = clientBuff.data();
		string msg_code = toMorseCode(msg);

		// Check whether client like to stop chatting 
		if (clientBuff[0] == '!') {
			packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0);

			if (packet_size == SOCKET_ERROR) {
				cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
				closesocket(ClientSock);
				WSACleanup();
				return EXIT_FAILURE;
			}

			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}

		cout << "Client's encoded message: " << msg_code << endl;

		for (int i = 0; i < msg_code.length(); ++i) {
			clientBuff[i] = msg_code[i];
		}

		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return EXIT_FAILURE;
		}




		//packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);

		/*if (packet_size == SOCKET_ERROR) {
			cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
			closesocket(ClientSock);
			WSACleanup();
			return EXIT_FAILURE;
		}
		else
			cout << "Server message: " << servBuff.data() << endl;*/

	}

	closesocket(ClientSock);
	WSACleanup();

	return 0;

}
