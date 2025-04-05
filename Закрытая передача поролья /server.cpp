#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <thread>
#include <postgresql/libpq-fe.h>

using namespace std;

PGconn* conn = nullptr;

// ==============================================
// Db Functions
// ==============================================

bool is_user_exists(const string& nickname) {
    const char* query = "SELECT id FROM pr3 WHERE login = $1";
    const char* params[1] = {nickname.c_str()};

    PGresult* res = PQexecParams(conn, query, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "Query execution error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return false;
    }

    bool exists = (PQntuples(res) > 0);
    PQclear(res);
    return exists;
}

string get_salt_by_login(const string& login) {
    const char* query = "SELECT salt FROM pr3 WHERE login = $1";
    const char* params[1] = {login.c_str()};

    PGresult* res = PQexecParams(conn, query, 1, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "Query execution error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return "";
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return "";
    }

    string salt = PQgetvalue(res, 0, 0);
    PQclear(res);
    return salt;
}

bool check_user(const string& nickname, const string& password) {
    const char* query = "SELECT id FROM pr3 WHERE login = $1 AND password_hash = $2";
    const char* params[2] = {nickname.c_str(), password.c_str()};

    PGresult* res = PQexecParams(conn, query, 2, nullptr, params, nullptr, nullptr, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "Query execution error: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return false;
    }

    bool user_exists = (PQntuples(res) > 0);
    PQclear(res);
    return user_exists;
}

bool Connect_db(const string& ip_adr, const string& db_name, 
               const string& username, const string& passworld) {
    string conn_str = "user=" + username + " password=" + passworld + 
                     " host=" + ip_adr + " dbname=" + db_name;
    conn = PQconnectdb(conn_str.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        cerr << "Database connection error: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return false;
    }

    return true;
}

// ==============================================
// Client Handling Functions
// ==============================================

void client_reg(int clientSocket) {
    // Receive nickname
    char nickname[1024] = {0};
    recv(clientSocket, nickname, sizeof(nickname), 0);
    cout << "Received nickname: " << nickname << endl;

    // Generate and send random number
    srand(time(0));
    int randomNumber = rand() % 256;
    if (is_user_exists(nickname)) {
        randomNumber = 0;
    }

    string response = to_string(randomNumber);
    send(clientSocket, response.c_str(), response.size(), 0);

    // Receive password
    char password[1024] = {0};
    recv(clientSocket, password, sizeof(password), 0);
    cout << "Received password: " << password << endl;

    // Register new user if not exists
    if (!is_user_exists(nickname)) {
        const char* query = "INSERT INTO pr3 (login, password_hash, salt) VALUES ($1, $2::bytea, $3)";
        const char* params[3] = {nickname, password, to_string(randomNumber).c_str()};

        PGresult* res = PQexecParams(conn, query, 3, nullptr, params, nullptr, nullptr, 0);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            cerr << "User registration error: " << PQerrorMessage(conn) << endl;
        }

        PQclear(res);
    }

    close(clientSocket);
}

void auntifekasion(int clientSocket) {
    // Receive nickname
    char nickname[1024] = {0};
    recv(clientSocket, nickname, sizeof(nickname), 0);
    cout << "Received nickname: " << nickname << endl;

    // Get salt from DB
    string salt = get_salt_by_login(nickname);
    if (salt.empty()) {
        send(clientSocket, "user_not_found", 14, 0);
        return;
    }

    // Send salt to client
    send(clientSocket, salt.c_str(), salt.size(), 0);

    // Receive password
    char passwd[1024] = {0};
    recv(clientSocket, passwd, sizeof(passwd), 0);

    // Verify user
    if (check_user(nickname, passwd)) {
        send(clientSocket, "yes", 3, 0);
    } else {
        send(clientSocket, "no", 2, 0);
    }
}

// ==============================================
// Main Server Functions
// ==============================================

void handle_client(int clientSocket) {
    char command[2] = {0};
    recv(clientSocket, command, sizeof(command), 0);

    switch (command[0]) {
        case '1':
            cout << "Starting registration process..." << endl;
            client_reg(clientSocket);
            break;
        case '2':
            cout << "Starting authentication process..." << endl;
            auntifekasion(clientSocket);
            break;
        default:
            cerr << "Unknown command: " << command[0] << endl;
            send(clientSocket, "unknown_command", 15, 0);
    }

    close(clientSocket);
}

int main() {
    // Database connection
    if (!Connect_db("127.0.0.1", "db_name", "username", "password")) {
        return 1;
    }

    // Create server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Socket creation error" << endl;
        return 1;
    }

    // Configure server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "Socket binding error" << endl;
        close(serverSocket);
        return 1;
    }

    // Start listening
    if (listen(serverSocket, 5) < 0) {
        cerr << "Socket listening error" << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Server started and waiting for connections..." << endl;

    vector<thread> threads;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cerr << "Connection accept error" << endl;
            continue;
        }

        threads.emplace_back(handle_client, clientSocket);
    }

    // Cleanup (unreachable in current implementation)
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    close(serverSocket);
    return 0;
}