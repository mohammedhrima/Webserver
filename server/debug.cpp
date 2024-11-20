/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:35 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:36 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.hpp"

ostream &operator<<(ostream &out, Connection &con)
{
    out << " fd " + to_string(con.fd);
    out << " read_fd " << con.read_fd;
    out << " write_fd " << con.write_fd;
    out << " action " << to_string(con.action);
    out << " method " << to_string(con.req.header.method);
#if 0
    out << " buff <" + con.buff + ">";
    out << " respbuff <" + con.respbuff + ">";
#endif
    out << " <" + to_string(con.res.status) + "> " + (is_error(con.res.status) ? con.res.cause : "");
    return out;
}

ostream &operator<<(ostream &out, Location &loc)
{
    GLOG("src") << loc.src << endl;
    GLOG("dest") << loc.dest << endl;
    if (loc.index.length())
        GLOG("index") << loc.index << endl;
    if (loc.limit)
        GLOG("limit") << loc.limit << endl;
    if (loc.methods.size())
    {
        GLOG("methods") << (loc.methods[GET_] ? "GET " : "") << (loc.methods[POST_] ? "POST " : "") << (loc.methods[DELETE_] ? "DELETE" : "");
        out << endl;
    }
    int val = loc.autoindex;
    GLOG("autoindex") << (val == on    ? "on"
                         : val == off ? "off>"
                                      : "none")
                     << endl;
    if (loc.ret_location.length())
        GLOG("return") << to_string(loc.ret_status) + " " + loc.ret_location << endl;
    map<string, string>::iterator it;
    for (it = loc.cgi.begin(); it != loc.cgi.end(); it++)
        GLOG("cgi") << it->first << ", path: " << it->second << endl;
#if 0
    out << setw(space) << "errors:   " << endl;
    map<int, string>::iterator it_err;
    for (it_err = loc.errors.begin(); it_err != loc.errors.end(); it_err++)
        out << setw(space + 5) << "status: " << it_err->first << ", path: " << it_err->second << endl;
#endif
    return out;
}

ostream &operator<<(ostream &out, Server &serv)
{
    cout << CYAN "       listen: " RESET << serv.listen << ":" << serv.port << ", name: " << serv.name << endl;
    map<string, Location>::iterator it;
    for (it = serv.locations.begin(); it != serv.locations.end(); it++)
    {
        cout << CYAN "     location: " RESET << it->first << endl;
        out << it->second;
    }
    return out;
}

void see_pfds_size()
{
    GLOG("poll fds size") << pfds.size() << endl;
    // for (size_t i = 0; i < pfds.size(); i++)
    // {
    //     if (files.count(pfds[i].fd))
    //         cout << "FILE  : " << pfds[i].fd << endl;
    //     if (clients.count(pfds[i].fd))
    //         cout << "CLIENT: " << clients[pfds[i].fd].fd
    //              << " rfd: " << clients[pfds[i].fd].read_fd
    //              << " wfd: " << clients[pfds[i].fd].write_fd << endl;
    //     // if (cons.count(pfds[i].fd) && to_string(types[pfds[i].fd]) != "SERVER")
    //     //     cout << "   " << cons[pfds[i].fd] << endl;
    // }
    // cout << endl;
}