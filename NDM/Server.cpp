#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <jansson.h>

#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 4096

void handle_request(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Получаем JSON-запрос от клиента
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Ошибка получения данных\n");
        return;
    }
    buffer[bytes_received] = '\0';

    // Парсим JSON
    json_error_t error;
    json_t* root = json_loads(buffer, 0, &error);
    if (!root) {
        printf("Ошибка разбора JSON: %s\n", error.text);
        return;
    }

    // Извлекаем параметры
    const char* action;
    double arg1, arg2;
    json_unpack(root, "{s:s, s:f, s:f}",
        "action", &action,
        "argument_1", &arg1,
        "argument_2", &arg2);

    // Выполняем операцию
    double result = 0;
    if (strcmp(action, "add") == 0) {
        result = arg1 + arg2;
    }
    else if (strcmp(action, "sub") == 0) {
        result = arg1 - arg2;
    }
    else if (strcmp(action, "mul") == 0) {
        result = arg1 * arg2;
    }
    else {
        printf("Неизвестная операция: %s\n", action);
        json_decref(root);
        return;
    }

    // Формируем ответ
    json_t* response = json_object();
    json_object_set_new(response, "result", json_real(result));
    char* response_str = json_dumps(response, 0);

    // Отправляем ответ
    send(client_socket, response_str, strlen(response_str), 0);

    // Освобождаем ресурсы
    free(response_str);
    json_decref(response);
    json_decref(root);
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Ошибка инициализации Winsock\n");
        return 1;
    }

    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Ошибка создания сокета\n");
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязка сокета
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Ошибка привязки сокета\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Ожидание подключений
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        printf("Ошибка listen\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Сервер запущен на порту %d...\n", PORT);

    // Основной цикл сервера
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket == INVALID_SOCKET) {
            printf("Ошибка accept\n");
            continue;
        }

        printf("Новое подключение\n");
        handle_request(client_socket);
        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}