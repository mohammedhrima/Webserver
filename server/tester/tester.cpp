/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tester.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:45 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:45 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "tester_cases.cpp"

#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

#define BUFFERSIZE 8192

#define COUT cout << GREEN
#define ERROR cerr << RED
#define END RESET << endl

#define SEND_REQUEST COUT << "====================================SEND REQUEST=====================================" << END
#define RECEIVE_RESPONSE COUT << "==================================RECEIVE RESPONSE===================================" << END
#define END_RESPONSE COUT << "\n====================================END RESPONSE=====================================" << END

int create_socket(string address, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        cerr << "Error creating socket" << endl;
        return -1;
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);                       // Port number
    serverAddr.sin_addr.s_addr = inet_addr(address.c_str()); // Localhost IP address

    if (connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cerr << "Error connecting to server" << endl;
        close(fd);
        return -1;
    }
    return fd;
};

int main()
{
    vector<string> tests = generate_tests();
    for (size_t i = 0; i < tests.size(); i++)
    {
        int fd = create_socket("127.0.0.1", 17000);
        if (fd < 0)
            return -1;

        string req = tests[i];
        if (send(fd, req.c_str(), req.length(), 0) < 0)
            ERROR << "Error sending request: " << req << END;
        else
        {
            SEND_REQUEST;
            cout << req << endl;
            RECEIVE_RESPONSE;
            char buffer[BUFFERSIZE + 1] = {0};
            int bytes = 0;
            while ((bytes = recv(fd, buffer, BUFFERSIZE, 0)) > 0)
            {
                cout << buffer;
                memset(buffer, 0, sizeof(buffer));
            }
            if (bytes < 0)
                ERROR << "Error receiving response" << END;
            END_RESPONSE;
        }
        close(fd);
    }
    return 0;
}
