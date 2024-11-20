#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

using namespace std;

#define PORT 17000

int main()
{
#if 1
    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Set up the server address structure
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);                   // Port number
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost IP address

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Error connecting to server" << std::endl;
        close(sockfd);
        return 1;
    }

    // Send a GET request
    string req, req0, req1, req2;

    ifstream file_name("serv/name1/src/007.txt");
    string text((istreambuf_iterator<char>(file_name)), (istreambuf_iterator<char>()));
#if 0
#else
    req0 = "abcdfskdfklsjd";
    req1 = "GET http://127.0.0.1:17000/004.txt HTTP/1.1\r\n"
           "Host: name\r\n"
           "Content-Length: " +
           to_string(text.length()) + "\r\n"
                                      "\r\n" +
           text;

    // req2 = "abcdefgfdgfdgdfgodufhgdsjhgkjdfhkjsghkdsgkhd";
    // string s = to_string(req2.length());
    // req2 = "POST /ab HTTP/1.1\r\n"
    //        "Host: empty\r\n"
    //        "Content-Type: text/plain\r\n"
    //        "Content-Length: " +
    //        s + "\r\n"
    //            "\r\n" +
    //        req2;

#endif
    req = req1; //+ req2 + req1 + req2;
    size_t i = 0;
    while (i < 1)
    {
        // cout << "send: " << req << "\n\n";
        if (send(sockfd, req.c_str(), req.length(), 0) < 0)
        {
            std::cerr << "Error sending request" << std::endl;
            // close(sockfd);
            break;
            // return 1;
        }
        i++;
    }

    // Receive response
    char buffer[1024];
    int bytesRead;
    i = 0;
    while ((bytesRead = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        // Print received data (you may want to handle it differently)
        cout << "========================================================================" << endl;
        std::cout.write(buffer, bytesRead);
        cout << "========================================================================" << endl;
        i++;
    }
    cout << "receive " << i << " response" << endl;

    // Check for receive errors
    if (bytesRead < 0)
    {
        std::cerr << "Error receiving response" << std::endl;
    }

    // Close the socket
    close(sockfd);
#else

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (sockfd < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Set up server address structure
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(17000); // Example port
    std::string ipddr = "10.13.10.7";
    serverAddr.sin_addr.s_addr = inet_addr(ipddr.c_str()); // Example IP address

    // Bind socket to specific IP address
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    std::cout << "Socket bound to IP address " << ipddr << std::endl;

#endif
    // Continue with socket operations...
    std::cout << std::endl;
    return 0;
}
