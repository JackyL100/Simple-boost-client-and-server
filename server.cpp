//#include <asio.hpp>
//#include <asio/ts/buffer.hpp>
//#include <asio/ts/internet.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <set>
#include <map>
#include <memory>
#include <deque>
#include <list>
#include <boost/asio.hpp>
#include <cctype>
#include "packet.hpp"

using boost::asio::ip::tcp;

//compile command: g++ main.cpp -I /Users/sulanyu/Downloads/asio-1.22.1/include -std=c++11

class playerClient {
    public:
        virtual void deliver(const packet& pack) = 0;
};

typedef std::shared_ptr<playerClient> playerClient_ptr;

class room {
    private:
        //std::set<playerClient_ptr> players;
        enum {max_recent_pack = 100};
        std::deque<packet> packet_queue;
        packet setup;
    public:
        std::map<playerClient_ptr, int> players;
        void join(playerClient_ptr p) {
            std::cout << "a\n";
            players.insert({p,0});
        }
        void leave(playerClient_ptr p) {
            players.erase(p);
        } 
        void deliver(const packet pack) {
            packet_queue.push_back(pack);
            while (packet_queue.size() > 0) {
                packet_queue.pop_front();
                for (auto p : players) {
                    p.first->deliver(pack);
                }
            }
        }
};

class connection : public playerClient, public std::enable_shared_from_this<connection> {
private:
    tcp::socket socket_;
    room& room_;
    packet read_pack;
    std::deque<packet> write_pack;
    std::vector<char> vBuffer;
public:
  connection(tcp::socket socket, room& r)
    : socket_(std::move(socket)),
      room_(r)
  {
    vBuffer.resize(1024);
  }

  void start()
  {
    std::cout << "b\n";
    //room_.join(shared_from_this());
    //room_.players.insert_or_assign(shared_from_this(), 0);
    room_.players[shared_from_this()] = 0;
    std::cout << "joined" << &room_;
    //do_read_header();
    grabSomeData();
  }

  void deliver(const packet& msg)
  {
    write_pack.push_back(msg);
    bool write_in_progress = !write_pack.empty();
    if (write_in_progress)
    {
      do_write();
    }
  }

private:
void grabSomeData() {
    socket_.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), 
    [&](std::error_code ec, std::size_t length) {
        if (!ec) {
            /*for (int i = 0; i < length; i++) {
                std::cout << vBuffer[i];
            }*/
            using namespace std; 
            packet msg;
            msg.changeBodLength(length);
            string str(vBuffer.begin(), vBuffer.end());
            for (int i = 10; i < str.size(); i++) {
              if (isdigit(str[i]) != 0) {
                str.insert(str.begin() + i - 1, 1, '/');
              }
            }
            memcpy(msg.body(), vBuffer.data(), msg.getBodyLength());
            msg.encodeH();
            cout << "packet: " << msg.body() << "\n";
            // find way to add slash in between player packets
            vBuffer.erase(vBuffer.begin());
            if (strlen(msg.body()) == 11) {
              string s(msg.body());
              room_.players.insert_or_assign(shared_from_this(), std::stoi(s.substr(5)));
              cout << "assigned id: " << std::stoi(s.substr(5)) << " to player\n";
            } else {
              room_.deliver(msg);
            }
            grabSomeData();
        }
    });
}
  void do_write() {
    socket_.async_write_some(boost::asio::buffer(write_pack.front().data(), write_pack.front().length()), 
      [&](std::error_code ec, std::size_t length) {
          if (!ec) {
            write_pack.pop_front();
            std::cout << "sent message\n";
          }
        });
  }
};

typedef boost::shared_ptr<connection> connection_ptr;

class server {
    private:
        boost::asio::io_context& context;
        tcp::acceptor acceptor_;
        room room_;
    public:
        server(boost::asio::io_context& io_context, const tcp::endpoint& endpoint) : context(io_context), acceptor_(io_context, endpoint) {
            do_accept();
        }
        void do_accept() {
            acceptor_.async_accept(
                [this] (boost::system::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::cout << "accepting\n";
                        std::make_shared<connection>(std::move(socket), room_)->start();
                    }
                    do_accept();
                }
            );
        }
};

typedef boost::shared_ptr<server> server_ptr;

int main(int argc, char* argv[]) {
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_context io_context;

    std::list<server> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}