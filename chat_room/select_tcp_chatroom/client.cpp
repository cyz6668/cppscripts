#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#define ARGS_CHECK(argc, val) {if (argc!=val){printf("Args ERROR\n"); return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0) \
        {printf("%s:%s\n",funcName,strerror(ret));return -1;}}
#define ERROR_CHECK(ret, retval, filename) {if (ret == retval){perror(filename); return -1;}}  
#include <iostream>
using namespace std;

int main()
{
    int ret;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(sockfd, -1, "socket");
    struct sockaddr_in sockinfo;
    bzero(&sockinfo, sizeof(sockinfo));
    sockinfo.sin_addr.s_addr =  htonl(INADDR_ANY);  
    sockinfo.sin_port = htons(5005);
    sockinfo.sin_family = AF_INET;
    ret = connect(sockfd, (sockaddr *)&sockinfo, sizeof(sockinfo));
    ERROR_CHECK(ret, -1, "connect");
    cout << "connect succuss" << endl;

    int epfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);//注册 键盘输入信号
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);//注册 socket
    while (1)
    {
        struct epoll_event evs[2];
        int nfds = epoll_wait(epfd, evs, 2, 3000);
        ERROR_CHECK(nfds, -1, "epoll_wait");
        if (nfds > 0)
        {
            for (int i = 0; i < nfds; i++)
            {
                if (evs[i].data.fd == STDIN_FILENO)
                {
                    char buf[100] = {0};
                    read(0, buf, sizeof(buf));
                    write(sockfd, buf, strlen(buf) - 1);
                }
                if (evs[i].data.fd == sockfd)
                {
                    char buf[100] = {0};
                    ret = read(sockfd, buf, sizeof(buf));
                    if (ret == 0)
                    {
                        cout << "bye" << endl;
                        goto chatOver;
                    }
                    cout << buf << endl;
                }
            }
        }
        else //nfds == 0
            cout << "timeout!" << endl;
    }
chatOver:
    close(sockfd);
    close(epfd);
    return 0;
}
