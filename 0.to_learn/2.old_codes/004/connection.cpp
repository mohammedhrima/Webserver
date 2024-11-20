#include "header.hpp"

// CREATE SERVER / CLIENT CONNECTION
int create_server_connection(Server &srv)
{
    Connection con = {};
    con.type = SERVER_;
    con.srv_port = srv.port;
    con.srv_listen = srv.listen;
    int &fd = con.fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERROR << "failed to create server socket" << END;
        return 1;
    }
    int option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        close(fd);
        ERROR << "setsockopt failed" << END;
        return 1;
    }
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(srv.port);

    if (inet_pton(AF_INET, srv.listen.c_str(), &addr.sin_addr) <= 0)
    {
        close(fd);
        ERROR << "Invalid address: " << srv.listen << END;
        return 1;
    }
    if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        close(fd);
        ERROR << "bind failed in " << srv.listen << END;
        return 1;
    }
    if (listen(fd, LISTEN_LEN))
    {
        close(fd);
        ERROR << "listen failed" << END;
        return 1;
    }
    char ipstr[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &addr.sin_addr, ipstr, INET_ADDRSTRLEN))
    {
        close(fd);
        ERROR << "inet_ntop failed" << END;
        return 1;
    }
    con.address = string(ipstr);
    pfds.push_back((pollfd){.fd = fd, .events = POLLIN | POLLOUT});
    types[fd] = SERVER_;
    cout << "New server connection has fd " << con.fd << " address " << ipstr << endl;
    cons[fd] = con;
    servers[srv.port].push_back(srv);
    return 0;
}

void connection_accept(int serv_fd)
{
    if (!cons.count(serv_fd))
    {
        throw Error("accepting client from server ");
        cerr << "Error: accepting client from server " << serv_fd << endl;
        return;
    }
    Connection srv = cons[serv_fd];
    Connection con = (Connection){0};

    con.type = CLIENT_;
    con.action = READ_;
    con.srv_port = srv.srv_port;
    con.srv_listen = srv.srv_listen;
    sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    con.fd = accept(serv_fd, (sockaddr *)&addr, &len);
    if (con.fd < 0)
    {
        ERROR << "Failed to accept connection from " << srv.srv_listen << ":" << srv.srv_port << END;
        return;
    }
    char ipstr[INET_ADDRSTRLEN];   // Buffer to hold the IP address string
    if (addr.ss_family == AF_INET) // Check if the address family is IPv4
    {
        sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(&addr);
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, INET_ADDRSTRLEN);
    }
    else
    {
        // Unsupported address family
        ERROR << "Unsupported address family" << END;
        return;
    }
    con.address = ipstr;
    con.read_fd = con.fd;
    con.write_fd = -1;
    pfds.push_back((pollfd){.fd = con.fd, .events = POLLIN | POLLOUT});
    types[con.fd] = CLIENT_;
    cout << "Accept client " << con.address << " has fd " << con.fd << endl;
    con.timeout = time(NULL);
    cons[con.fd] = con;
}
