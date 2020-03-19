#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "68000"


int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    int sendbuflen = DEFAULT_BUFLEN;
    bool endConnection = 0;

    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    } 

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    //// Send an initial buffer
    //iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    //if (iResult == SOCKET_ERROR) {
    //    printf("send failed with error: %d\n", WSAGetLastError());
    //    closesocket(ConnectSocket);
    //    WSACleanup();
    //    return 1;
    //}

    //printf("Bytes Sent: %ld\n", iResult);


    //Antes de fechar o socket temos de fazer v�rios e indeterminados envios de mensagens para o servidor e rece��es.


    // shutdown the connection since no more data will be sent
    //iResult = shutdown(ConnectSocket, SD_SEND);
    //if (iResult == SOCKET_ERROR) {
    //    printf("shutdown failed with error: %d\n", WSAGetLastError());
    //    closesocket(ConnectSocket);
    //    WSACleanup();
    //    return 1;
    //}



    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("%.*s\n", iResult, recvbuf);

            //get line from terminal
            ZeroMemory(sendbuf, DEFAULT_BUFLEN);
            fgets(sendbuf, DEFAULT_BUFLEN, stdin);

            //Remove last character. It was a '\n'
            sendbuflen = strlen(sendbuf);
            sendbuf[sendbuflen-1] = '\0';

            // Transforms all characters in the buffer to Upper Case to allow commands to be case insensitive
            for (int i = 0; i < strlen(sendbuf); i++)
            {
                sendbuf[i] = toupper(sendbuf[i]);
            }

            if (strcmp(sendbuf, "END") == 0)
                break;

            // Send a buffer
            iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }

        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}



