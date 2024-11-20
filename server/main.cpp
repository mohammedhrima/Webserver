/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhrima <mhrima@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 16:53:27 by mhrima            #+#    #+#             */
/*   Updated: 2024/06/07 16:53:27 by mhrima           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.hpp"

map<int, string> servers_addresses;
map<int, ssize_t> servers_ports;
map<int, Connection> clients;
map<int, int> files;

vector<pollfd> pfds;
map<ssize_t, vServer> servers;
map<string, string> memetype;
string machine_ip_address;

int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw Error(string("Invalid number of arguments"));
        set_machine_ip_address();
        parse_config(argv[1]);
        init_memetypes();
        WebServer();
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
}