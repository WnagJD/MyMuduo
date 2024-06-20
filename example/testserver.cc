#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>

#include <string>
#include <functional>

class EchoServer
{
public:
    EchoServer(EventLoop*loop, const InetAddress& listenaddr, const std::string& name)
            :loop_(loop)
            ,server_(loop,listenaddr,name)
            {
                //设置Tcpserver的回调函数

                server_.setConnectionCallback(std::bind(&EchoServer::onConnectionCallback, this, std::placeholders::_1));
                server_.setMessageCallback(std::bind(&EchoServer::onMessageCallback, this,std::placeholders::_1,
                                                        std::placeholders::_2,std::placeholders::_3));

                //设置底层subloop的线程数
                server_.setThreadNums(3);
            }
   
    void start()
    {
        server_.start();
    }
private:
    void onConnectionCallback(const TcpConnectionPtr& conn)
    {
        if(conn->connected())
        {
            LOG_INFO("sss");
            LOG_INFO("Connection UP : %s", conn->peerAddr().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN :%s", conn->peerAddr().toIpPort().c_str());
        }
    }

    void onMessageCallback(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp receivetime)
    {
        std::string data = buf->retrieveAllAsString();
        conn->send(data);
        //主动关闭服务端的连接
        conn->shutdwon();
    }


    EventLoop* loop_;
    TcpServer server_;
};


int main()
{
    EventLoop loop;
    InetAddress listenaddr(8000);
    EchoServer testserver(&loop,listenaddr,"TestServer-01");
    testserver.start();
    loop.loop();

    return 0;
}