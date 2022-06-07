#include <iostream>
#include <boost/asio.hpp>
#include <boost/system.hpp>
#include <string_view>
#include <deque>
#include <vector>
#include "packet.hpp"

using boost::asio::ip::tcp;

typedef std::deque<packet> chat_message_queue;

class chat_client
{
public:
  bool cont_run;
  std::string received;
  chat_client(boost::asio::io_context& io_context,
      tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    vBuffer.resize(1024);
    do_connect(endpoints);
    cont_run = true;
  }

  void write(const packet& msg)
  {
    boost::asio::post(io_context_, 
      [this,msg] () {
        bool writeProgress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!writeProgress) {
          do_write();
        }
      });
  }

  void close()
  {
    std::cout << "socket closed\n";
    boost::asio::post(io_context_, [this](){socket_.close();});
  }

private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    boost::asio::async_connect(socket_, endpoints,
        [this](boost::system::error_code ec, tcp::endpoint)
        {
          if (!ec)
          { 
            std::cout << "connected\n";
            grabSomeData();
          }
        });
  }

  void grabSomeData() {
    socket_.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), 
    [&](std::error_code ec, std::size_t length) {
        if (!ec) {
            //std::string str(vBuffer.begin(),vBuffer.end());
            //std::cout << str;
            std::string_view str(vBuffer.data(), vBuffer.size());
            received = str;
            cont_run = true;
            grabSomeData();
        }
    });
  }

  void do_write()
  {
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
              std::cout << "wah\n";
              do_write();
            }
          }
          else
          {
            socket_.close();
          }
        });
  }

  void do_close() {socket_.close();}

private:
  boost::asio::io_context& io_context_;
  tcp::socket socket_;
  packet read_msg_;
  chat_message_queue write_msgs_;
  std::vector<char> vBuffer;
};