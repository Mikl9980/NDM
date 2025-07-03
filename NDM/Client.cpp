#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <jansson.h>

#pragma comment(lib, "ws2_32.lib")
#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 4096

int main(int argc, char** argv) {
    WSADATA wsaData;
    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // �������� ����������
    if (argc != 4) {
        printf("�������������: %s <��������> <�����1> <�����2>\n", argv[0]);
        printf("���������� ��������: add, sub, mul\n");
        return 1;
    }

    // ������������� Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("������ ������������� Winsock\n");
        return 1;
    }

    // �������� ������
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("������ �������� ������\n");
        WSACleanup();
        return 1;
    }

    // ��������� ������ �������
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // ����������� � �������
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("������ ����������� � �������\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // �������� JSON-�������
    json_t* request = json_object();
    json_object_set_new(request, "action", json_string(argv[1]));
    json_object_set_new(request, "argument_1", json_real(atof(argv[2])));
    json_object_set_new(request, "argument_2", json_real(atof(argv[3])));
    char* request_str = json_dumps(request, 0);

    // �������� �������
    send(client_socket, request_str, strlen(request_str), 0);
    printf("��������� ������: %s\n", request_str);

    // ��������� ������
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("������ ��������� ������\n");
    }
    else {
        buffer[bytes_received] = '\0';
        printf("������� �����: %s\n", buffer);
    }

    // ������������ ��������
    free(request_str);
    json_decref(request);
    closesocket(client_socket);
    WSACleanup();

    return 0;
}