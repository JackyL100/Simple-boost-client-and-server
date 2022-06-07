#include <cstdio>
#include <cstring>
#include <cstdlib>
#pragma once
class packet {
    public:
        static constexpr std::size_t headerLength = 4;
        static constexpr std::size_t maxBodLength = 512;
        packet() : bodyLength(0) {}
        char* data () {
            return stuff;
        }
        char* body() {
            return stuff + headerLength;
        }
        size_t length() {
            return headerLength + bodyLength;
        }
        size_t getBodyLength() {
            return bodyLength;
        }
        void changeBodLength(size_t newLength) {
            bodyLength = newLength;
            if (newLength > maxBodLength) {
                bodyLength = maxBodLength;
            }
        }
        bool decodeH() {
            char header[headerLength + 1] = "";
            std::strncat(header, stuff, headerLength);
            bodyLength = std::atoi(header);
            if (bodyLength > maxBodLength) {
                bodyLength = 0;
                return false;
            }
            return true;
        }
        void encodeH() {
            char header[headerLength + 1] = "";
            std::sprintf(header, "%4d", bodyLength);
            memcpy(stuff, header, headerLength);
        }
        packet(std::string msg) {
            std::memcpy(this->body(), msg.c_str(), msg.length());
        }
    private:
        char stuff[headerLength + maxBodLength];
        size_t bodyLength;
        
};