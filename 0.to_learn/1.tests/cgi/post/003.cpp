#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
using namespace std;

int main(int argc, char **argv, char **env)
{
    int pid = fork();
    if (pid == 0)
    {
        int infd = open("in.txt", O_RDONLY, 0777);
        int oufd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (infd < 0 || oufd < 0)
        {
            cerr << "opening file" << endl;
            exit(-1);
        }
        std::vector<std::string> env;
        // env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        // env.push_back("PATH_INFO=/usr/bin/python3");
        // env.push_back("SERVER_PROTOCOL=HTTP/1.1");
        // env.push_back("QUERY_STRING=blah=blahlah");
        env.push_back("PATH_INFO=/usr/bin/php-cgi");
        env.push_back("REQUEST_METHOD=POST"); // required in php-cgi
        // env.push_back("SCRIPT_FILENAME=./test.py");
        env.push_back("SCRIPT_FILENAME=./test.php"); // required in php-cgi
        env.push_back("REDIRECT_STATUS=CGI"); // required in php-cgi
        env.push_back("SCRIPT_NAME=./test.php");
        env.push_back("CONTENT_LENGTH=15"); // required in php-cgi
        env.push_back("CONTENT_TYPE=application/x-www-form-urlencoded"); // required in php-cgi
        // env.push_back("=15");
        //SCRIPT_NAME,CONTENT_LENGTH

        std::vector<char *> envp(env.size() + 1);
        for (size_t i = 0; i < env.size(); i++)
            envp[i] = (char *)env[i].c_str();
        envp[env.size()] = NULL;

        if (dup2(infd, 0) < -1)
        {
            cerr << "dup2 failed 0" << endl;
            exit(-1);
        }
        if (dup2(oufd, 1) < -1)
        {
            cerr << "dup2 failed 1" << endl;
            exit(-1);
        }
        close(infd);
        close(oufd);
        char *arg[3];
        // arg[0] = (char *)"/usr/bin/python3";
        arg[0] = (char *)"/usr/bin/php-cgi";
        // arg[1] = (char *)"./test.py";
        arg[1] = (char *)"./test.php";
        arg[2] = NULL;
        execve(arg[0], arg, envp.data());
        cerr << "fork failed" << endl;
        exit(-1);
    }
    else if (pid < 0)
    {
        cerr << "fork failed" << endl;
    }
    int status = 1;
    if (waitpid(pid, &status, 0) > 0)
    {
        if (!WIFEXITED(status))
            cerr << "CGI failed" << endl;
    }
    cout << "program finished" << endl;
}