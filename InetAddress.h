#pragma once
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>

class InetAddress
{
    public:
        explicit InetAddress(uint16_t port=0, std::string ip="127.0.0.1");
        explicit InetAddress(struct sockaddr_in &addr):addr_(addr){};

        std::string toIp()const;
        uint16_t toPort()const;
        std::string toIpPort()const;

        const sockaddr_in* getSockAddr()const{return & addr_;}
        void SetScokaddr(const sockaddr_in & addr){addr_ = addr;}



    private:
        // std::string ip_;
        // int16_t port_;
        struct sockaddr_in addr_;
};
