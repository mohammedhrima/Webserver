#include "header.hpp"
// map<int, Type> types;

void plocation(Location &location, int space)
{
    cout << setw(space) << "root        " << location.root << endl;
    cout << setw(space) << "source      " << location.src << endl;
    cout << setw(space) << "destination " << location.dest << endl;
    cout << setw(space) << "index       " << location.index << endl;
    cout << setw(space) << "autoindex   " << (location.autoindex == on ? "on" : location.autoindex == off ? "off"
                                                                                                          : "none")
         << endl;
    cout << setw(space) << "limit       " << location.limit_body << endl;
    cout << setw(space) << "methods     "
         << " GET " << (location[GET_] ? "on" : "off")
         << " POST " << (location[POST_] ? "on" : "off")
         << " DELETE " << (location[DELETE_] ? "on" : "off") << endl;
    cout << setw(space) << "return      " << location.is_return << (location.is_return ? (" -> " + location.return_location) : "") << endl;
    cout << setw(space) << "cgi:        " << endl;
    map<string, string>::iterator it;
    for (it = location.cgi.begin(); it != location.cgi.end(); it++)
        cout << setw(space) << "ext:      " << it->first << ", path: " << it->second << endl;
    cout << setw(space) << "errors:   " << endl;
    map<int, string>::iterator it_err;
    for (it_err = location.errors.begin(); it_err != location.errors.end(); it_err++)
        cout << setw(space + 5) << "status: " << it_err->first << ", path: " << it_err->second << endl;
}

void pserver(Server &serv)
{
    cout << "listen " << serv.listen << endl;
    cout << "port   " << serv.port << endl;
    cout << "name   " << serv.name << endl;
    map<string, Location>::iterator it;
    for (it = serv.location.begin(); it != serv.location.end(); it++)
    {
        cout << "location: " << it->first << endl;
        plocation(it->second, 14);
    }
}

string con_state(Connection &con)
{
    stringstream stream;
    stream << "type " + to_string(types[con.fd]) +
                  " fd " + to_string(con.fd) +
                  " read_fd " + to_string(con.read_fd) +
                  " write_fd " + to_string(con.write_fd) +
                  " action " + to_string(con.data.action) +
                  " method " + to_string(con.data.method) +
#if 1
                  " requbuff <" + con.data.requbuff + ">" +
                  " respbuff <" + con.data.respbuff + ">" +
#endif
                  " status <" + to_string(con.data.status) + ">";
    return stream.str();
}