#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include <string>
#include "Common.h"

using namespace std;

class Server{
public:
    Server();

    void Init();

    void Close();

    void Start();

private:
    int SendBroadcastMessage(int clientfd);

    //服务器端serverAddr信息；
    struct sockaddr_in serverAddr;

    //创建监听的socket
    int listener;

    //epoll_create创建后的返回值
    int epfd;

    //客户端列表
    list<int> clients_list;
};

#endif