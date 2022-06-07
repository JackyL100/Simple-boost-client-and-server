#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "client_connection.hpp"
#include <random>
#include <string>
#include <string_view>
#include <charconv>
#include <map>
#include <vector>
#include "other_player.hpp"

const int HEIGHT = 640;
const int WIDTH = 960;

int randNum(int const &min, int const &max) {
    std::random_device randDev;
    std::mt19937 rng(randDev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(min,max);
    return dist6(rng);
}

class player {
    public:
        chat_client client;
        SDL_Rect hitbox;
        int id;
        int velo;
        SDL_Scancode keys[4];
        std::vector<std::string> received;
        std::map<int, other_player> others;
        player(int x, int y, int w, int h, boost::asio::io_context& io_cont, tcp::resolver::results_type& endpoints) : client(io_cont, endpoints) {
            hitbox.x = x;
            hitbox.y = y;
            hitbox.w = w;
            hitbox.h = h;
            do {
                for (int i = 1; i < 6; i++) {
                    id += randNum(0,9) * pow(10, i);
                }  
            } while (id < 99999);
            keys[0] = SDL_SCANCODE_W;
            keys[1] = SDL_SCANCODE_S;
            keys[2] = SDL_SCANCODE_A;
            keys[3] = SDL_SCANCODE_D;
            velo = 5;
        }
        std::string move(const Uint8 *keyState, SDL_Renderer *target) {
            if (keyState[keys[0]]) {
                if (hitbox.y > velo) {
                    hitbox.y -= velo;
                    return "w";
                } 
            } else if (keyState[keys[1]]) {
                if (hitbox.y < HEIGHT - hitbox.h) {
                    hitbox.y += velo;
                    return "s";
                }
            } else if (keyState[keys[2]]) {
                if (hitbox.x > velo) {
                    hitbox.x -= velo;
                    return "a";
                }
            } else if (keyState[keys[3]]) {
                if (hitbox.x < WIDTH - hitbox.w) {
                    hitbox.x += velo;
                    return "d";
                }
            }
            return "n";
        }
        void renderOthers(SDL_Renderer* target) {
            for (int i = 0; i < received.size(); i++) {
                std::string_view view(received[i].c_str(), 7);
                int id;
                std::from_chars(view.data(), view.data() + 7, id);
                if (id == this->id) {
                    continue;
                }
                if (others.find(id) == others.end()) {
                    others[id];
                } else {
                    char move = received[i][7]; // whatever char pos move starts at
                    switch(move) {
                        case 'a':
                            others[id].hitbox.x -= velo;
                            break;
                        case 'd':
                            others[id].hitbox.x += velo;
                            break;
                        case 'w':
                            others[id].hitbox.y -= velo;
                            break;
                        case 's':
                            others[id].hitbox.y += velo;
                            break;
                    }
                    SDL_RenderDrawRect(target, &others[id].hitbox);
                    SDL_RenderFillRect(target, &others[id].hitbox);
                }
            }
        }
    private: 
        void parse() {
            std::string readIn = client.received;
            int pos;
            while ((pos = readIn.find("/")) != std::string::npos) {
                received.push_back(readIn.substr(0,pos));
                readIn.erase(0,pos + 1);
            }
            
        }
};