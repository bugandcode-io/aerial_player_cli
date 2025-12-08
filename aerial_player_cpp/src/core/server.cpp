#include "Server.hpp"
#include "Player.hpp"
#include "Playlist.hpp"
#include "UI.hpp"
#include "DB.hpp"

#include <thread>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
static const socket_t INVALID_SOCKET_FD = INVALID_SOCKET;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using socket_t = int;
static const socket_t INVALID_SOCKET_FD = -1;
#endif

static void close_socket(socket_t s)
{
#ifdef _WIN32
    if (s != INVALID_SOCKET_FD)
        closesocket(s);
#else
    if (s != INVALID_SOCKET_FD)
        close(s);
#endif
}

static std::string trim(const std::string &s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
    {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
    {
        --end;
    }
    return s.substr(start, end - start);
}

// ===================== TCP (telnet-style) =====================

static void handle_tcp_client(socket_t client,
                              Player &player,
                              std::shared_ptr<Playlist> playlist,
                              PlayDatabase *db)
{
    const char *welcome =
        "Aerial TCP Control\n"
        "Commands: play, pause, resume, next, prev, ff, rew, stop, quit\n";

    send(client, welcome, static_cast<int>(std::strlen(welcome)), 0);

    char buf[1024];
    std::string pending;
    bool running = true;

    while (running)
    {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0)
            break; // client closed or error

        pending.append(buf, n);

        size_t pos;
        while ((pos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);

            line = trim(line);
            if (line.empty())
                continue;

            std::string lower = line;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

            std::ostringstream reply;

            if (lower == "play")
            {
                player.playCurrent();
                reply << "OK play\r\n";

                if (db && playlist && !playlist->empty())
                {
                    db->logPlay(playlist->current());
                }
            }
            else if (lower == "pause")
            {
                player.pause();
                reply << "OK pause\r\n";
            }
            else if (lower == "resume")
            {
                player.resume();
                reply << "OK resume\r\n";
            }
            else if (lower == "next")
            {
                std::string prevTrack;

                // Capture what was playing *before* skipping
                if (db && playlist && !playlist->empty())
                {
                    prevTrack = playlist->current();
                }

                player.playNext();
                reply << "OK next\r\n";

                if (db && playlist && !playlist->empty())
                {
                    if (!prevTrack.empty())
                    {
                        db->logSkip(prevTrack); // moved away from this track
                    }
                    db->logPlay(playlist->current()); // new current track
                }
            }
            else if (lower == "prev" || lower == "previous")
            {
                player.playPrevious();
                reply << "OK prev\r\n";

                if (db && playlist && !playlist->empty())
                {
                    db->logPlay(playlist->current()); // log the track we jumped back to
                }
            }

            else if (lower == "ff")
            {
                player.seekBy(10.0);
                reply << "OK ff +10s\r\n";
            }
            else if (lower == "rew")
            {
                player.seekBy(-10.0);
                reply << "OK rew -10s\r\n";
            }
            else if (lower == "stop")
            {
                player.stop();
                reply << "OK stop\r\n";
            } 
            else if(lower == "status"){
                if (lower != "quit" && lower != "exit") {
    std::string nowPath;
    std::string nextPath;

    if (!playlist->empty()) {
        nowPath  = playlist->current();
        nextPath = playlist->peekNext();
    }

    reply << renderNowPlayingBoxPlain(nowPath, nextPath);

    double posSeconds = player.getPositionSeconds();
    reply << renderProgressBar(posSeconds);

    // Optional: show volume too
    reply << "\r\nVolume: " << player.getVolumePercent() << "%\r\n";
}

            }
            else if (lower == "quit" || lower == "exit")
            {
                reply << "Bye\r\n";
                running = false;
            }
            else
            {
                reply << "ERR unknown command: " << line << "\r\n";
            }

            // 🔹 Append Now Playing / Up Next box for non-quit commands
            if (lower != "quit" && lower != "exit" && lower != "status")
            {

                std::string nowPath;
                std::string nextPath;

                if (!playlist->empty())
                {
                    nowPath = playlist->current();
                    nextPath = playlist->peekNext();
                }

                reply << renderNowPlayingBoxPlain(nowPath, nextPath);
                // Progress bar
                double posSeconds = player.getPositionSeconds();
                reply << renderProgressBar(posSeconds);
            }

            std::string out = reply.str();
            if (!out.empty())
            {
                send(client, out.c_str(), static_cast<int>(out.size()), 0);
            }
        }
    }

    close_socket(client);
}

void start_control_server(Player &player, std::shared_ptr<Playlist> playlist, PlayDatabase *db)
{
    std::thread([&player, playlist, db]()
                {
#ifdef _WIN32
                    WSADATA wsaData;
                    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
                    if (wsaInit != 0)
                    {
                        std::cerr << "[TCP] WSAStartup failed: " << wsaInit << "\n";
                        return;
                    }
#endif

                    socket_t serverSock = socket(AF_INET, SOCK_STREAM, 0);
                    if (serverSock == INVALID_SOCKET_FD)
                    {
                        std::cerr << "[TCP] Failed to create socket\n";
#ifdef _WIN32
                        WSACleanup();
#endif
                        return;
                    }

                    int opt = 1;
                    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR,
                               reinterpret_cast<char *>(&opt), sizeof(opt));

                    sockaddr_in addr{};
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(5050);
                    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

                    if (bind(serverSock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
                    {
                        std::cerr << "[TCP] bind failed\n";
                        close_socket(serverSock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return;
                    }

                    if (listen(serverSock, 4) < 0)
                    {
                        std::cerr << "[TCP] listen failed\n";
                        close_socket(serverSock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return;
                    }

                    std::cout << "[TCP] Listening on 127.0.0.1:5050\n";

                    while (true)
                    {
                        sockaddr_in clientAddr{};
#ifdef _WIN32
                        int clientLen = sizeof(clientAddr);
#else
                        socklen_t clientLen = sizeof(clientAddr);
#endif
                        socket_t clientSock = accept(serverSock,
                                                     reinterpret_cast<sockaddr *>(&clientAddr),
                                                     &clientLen);
                        if (clientSock == INVALID_SOCKET_FD)
                        {
                            std::cerr << "[TCP] accept failed, shutting down TCP server thread\n";
                            break;
                        }

                        // For now: handle one client at a time (simple).
                        handle_tcp_client(clientSock, player, playlist, db);
                    }

                    close_socket(serverSock);

#ifdef _WIN32
                    WSACleanup();
#endif
                })
        .detach();
}

// ===================== HTTP server (for Postman/curl) =====================

static void send_http_response(socket_t client, int statusCode,
                               const std::string &bodyJson)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " OK\r\n"
        << "Content-Type: application/json\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Content-Length: " << bodyJson.size() << "\r\n"
        << "\r\n"
        << bodyJson;

    const std::string resp = oss.str();
    send(client, resp.c_str(), static_cast<int>(resp.size()), 0);
}

static void handle_http_client(socket_t client,
                               Player &player,
                               std::shared_ptr<Playlist> playlist)
{
    char buf[4096];
    int n = recv(client, buf, sizeof(buf) - 1, 0);
    if (n <= 0)
    {
        close_socket(client);
        return;
    }
    buf[n] = '\0';

    std::string req(buf);

    // Extract request line
    size_t lineEnd = req.find("\r\n");
    if (lineEnd == std::string::npos)
    {
        close_socket(client);
        return;
    }

    std::string requestLine = req.substr(0, lineEnd);
    std::istringstream iss(requestLine);
    std::string method, path, version;
    iss >> method >> path >> version;

    std::string lowerMethod = method;
    std::transform(lowerMethod.begin(), lowerMethod.end(), lowerMethod.begin(), ::tolower);

    // Basic routing
    if (lowerMethod == "get" && path == "/status")
    {
        std::string now = player.nowPlaying();
        // Simple JSON body
        std::string body = std::string("{\"nowPlaying\":\"") + now + "\"}";
        send_http_response(client, 200, body);
    }
    else if (lowerMethod == "post" && path == "/play")
    {
        player.playCurrent();
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"play\"}");
    }
    else if (lowerMethod == "post" && path == "/pause")
    {
        player.pause();
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"pause\"}");
    }
    else if (lowerMethod == "post" && path == "/resume")
    {
        player.resume();
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"resume\"}");
    }
    else if (lowerMethod == "post" && path == "/next")
    {
        player.playNext();
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"next\"}");
    }
    else if (lowerMethod == "post" && path == "/prev")
    {
        player.playPrevious();
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"prev\"}");
    }
    else if (lowerMethod == "post" && path == "/ff")
    {
        player.seekBy(10.0);
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"ff\",\"delta\":10}");
    }
    else if (lowerMethod == "post" && path == "/rew")
    {
        player.seekBy(-10.0);
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"rew\",\"delta\":-10}");
    }
    else if (lowerMethod == "post" && path == "/stop")
    {
        player.stop();
        send_http_response(client, 200, "{\"ok\":true,\"cmd\":\"stop\"}");
    }
    else
    {
        send_http_response(client, 404, "{\"error\":\"not found\"}");
    }

    close_socket(client);
}

void start_http_server(Player &player, std::shared_ptr<Playlist> playlist, int port)
{
    std::thread([&player, playlist, port]()
                {
#ifdef _WIN32
                    WSADATA wsaData;
                    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
                    if (wsaInit != 0)
                    {
                        std::cerr << "[HTTP] WSAStartup failed: " << wsaInit << "\n";
                        return;
                    }
#endif

                    socket_t serverSock = socket(AF_INET, SOCK_STREAM, 0);
                    if (serverSock == INVALID_SOCKET_FD)
                    {
                        std::cerr << "[HTTP] Failed to create socket\n";
#ifdef _WIN32
                        WSACleanup();
#endif
                        return;
                    }

                    int opt = 1;
                    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR,
                               reinterpret_cast<char *>(&opt), sizeof(opt));

                    sockaddr_in addr{};
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(static_cast<uint16_t>(port));
                    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

                    if (bind(serverSock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
                    {
                        std::cerr << "[HTTP] bind failed on port " << port << "\n";
                        close_socket(serverSock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return;
                    }

                    if (listen(serverSock, 8) < 0)
                    {
                        std::cerr << "[HTTP] listen failed\n";
                        close_socket(serverSock);
#ifdef _WIN32
                        WSACleanup();
#endif
                        return;
                    }

                    std::cout << "[HTTP] Listening on http://127.0.0.1:" << port << "\n";

                    while (true)
                    {
                        sockaddr_in clientAddr{};
#ifdef _WIN32
                        int clientLen = sizeof(clientAddr);
#else
                        socklen_t clientLen = sizeof(clientAddr);
#endif
                        socket_t clientSock = accept(serverSock,
                                                     reinterpret_cast<sockaddr *>(&clientAddr),
                                                     &clientLen);
                        if (clientSock == INVALID_SOCKET_FD)
                        {
                            std::cerr << "[HTTP] accept failed, shutting down HTTP server thread\n";
                            break;
                        }

                        // For simplicity handle one client per accept synchronously.
                        handle_http_client(clientSock, player, playlist);
                    }

                    close_socket(serverSock);

#ifdef _WIN32
                    WSACleanup();
#endif
                })
        .detach();
}
