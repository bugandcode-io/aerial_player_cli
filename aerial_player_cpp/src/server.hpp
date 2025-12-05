#pragma once

#include <memory>

class Player;
class Playlist;

// Telnet-style raw TCP control: play, pause, next, etc.
void start_control_server(Player& player, std::shared_ptr<Playlist> playlist);

// HTTP control server for Postman/curl/etc. (default port 8080)
void start_http_server(Player& player, std::shared_ptr<Playlist> playlist, int port = 8080);
