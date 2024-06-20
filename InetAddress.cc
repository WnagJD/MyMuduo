#include "InetAddress.h"
#include <string.h>


InetAddress::InetAddress(uint16_t port, std::string ip)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}


std::string InetAddress::toIp()const
{
    char buf[64]={0};
    ::inet_ntop(AF_INET, &addr_.sin_addr.s_addr,buf, sizeof(buf));
    return buf;
}
uint16_t InetAddress::toPort()const
{
    return ntohs(addr_.sin_port);
}
std::string InetAddress::toIpPort()const
{
    char buf[64]={0};
    ::inet_ntop(AF_INET, &addr_.sin_addr.s_addr,buf,sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end,":%u", port);
    return buf;
}

// #include <iostream>
// int main()
// {
//     InetAddress ine(8080);
//     std::cout<<ine.toIpPort()<<std::endl;
//     return 0;
// }