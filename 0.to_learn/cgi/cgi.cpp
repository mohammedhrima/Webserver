#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <wait.h>

using namespace std;

int main()
{
    string exec_path;
    string cgi_output;
    string cgi_filename;

    exec_path = "/usr/bin/python3";
    cgi_output = "out.txt";
    cgi_filename = "file.py";

    pid_t pid = fork();
    if (pid == 0)
    {
        int fd = open(cgi_output.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if (fd < 0)
            exit(1);
        vector<string> env;
        // env.push_back("QUERY_STRING=" + con.data.queries);
        // env.push_back("PATH_INFO=" + con.data.uri);
        env.push_back("REQUEST_METHOD=GET");
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");

        env.push_back("REMOTE_ADDR=127.0.0.1");
        env.push_back("SERVER_PORT=17000");
        env.push_back("SERVER_PROTOCOL=HTTP/1.1");
        env.push_back("SERVER_SOFTWARE=");
        env.push_back("SCRIPT_FILENAME=" + cgi_filename);
        env.push_back("REDIRECT_STATUS=CGI");
        vector<char *> envp(env.size() + 1);
        for (size_t i = 0; i < env.size(); i++)
            envp[i] = (char *)env[i].c_str();
        envp[env.size()] = NULL;

        cout << "has exec path " << exec_path << endl;
        cout << "execute       " << cgi_filename << endl;
        cout << "cgi output    " << cgi_output << endl;
        cout << "env: " << endl;
        for (size_t i = 0; i < envp.size(); i++)
            cout << envp[i] << endl;

        if (dup2(fd, 1) > 0)
        {
            char *arg[3];
            arg[0] = (char *)exec_path.c_str();
            arg[1] = (char *)cgi_filename.c_str();
            arg[2] = NULL;
            execve(exec_path.c_str(), arg, envp.data());
        }
        close(fd);
        exit(-1);
    }
    int status;
    int val = waitpid(pid, &status, 0);
    if (val < 0)
    {
        cout << "CGI failed" << endl;
    }
    else if (val > 0)
    {
    }
}