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

    // Проверка аргументов
    if (argc != 4) {
        printf("Использование: %s <операция> <число1> <число2>\n", argv[0]);
        printf("Допустимые операции: add, sub, mul\n");
        return 1;
    }

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Ошибка инициализации Winsock\n");
        return 1;
    }

    // Создание сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Ошибка создания сокета\n");
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Ошибка подключения к серверу\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Создание JSON-запроса
    json_t* request = json_object();
    json_object_set_new(request, "action", json_string(argv[1]));
    json_object_set_new(request, "argument_1", json_real(atof(argv[2])));
    json_object_set_new(request, "argument_2", json_real(atof(argv[3])));
    char* request_str = json_dumps(request, 0);

    // Отправка запроса
    send(client_socket, request_str, strlen(request_str), 0);
    printf("Отправлен запрос: %s\n", request_str);

    // Получение ответа
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Ошибка получения ответа\n");
    }
    else {
        buffer[bytes_received] = '\0';
        printf("Получен ответ: %s\n", buffer);
    }

    // Освобождение ресурсов
    free(request_str);
    json_decref(request);
    closesocket(client_socket);
    WSACleanup();

    return 0;
}