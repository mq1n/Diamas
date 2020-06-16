#pragma once
#include <string>
#include <istream>
#include <sstream>
#include <cstdint>

namespace net_engine
{
    static std::string GetLocalAddress()
    {
        asio::io_service service;
        asio::ip::tcp::resolver resolver(service);

        std::string hostName = asio::ip::host_name();

        auto endpoints = resolver.resolve({ hostName, "" });
        for (auto& endpoint : endpoints)
        {
            if (endpoint.endpoint().protocol() != asio::ip::tcp::v4())
                continue;  // the client currently only supports IPv4!

            return endpoint.endpoint().address().to_string();
        }

        return "127.0.0.1";
    }

    static uint32_t ConvertFromIP(const std::string& ip)
    {
        std::istringstream stream(ip);
        int32_t a, b, c, d;

        stream >> a;
        stream.ignore(1);
        stream >> b;
        stream.ignore(1);
        stream >> c;
        stream.ignore(1);
        stream >> d;

        uint32_t ret = 0;
        ret |= (d & 0xFF) << 24;
        ret |= (b & 0xFF) << 16;
        ret |= (c & 0xFF) << 8;
        ret |= (a & 0xFF) << 0;

        return ret;
    }

    static std::string ConvertToIP(const uint32_t ip)
    {
        uint8_t d = ip & 0xFF;
        uint8_t c = (ip >> 8) & 0xFF;
        uint8_t b = (ip >> 16) & 0xFF;
        uint8_t a = (ip >> 24) & 0xFF;

        std::ostringstream buffer;
        buffer << a << "." << b << "." << c << "." << d;
        return buffer.str();
    }
};
