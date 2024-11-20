/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   header.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:30 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:30 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADER_HPP
#define HEADER_HPP
#include "include/header.hpp"
// STRUCTS, ENUMS

enum Action
{
    READ_ = 1,
    WRITE_,
};

enum Method
{
    GET_ = 5,
    POST_,
    DELETE_
};

enum HTTP
{
    HTTP1_0 = 10,
    HTTP1_1
};

enum Status
{
    HTTP_NONE = 0,
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_NO_CONTENT = 204,
    // Redirection
    HTTP_MOVE_PERMANENTLY = 301,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_PERMANENT_REDIRECT = 308,
    // Errors
    HTTP_BAD_REQUEST = 400,
    HTTP_FORBIDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_TIMEOUT = 408,
    HTTP_LENGTH_REQUIRED = 411,
    HTTP_CONTENT_TO_LARGE = 413,
    HTTP_URI_TO_LARGE = 414,
    HTTP_HEADER_TO_LARGE = 431,
    // Server Errors
    HTTP_INTERNAL_SERVER = 500,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_GATEWAY_TIMEOUT = 504,
    HTTP_INSUPPORTED_HTTP = 505,
};

struct Location
{
    string location;

    string root;
    map<Method, bool> methods;
    string src;
    string dest;

    Status ret_status;
    string ret_location;

    int autoindex;
    string index;

    size_t limit;

    map<string, string> cgi;
    map<int, string> errors;
};

struct Server
{
    string listen;
    ssize_t port;
    string name;

    map<string, Location> locations;
};

struct Header
{
    Method method;
    string uri;
    HTTP version;

    size_t content_len;
    string content_type;
    string connection;
    string transfer;

    string queries;
    vector<string> cookies;
    vector<string> hosts;

    map<string, bool> conds; 
    map<string, bool> found;
};

struct Request
{
    Header header;
    string buffer;

    size_t read_size;
};


struct Path
{
    string name;
    State state;
};

struct CGI
{
    map<string, string> header;
    vector<string>cookies;
    pid_t pid;
    string exec;

    Path file;
    Path input;
    Path output;
};

struct Response
{
    Header header;

    Status status;
    string cause;

    string buffer;

    Path path;

    bool is_cgi;
    CGI cgi;

    Location *loc;

    size_t full_size;
    size_t write_size;
    size_t read_size;
};

struct Connection
{
    string server_address;
    ssize_t server_port;

    string address;

    int fd;
    int read_fd;
    int write_fd;

    Action action;

    Request req;
    Response res;

    time_t timing;
    string cause;
    bool remove;
};

extern map<int, string> servers_addresses;
extern map<int, ssize_t> servers_ports;

extern map<int, Connection> clients;
extern map<int, int> files;

extern vector<pollfd> pfds;

typedef std::vector<Server> vServer;
// port
extern map<ssize_t, vServer> servers;

extern map<string, string> memetype;
extern string machine_ip_address;

// OPERATOR OVERLOADING
ostream &operator<<(ostream &out, Connection &con);
ostream &operator<<(ostream &out, Server &serv);
ostream &operator<<(ostream &out, Location &loc);
void see_pfds_size();

// FUNCTIIONS
void parse_config(char *filename);
void set_machine_ip_address();
string substr(size_t line, string &str, ssize_t s, ssize_t e);
string to_string(size_t size);
string to_string(HTTP version);
string to_string(Action action);
string to_string(Method method);
string to_string(Status status);
string _tolower(string str);
string _toupper(string &str);
bool _isdigits(string &str);
bool _isaldigits(string &str);
string parse_hexadecimal(string value);
string clear_path(string path);
bool starts_with(string str, string sub);
bool ends_with(string str, string sub);
bool compare(string left, string right);
ssize_t find(string &str, char *to_find);
string substr(size_t line, string &str, ssize_t s, ssize_t e);
bool is_error(Status &status);
void init_memetypes();
void WebServer();
int create_server_connection(Server srv);
void connection_accept(int serv_fd);
void parse_header(Connection &con, string &buff, Header &header);
string trim(string str);
bool check_error(int line, Connection &con, bool cond, Status status, string cause);
bool is_move_dir(Status &status);
ssize_t find_sep(string &buff, string &sep);
State get_state(string &pathname);
string rand_name();
void connection_close(int line, int fd, string cause);
void add_pfd(int fd);

#endif