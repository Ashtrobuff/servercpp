#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>


void handleClientRequest(int clientSocket, const std::unordered_map<std::string, std::string>& routes) {
    const int bufferSize = 1024;
    char buffer[bufferSize] = {0};
    
    // Read the client's request
    int bytesReceived = read(clientSocket, buffer, bufferSize - 1);
    if (bytesReceived < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        close(clientSocket);
        return;
    }

    // Print the request (for debugging purposes)
    std::cout << "Client Request:\n" << buffer << std::endl;

    // Parse the first line of the request to extract the HTTP method and path
    std::string request(buffer);
    std::string method, path;
    std::size_t methodEnd = request.find(' ');
    if (methodEnd != std::string::npos) {
        method = request.substr(0, methodEnd);
        std::size_t pathStart = methodEnd + 1;
        std::size_t pathEnd = request.find(' ', pathStart);
        if (pathEnd != std::string::npos) {
            path = request.substr(pathStart, pathEnd - pathStart);
        }
    }

    // Check if the requested path exists in the routes
    std::string response;
    if (routes.find(path) != routes.end()) {
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + routes.at(path);
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 - Page Not Found</h1>";
    }

    // finally send response
    send(clientSocket, response.c_str(), response.length(), 0);

    // Close the socket connection
    close(clientSocket);
}

int main() {
  
    const int port = 8080;
    
    std::unordered_map<std::string, std::string>routes = {
        {"/", "<h1>Welcome to the Home Page</h1>"},
        {"/about", "<h1 style=""color:red"">About Us</h1><p>This is the about page.</p>"},
        {"/contact", "<h1>Contact Us</h1><p>Email us at ashishpatil@example.com</p>"}
    };

    // making a socket and family specifying
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        std::cerr << "Socket failed" << std::endl;
        return -1;
    }

    //socket address (matlab server address)
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Binding socket
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    // start listening,max 3 lsteners
    if (listen(serverSocket, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }

    std::cout << "Server is listening on port " << port << std::endl;

    while (true) {
        // Accept a new connection
        int addrlen = sizeof(address);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (clientSocket < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        handleClientRequest(clientSocket, routes);
    }

   //good practice close whatever started :)
    close(serverSocket);

    return 0;
}


//sheeeeeeesh:)