#include <iostream>

#include "Server.h"

using namespace std;

Server::Server() {
    //初始化服务器地址和端口
    serverAddr.sin_family=PF_INET;
    serverAddr.sin_port= htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr= inet_addr(SERVER_IP);
    //初始化socket
    listener=0;

    //epoll fd
    epfd=0;
}

void Server::Init() {
    cout<<"init server..."<<endl;
    //step 1 : 创建监听socket
    //使用socket
    listener = socket(PF_INET,SOCK_STREAM,0);
    if (listener<0){
        perror("listener");
        exit(-1);
    }

    //step 2 ：绑定地址
    //使用bind（）
    if (bind(listener,(struct sockaddr *)&serverAddr,sizeof(serverAddr))<0){
        perror("bind error");
    }

    //step 3 ：监听连接
    //使用listen（）
    int ret = listen(listener,5);
    if (ret<0){
        perror("listen error");
        exit(-1);
    }
    cout<<"start to listen: "<<SERVER_IP<<endl;

    //step 4 ：创建事件表
    //epoll_create
    epfd = epoll_create(EPOLL_SIZE);

    if (epfd<0){
        perror("epfd error");
        exit(-1);
    }

    //step 5 ：添加监听fd到epoll fd
    //addfd（）
    addfd(epfd,listener, true);
}

void Server::Close() {
    close(listener);
    close(epfd);
}

int Server::SendBroadcastMessage(int clientfd) {
    //buf[BUF_SIZE] 接收新消息
    //message[BUF_SIZE] 保存格式化的消息

    char buf[BUF_SIZE],message[BUF_SIZE];
    bzero(buf,BUF_SIZE);
    bzero(message,BUF_SIZE);

    //step 1 : 接受新消息
    //recv（）
    cout<<"read from client(clientID = "<<clientfd<<")";
    int len = recv(clientfd,buf,BUF_SIZE,0);

    //step 2 ：判断是否是客户端终止连接
    if (len == 0){
        close(clientfd);

        clients_list.remove(clientfd);
        cout<<"clientID = "<<clientfd
            <<" closed.\n now there are "
            <<clients_list.size()
            <<" client in the chat room"
            <<endl;
    }

    //step 3 ：判断是否聊天室还有其他客户端
    else{
        if (clients_list.size()==1){
            send(clientfd,CAUTION, strlen(CAUTION),0);
            return len;
        }
        sprintf(message,SERVER_MESSAGE,clientfd,buf);

        list<int>::iterator it;
        for (it==clients_list.begin();it!=clients_list.end();++it) {
            if (*it!=clientfd){
                if (send(*it,message,BUF_SIZE,0)<0){
                    return -1;
                }
            }
        }
        return len;
    }

    //step 4 ：格式化发送的消息内容
    //springtf（）

    //step 5 ：遍历客户端列表依次发送消息
    //send（）

}

void Server::Start() {
    //step 1 :初始化服务端
    //init（）
    static struct epoll_event events[EPOLL_SIZE];
    Init();

    //step 2 ：进入主循环

    while (1){
        //epoll_events_count表示就绪事件的数目
        //step 3 ：获取就绪的事件
        //epoll_wait（）
        int epoll_events_count = epoll_wait(epfd,events,EPOLL_SIZE,-1);

        if (epoll_events_count<0){
            perror("error failure");
            break;
        }
        cout<<"epoll_events_count = \n"<<epoll_events_count<<endl;

        //step 4 ：循环处理所有就绪事件
        //4.1如果是新连接则接受连接并将连接添加到epoll fd
        //accept（） addfd（）
        for (int i = 0; i < epoll_events_count; ++i) {
            int sockfd = events[i].data.fd;
            if (sockfd == listener){
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(listener,(struct sockaddr *)&client_address,&client_addrLength);

                cout << "client connection from: "
                     << inet_ntoa(client_address.sin_addr)<<":"
                     <<ntohs(client_address.sin_port)<<",clientfd = "
                     <<clientfd<<endl;

                addfd(epfd,clientfd, true);

                clients_list.push_back(clientfd);
                cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
                cout << "Now there are " << clients_list.size() << " clients int the chat room" << endl;

                cout<<"welcome message"<<endl;
                char message[BUF_SIZE];
                bzero(message,BUF_SIZE);
                sprintf(message,SERVER_WELCOME,clientfd);
                int ret = send(clientfd,message,BUFSIZ,0);
                if (ret<0){
                    perror("send error");
                    Close();
                    exit(-1);
                }
            } else{
                //4.2 如果是新消息则广播给其他客户端
                //SendBroadcastMessage
                int ret = SendBroadcastMessage(sockfd);
                if (ret<0){
                    perror("error");
                    Close();
                    exit(-1);
                }
            }

        }
    }
    Close();
}