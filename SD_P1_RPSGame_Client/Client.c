#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "25565"


// O IP do servidor a que o cliente se ir� conectar � passada como parametro (Project -> SD_PI_RPSGame_Cliente Properties -> Configuration Properties -> Debugging).
int __cdecl main(int argc, char** argv)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    // Variaveis que cont�m informa��o das mensagens entre o cliente e servidor.
    char sendbuf[DEFAULT_BUFLEN]; // Buffer com a string que ser� enviada pelo cliente.
    char recvbuf[DEFAULT_BUFLEN]; // Buffer com a string recebida pelo cliente.
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    int sendbuflen = DEFAULT_BUFLEN;

    // Booleano que indica a inten��o de terminar a conex�o com o servidor.
    bool endConnection = 0;

    // argumento 1 -> execut�vel ; argumento 2 -> IP. Caso isto n�o se verifique � porque o n�mero de parametros � inv�lido.
    /*if (argc != 2) { 
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    } */

    // Initialize a utiliza��o da biblioteca Winsock por um proceso.
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Configura��es dos dados de comunica��o.
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Obtem o endere�o e a porta do servidor.
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Tenta conectar a um endere�o at� que um seja bem sucedido
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Cria��o de um socket para conectar o cliente ao servidor.
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Conectar o cliente ao servidor
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    // Libertar o espa�o de mem�ria ocupado pela informa��o utilizado para criar o socket
    freeaddrinfo(result);

    // Verificar se o socket � v�lido
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Recebe o "100 OK: Connection Established"
    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        printf("%.*s\n", iResult, recvbuf);
    }

    // Recebe informa��o enquanto a conex�o n�o fechar.
    do {

        // Recebe informa��o do lado do servidor a que est� conectado e guarda-a no buffer "recvbuf"
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        
        // No caso de se ter recebido algum caracter.
        if (iResult > 0) {
            printf("%.*s\n", iResult, recvbuf);

            ZeroMemory(sendbuf, DEFAULT_BUFLEN);

            // Obtem do terminal o que o utilizador escreveu.
            fgets(sendbuf, DEFAULT_BUFLEN, stdin);

            // Remove o �ltimo caracter que � o '\n'.
            sendbuflen = strlen(sendbuf);
            sendbuf[sendbuflen-1] = '\0';

            // Transforma todos os caracteres do buffer que contem a mensagem lida para "Upper Case" de forma a que os comandos sejam case insensitive.
            for (int i = 0; i < strlen(sendbuf); i++)
                sendbuf[i] = toupper(sendbuf[i]);

            // No caso do utilizador ter escrito o comando END, sai-se do ciclo while e trata-se de fechar o socket de comunica��o.
            if (strcmp(sendbuf, "END") == 0)
                break;

            // Enviar para o servidor o buffer lido do terminal "sendbuf"
            iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (iResult == 0)// No caso do valor iResult ser 0 � porque n�o se leu nenhuma informa��o do servidor, ou seja, a conex�o n�o est� estabelecida.
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while (iResult > 0);

    // Quando se sair do ciclo � porque j� n�o se pretende comunicar com o servidor ou porque a conex�o foi fechada do lado do servidor, logo, fecha-se o socket e limpam-se possiveis recursos alocados.
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}