#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<err.h>
#include<iostream>

#define MCAST_PORT 8888
#define MCAST_ADDR "224.0.0.88" /*一个局部连接多播地址，路由器不进行转发*/
#define MCAST_INTERVAL 1  //发送时间间隔
#define BUFF_SIZE 256   //接收缓冲区大小
using namespace std;

int main(int argc, char const *argv[])
{
    int sock;
    struct sockaddr_in local_addr;
    int err = -1;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        cout << "sock error" << endl;
        return -1;   
    }

    /*初始化地址*/
    local_addr.sin_family=AF_INET;
    local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    local_addr.sin_port=htons(MCAST_PORT);

    /*设置端口复用*/
    int reusePort = 1;
    err = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&reusePort, sizeof(reusePort));
	if (err < 0) {
        cout << "setsockopt failed: " << errno << endl;
    }

    /*绑定socket*/
    err = bind(sock,(struct sockaddr*)&local_addr,sizeof(local_addr));
    if(err<0) {
        cout<<"bind error"<<endl;
        return -2;
    }

    /*设置回环许可*/
    int loop=1;
    err = setsockopt(sock,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop));
    if(err<0) {
        cout<<"set sock error"<<endl;
        return -3;
    }
    
    struct ip_mreq mreq;/*加入广播组*/
    mreq.imr_multiaddr.s_addr=inet_addr(MCAST_ADDR);//广播地址
    mreq.imr_interface.s_addr=htonl(INADDR_ANY); //网络接口为默认
    /*将本机加入广播组*/
    err = setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));
    if(err<0)
    {
            cout<<"set sock error"<<endl;
            return -4;
    }

    int times=0;
    socklen_t addr_len=0;
    char buff[BUFF_SIZE];
    int n=0;
    /*循环接受广播组的消息，5次后退出*/
    for(times=0;;times++)
    {
            addr_len=sizeof(local_addr);
            memset(buff,0,BUFF_SIZE);
            n=recvfrom(sock,buff,BUFF_SIZE,0,(struct sockaddr*)&local_addr,&addr_len);
            if(n==-1)
            {
                    cout<<"recv error"<<endl;
                    return -5;
            }
            /*打印信息*/
            printf("RECV %dst message from server : %s\n",times,buff);
            sleep(MCAST_INTERVAL);
    }

    /*退出广播组*/
    err=setsockopt(sock,IPPROTO_IP,IP_DROP_MEMBERSHIP,&mreq,sizeof(mreq));
    close(sock);
    return 0;
}
