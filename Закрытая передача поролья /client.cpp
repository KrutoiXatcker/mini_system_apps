#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sha256.h"

using namespace std;

// ==============================================
// Authentication Functions
// ==============================================

void register_user(int clientSocket, const string& nickname, string password) {
    // Send nickname to server
    send(clientSocket, nickname.c_str(), nickname.size(), 0);

    // Receive server response (salt value)
    char buffer[1024] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << "Server response: " << buffer << endl;

    // Process password if registration is possible
    if (stoi(buffer) != 0) {
        password = salt_hash_bitwise(sha256(password.c_str()), stoi(buffer));
    } else {
        cout << "Username already taken" << endl;
        close(clientSocket);
        return;
    }

    // Send processed password
    send(clientSocket, password.c_str(), password.size(), 0);
    close(clientSocket);
}

void authenticate_user(int clientSocket, const string& nickname, string password) {
    // Send nickname to server
    send(clientSocket, nickname.c_str(), nickname.size(), 0);

    // Receive salt from server
    char buffer[1024] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    
    // Process password with received salt
    password = salt_hash_bitwise(sha256(password.c_str()), stoi(buffer));
    
    // Send processed password
    send(clientSocket, password.c_str(), password.size(), 0);

    // Receive authentication result
    char auth_result[1024] = {0};
    recv(clientSocket, auth_result, sizeof(auth_result), 0);
    cout << "Authentication result: " << auth_result << endl;
    
    close(clientSocket);
}

// ==============================================
// User Interface Functions
// ==============================================

void register_user_ui(int clientSocket) {
    string nickname, password;
    
    cout << "Enter nickname for registration: ";
    cin >> nickname;
    cout << "Enter password: ";
    cin >> password;
    
    register_user(clientSocket, nickname, password);
}

void authenticate_user_ui(int clientSocket) {
    string nickname, password;
    
    cout << "Enter nickname for authentication: ";
    cin >> nickname;
    cout << "Enter password: ";
    cin >> password;
    
    authenticate_user(clientSocket, nickname, password);
}

// ==============================================
// Main Client Functions
// ==============================================

void handle_client_request(int clientSocket) {
    char choice;
    cout << "For registration enter 1" << endl
         << "For authentication enter 2" << endl;
    cin >> choice;

    switch (choice) {
        case '1':
            send(clientSocket, "1", 1, 0);
            register_user_ui(clientSocket);
            break;
        case '2':
            send(clientSocket, "2", 1, 0);
            authenticate_user_ui(clientSocket);
            break;
        default:
            cout << "Invalid choice" << endl;
            break;
    }
}

int create_client_socket() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Socket creation error" << endl;
        exit(1);
    }
    return clientSocket;
}

void connect_to_server(int clientSocket) {
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Connection failed" << endl;
        close(clientSocket);
        exit(1);
    }
}

int main() {
    // Initialize client socket
    int clientSocket = create_client_socket();
    
    // Connect to server
    connect_to_server(clientSocket);
    
    // Handle client requests
    handle_client_request(clientSocket);

    return 0;
}