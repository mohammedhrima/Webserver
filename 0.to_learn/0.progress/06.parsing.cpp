#include <iostream>
#include <stdbool.h>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <vector>
#include <map>
#include <stdbool.h>
#include <iomanip>

typedef struct Server Server;

struct Server
{
    std::map<std::string, std::string> Data;
    // std::string listen;
    size_t port;

    std::map<long, std::string> errors;
    std::map<std::string, bool> methods;
    size_t limit;
    // bool is_redirection;
    // std::string return_; // TODO: replace it with Server*

    std::vector<Server *> children;
    Server()
    {
        std::cout << "call constractor" << std::endl;
        port = 80;
        limit = 0;
        Data["NAME"] = Data["LISTEN"] = Data["ROOT"] =
            Data["INDEX"] = Data["LOCATION"] = Data["UPLOAD"] = "",
        methods["GET"] = methods["POST"] = methods["DELETE"] = false;
    }

    ~Server()
    {
        std::cout << "call destractor" << std::endl;
        for (int i = 0; i < children.size(); i++)
            delete children[i];
        Data.clear();
        errors.clear();
        children.clear();
        methods.clear();
    }
};

std::string text;
std::vector<std::string> tokens;

void Tokenize()
{
    int i = 0;
    while (text[i])
    {
        if (text[i] == ' ')
        {
            i++;
            continue;
        }
        if (text[i] && std::strchr("#{}\n", text[i]))
        {
            tokens.push_back(text.substr(i, 1));
            i++;
            continue;
        }
        int j = i;
        while (text[i] && !std::strchr("#{} \n", text[i]))
            i++;
        if (i > j)
            tokens.push_back(text.substr(j, i - j));
    }
}

bool isAllDigits(const std::string &str)
{
    for (size_t i = 0; i < str.length(); i++)
        if (!isdigit(str[i]))
            return false;
    return true;
}

std::vector<std::string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> vec;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        vec.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    vec.push_back(str.substr(start));
    return vec;
}

int i = 0;
Server *parse_config_file()
{
    Server *serv = new Server();
    std::string keyword;
    if (tokens[i] != std::string("{"))
    {
        delete serv;
        throw std::string("Expected '{");
    }
    i++;
    while (i < tokens.size() && tokens[i] != "}")
    {
        if (tokens[i] == std::string("\n"))
        {
            i++;
            continue;
        }
        if (tokens[i] == std::string("#"))
        {
            while (tokens[i] != std::string("\n") && i < tokens.size())
                i++;
            continue;
        }
#if 0
        std::string arr[] = {"NAME", "LISTEN", "ROOT", "INDEX", "LOCATION", "UPLOAD"};
        int j = 0;
        while (j < sizeof(arr) / sizeof(arr[0]))
        {
            std::cout << "loop " << arr[j] << std::endl;
            if (std::string(arr[j]) == tokens[i])
            {
                i++;
                if (tokens[i] == std::string("#") || tokens[i] == std::string("\n"))
                {
                    // delete serv;
                    throw std::string("Invalid value ") + tokens[i];
                }
                serv->Data.insert(std::pair<std::string, std::string>(arr[j], tokens[i]));
                i++;
                // serv->Data[arr[j]] = tokens[i++];
                break;
            }
            j++;
        }
        if (j != sizeof(arr) / sizeof(arr[0]))
            continue;
#else
        std::map<std::string, std::string>::iterator it;
        // TODO: check if hostname doesn't exists, throw error
        for (it = serv->Data.begin(); it != serv->Data.end(); ++it)
        {
            if (it->first == tokens[i])
            {
                i++;
                // TODO: protect it
                serv->Data[it->first] = tokens[i++];
                if (it->first == std::string("LISTEN"))
                {
                    std::cout << "found" << std::endl;
                    std::vector<std::string> vec = splitString(serv->Data[it->first], ':');
                    if (vec.size() > 2)
                    {
                        delete serv;
                        throw std::string("Invalid hostname");
                    }
                    if (vec.size() == 2)
                    {
                        if (!isAllDigits(vec[1]))
                        {
                            delete serv;
                            throw std::string("Invalid port");
                        }
                        serv->port = std::atol(vec[1].c_str());
                        serv->Data["LISTEN"] = vec[0];
                    }
                }
                break;
            }
        }
        if (it != serv->Data.end())
            continue;
#endif
        if (tokens[i] == std::string("METHODS"))
        {
            i++;
            while (tokens[i] != std::string("\n") && i < tokens.size())
            {
                if (tokens[i] == std::string("POST") || tokens[i] == std::string("GET") ||
                    tokens[i] == std::string("DELETE"))
                    serv->methods[tokens[i]] = true;
                else
                {
                    delete serv;
                    throw std::string("Invalid method ") + tokens[i];
                }
                i++;
            }
            continue;
        }
        if (tokens[i] == std::string("ERRORS"))
        {
            i++;
            if (tokens[i] != std::string("{"))
            {
                delete serv;
                throw std::string("Expected '{'");
            }
            i++;
            while (i < tokens.size() && tokens[i] != std::string("}"))
            {
                if (tokens[i] == std::string("\n"))
                {
                    i++;
                    continue;
                }
                else if (isAllDigits(tokens[i]))
                {
                    long status = std::atol(tokens[i].c_str());
                    if (status < 400 || status > 499)
                    {
                        delete serv;
                        throw std::string("Invalid error status");
                    }
                    i++;
                    serv->errors[status] = tokens[i++];
                    continue;
                }
                else
                {
                    delete serv;
                    throw std::string("Unexpected ") + tokens[i];
                }
            }
            if (tokens[i] != std::string("}"))
            {
                delete serv;
                throw std::string("Expected '}'");
            }
            i++;
            continue;
        }
        if (tokens[i] == std::string("{"))
        {
            try
            {
                Server *child = parse_config_file();
                serv->children.push_back(child);
            }
            catch (...)
            {
                delete serv;
                throw;
            }
            continue;
        }
        if (i < tokens.size())
        {
            delete serv;
            throw std::string("Unexpected ") + tokens[i];
        }
    }
    if (tokens[i] != std::string("}"))
    {
        delete serv;
        throw std::string("Expected '}");
    }
    i++;
    return serv;
}

void pserver(Server *serv, int space)
{
    if (serv == NULL)
        throw std::string("serv in NULL");
    std::map<std::string, std::string>::iterator it1;
    for (it1 = serv->Data.begin(); it1 != serv->Data.end(); it1++)
    {
        if (it1->second.length())
            std::cout << std::setw(space) << it1->first << " : <" << it1->second << ">" << std::endl;
    }
    if (serv->methods.size())
    {
        std::cout << std::setw(space + 3) << "METHODS : ";
        std::map<std::string, bool>::iterator it2;
        for (it2 = serv->methods.begin(); it2 != serv->methods.end(); it2++)
        {
            if (it2->second)
                std::cout << it2->first << ", ";
        }
        std::cout << std::endl;
    }
    if (serv->errors.size())
    {
        std::cout << std::setw(space + 3) << "ERRORS : " << std::endl;
        std::map<long, std::string>::iterator it3;
        for (it3 = serv->errors.begin(); it3 != serv->errors.end(); it3++)
        {
            if (it3->second.length())
                std::cout << std::setw(space + 6) << it3->first << " : <" << it3->second << ">" << std::endl;
        }
    }
    std::cout << std::setw(space + 3) << "PORT : " << serv->port << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < serv->children.size(); i++)
        pserver(serv->children[i], space + 8);
}

int main(int argc, char **argv)
{
    std::vector<Server *> servs;
    try
    {
        if (argc != 2)
            throw std::string("Invalid argument");
        std::ifstream file(argv[1]);
        if (!file.is_open())
            throw std::string("Failed to open file");
        text = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        Tokenize();
        while (i < tokens.size())
        {
            if (tokens[i] == std::string("\n"))
            {
                i++;
                continue;
            }
            servs.push_back(parse_config_file());
        }
        i = 0;
        while (i < servs.size())
        {
            pserver(servs[i], 10);
            delete servs[i];
            i++;
            std::cout << "=======================================================" << std::endl;
        }
        servs.clear();
    }
    catch (std::string &err_msg)
    {
        std::cerr << "\033[31mError: " << err_msg << "\033[0m" << std::endl;
    }
}
