#include "header.hpp"

map<int, int> pairs;
map<int, Connection> cons;
vector<pollfd> pfds;
map<size_t, vServer> servers;
map<string, string> memetype;
string machine_ip_address;
map<int, Type> types;

// TODO: ulimit is 63282
// TODO: handle this case siege -t 60s -b http://127.0.0.1:17000
int main(int argc, char **argv)
{
    try
    {
        if (argc != 2)
            throw Error(string("Invalid number of arguments"));
        set_machine_ip_address();
        mServers mservs = parse_config(argv[1]);
        init_memetypes();
        WebServer(mservs);
    }
    catch (exception &err)
    {
        cerr << RED "Error: " << err.what() << RESET << endl;
    }
}