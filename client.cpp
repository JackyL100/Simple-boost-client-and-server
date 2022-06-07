#include <boost/asio.hpp>
#include <boost/system.hpp>
#include <iostream>
#include <deque>
#include <cstdlib>
#include <thread>
#include <string>
#include <set>
#include "packet.hpp"
//#include "client_connection.hpp"
#include "client.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//g++ client.cpp -L /usr/local/Cellar/boost/1.78.0_1/include -L/usr/local/Cellar/sdl2/2.0.20/include -lSDL2 -lSDL2_image -std=c++11 -o client.out

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: chat_client <host> <port>\n";
    return 1;
  }
  boost::asio::io_context context;

  tcp::resolver resolver(context);
  auto endpoints = resolver.resolve(argv[1], argv[2]);
  player bear(0,0,40,40,context,endpoints);
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> idleWork = boost::asio::make_work_guard(context);
  std::thread t([&context](){context.run();});
  
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window *window = SDL_CreateWindow("Client", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 860, 640, SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Renderer *renderTarget = nullptr;
  if (window == NULL) {std::cout << "could not create window: " << SDL_GetError() << "\n"; return 1;}
  SDL_Event windowEvent;
  renderTarget = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Texture *texture = nullptr;
  SDL_Surface *surface = nullptr;
  std::string filePath = "bigBg.jpg";
  surface = IMG_Load(filePath.c_str());
  if (surface == nullptr) {
    std::cout << "couldn't load surface " << SDL_GetError() << "\n";
  } else {
    texture = SDL_CreateTextureFromSurface(renderTarget, surface);
    SDL_FreeSurface(surface);
    surface = nullptr;
    if (texture == nullptr) {std::cout << "couldn't load texture " << SDL_GetError() << "\n";}
  }
  const Uint8* keyState;
  bool isRunning = true;
  bear.client.write(packet(std::to_string(bear.id)));
  while (isRunning) {
    if (SDL_PollEvent(&windowEvent)) {
      if (windowEvent.type == SDL_QUIT) {
        isRunning = false;
      } else if (windowEvent.type == SDL_KEYDOWN) {
        keyState = SDL_GetKeyboardState(NULL);
        packet msg;
        std::string input;
        input = std::to_string(bear.id);
        input += "/";
        input += bear.move(keyState, renderTarget);
        msg.changeBodLength(strlen(input.c_str()));
        memcpy(msg.body(), input.c_str(), msg.getBodyLength());
        msg.encodeH();
        bear.client.write(msg);
        //bear.client.cont_run = false;
        //while (!bear.client.cont_run) {}
      }
     }
     SDL_RenderClear(renderTarget);
     SDL_SetRenderTarget(renderTarget, texture);
     SDL_RenderCopy(renderTarget, texture, NULL, NULL);
     SDL_RenderDrawRect(renderTarget, &bear.hitbox);
     SDL_RenderFillRect(renderTarget, &bear.hitbox);
     bear.renderOthers(renderTarget);
     SDL_RenderPresent(renderTarget);
  }
  /*
  boost::asio::io_context context;

  tcp::resolver resolver(context);
  auto endpoints = resolver.resolve(argv[1], argv[2]);
  chat_client client(context, endpoints);
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> idleWork = boost::asio::make_work_guard(context);
  std::thread t([&context](){context.run();});

  char line[packet::maxBodLength + 1];
  while (true)
  {
    std::cin >> line;
    using namespace std; // For strlen and memcpy.
    packet msg;
    msg.changeBodLength(strlen(line));
    memcpy(msg.body(), line, msg.getBodyLength());
    msg.encodeH();
    client.write(msg);
    client.cont_run = false;
    while (!client.cont_run) {}
    std::cout << "\n";
  }*/
  bear.client.close();
  context.stop();
  t.join();
  SDL_DestroyRenderer(renderTarget);
  SDL_DestroyWindow(window);
  SDL_DestroyTexture(texture);
  renderTarget = nullptr;
  window = nullptr;
  texture = nullptr;
  SDL_Quit();
  return EXIT_SUCCESS;
}