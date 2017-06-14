// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "transporter.hpp"
#include <seasocks/Logger.h>
#include <seasocks/PrintfLogger.h>
#include <seasocks/Server.h>
#include <seasocks/StringUtil.h>
#include <seasocks/WebSocket.h>

#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <thread>
#include <condition_variable>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

using namespace transport;
using namespace seasocks;
using namespace std;

class WsTransporter : public Transporter
{
    EventCallbacks& callback;

    // server stuff
    set<WebSocket*> connections;
    int websocketPort;
    const char* serverPath;

    unique_ptr<Server> server;
    shared_ptr<Logger> logger;
    shared_ptr<WebSocket::Handler> websocket_handler;

    // thread synchronization stuff
    mutex mut;
    condition_variable started_cond;
    bool server_available = false;

    class SocksHandler: public WebSocket::Handler
    {
    public:
        void onConnect(WebSocket* connection) override;
        void onData(WebSocket* connection, const uint8_t* data, size_t length)
        override;
        void onData(WebSocket* connection, const char* data) override;
        void onDisconnect(WebSocket* connection) override;

        WsTransporter * parent;
        SocksHandler(WsTransporter* wsd) :
            parent(wsd) {};
    };
    friend class SocksHandler;


    // wait for started_cond
    void wait_for_server_status(bool expectedStatus)
    {
        unique_lock<mutex> lock(mut);
        started_cond.wait(lock, [&] {return expectedStatus == server_available;});
    }

    // notify started_cond
    void notify_server_status(bool status)
    {
        lock_guard<mutex> lock(mut);
        server_available = status;
        started_cond.notify_one();
    }

public:
    WsTransporter(EventCallbacks& callback, const char* path, int port) :
        callback(callback), websocketPort(port), serverPath(path)
    {
    }

    ~WsTransporter() = default;

    bool is_connected() override
    {
        return server_available;
    }

    // Returns IP address according to preference
    // 0=nothing,  1=lo,  2=others,  3=wl,  4=en

    int classify_ifa_name(char *ifa_name)
    {
        if (!ifa_name || strlen(ifa_name)<2)
            return 0;
        if (ifa_name[0] == 'e'  &&  ifa_name[1] == 'n')
            return 4;
        if (ifa_name[0] == 'w'  &&  ifa_name[1] == 'l')
            return 3;
        if (ifa_name[0] == 'l'  &&  ifa_name[1] == 'o')
            return 1;
        return 2;
    }

    // Get IP address. Priorit order: Ethernet, Wifi, something else, local.
    string read_ip_addr()
    {
        struct ifaddrs *ifaddr, *ifa;
        int family, result, count;
        string best_ifa_name;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1)
        {
            return "0.0.0.0";
        }

        // traverse through the linked list, maintaining head pointer to free the list later

        // As we loop through the entries, remember the best choice
        int best_ifa = 0;			// 0=nothing,  1=lo,  2=others,  3=wl,  4=en
        char best_host[NI_MAXHOST+1];	        // Host IP addr of best found
        best_host[NI_MAXHOST] = 0;              // Just in case host IP addr string is too long

        for (ifa = ifaddr, count = 0; ifa != NULL; ifa = ifa->ifa_next, count++)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;

            // For an AF_INET* interface address, process for the IP and get the one with highest priority.

            if (family == AF_INET)
            {
                result = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (result != 0)
                {
                    // Cannot find ip address for network interface.
                    continue;
                }

                // cout << "name: " << ifa->ifa_name << "\t\t address: " << host << endl;
                int new_ifa = classify_ifa_name(ifa->ifa_name);
                if (new_ifa > best_ifa)
                {
                    best_ifa = new_ifa;
                    best_ifa_name = ifa->ifa_name;
                    strncpy(best_host, host, NI_MAXHOST);
                }
            }
        }
        cout <<"Best host: " << best_host << " of ifa type: " << best_ifa_name << endl;

        freeifaddrs(ifaddr);
        return (best_host);
    }


    void connect() override
    {
        if (serverPath == nullptr)
        {
            printf("no path specified...\n");
            return;
        }
        assert(!server_available);
        websocket_handler.reset(new SocksHandler(this));

        connections.clear();
        thread server_thread([&]()
        {
            logger.reset(new PrintfLogger(Logger::WARNING));
            server.reset(new Server(logger));
            server->addWebSocketHandler("/",
                                        websocket_handler,
                                        true);  //allow cross orgin
            server->serve(serverPath, websocketPort);
            notify_server_status(false);
        });
        server_thread.detach();
        cout << "\nServer: waiting for client to connect...\n" << endl;

        string ui_path =  "http://" + read_ip_addr() + ":8000/view.html";
        cout << " >>> point your browser to: \033[4;36m"
             << ui_path << "\033[0m\n" << endl << endl;

        // block until client connects
        wait_for_server_status(true);
    }

    void disconnect() override
    {
        if (!server_available) return;
        server->terminate();
        wait_for_server_status(false);
    }

    void send_data(const iovector* iov, const int count) override
    {
        if (count < 1)
        {
            cerr << "send_data, called with 0 vectors";
            return;
        }

        // get size of iov
        size_t size = 0;
        for (iovector* i = (iovector*)iov; i < iov+count; i++)
        {
            size += i->iov_len;
        }

        // copy into contiguous array
        uint8_t* buf = new uint8_t[size];
        uint8_t* p = buf;
        for (iovector* i = (iovector*)iov; i < iov+count; i++)
        {
            memcpy(p, i->iov_base, i->iov_len);
            p += i->iov_len;
        }

        shared_ptr<uint8_t> shared_buf(buf, [buf](uint8_t* p)
        {
            delete[] buf;
        });

        server->execute([this, shared_buf, size]
        {
            for (WebSocket* ws : connections)
            {
                ws->send(shared_buf.get(), size);
            }
        });
    }

    void send_data(void *data, size_t len) override
    {
        iovector iov = { data, len };
        send_data(&iov, 1);
    }

    void send_data_string(std::string string) override
    {
        server->execute([this, string]
        {
            for (WebSocket* ws : connections)
            {
                ws->send(string);
            }
        });
    }
};

///////////////////////////////////////////////////////////////////////////////
// WebSocket::Handler methods

void WsTransporter::SocksHandler::onConnect(WebSocket* connection)
{
    cout << "server: got connection "
         << formatAddress(connection->getRemoteAddress()) << endl;
    parent->connections.insert(connection);
    if (parent->connections.size() == 1 && !parent->server_available)
    {
        parent->notify_server_status(true);
    }

}

void WsTransporter::SocksHandler::onDisconnect(WebSocket* connection)
{
    cout << "server: disconnect "
         << formatAddress(connection->getRemoteAddress()) << endl;
    parent->connections.erase(connection);
#if 0
    if (parent->connections.size() == 0)
    {
        cout << "server: no more connections remaining... stopping" << endl;
        parent->callback.on_disconnect(*parent);
    }
#endif
}

void WsTransporter::SocksHandler::onData(WebSocket* connection,
        const uint8_t* data, size_t length)
{
    parent->callback.on_data(*parent, data, length);
}

void WsTransporter::SocksHandler::onData(WebSocket* connection,
        const char* data)
{
    parent->callback.on_data_string(*parent, string{data});
}

unique_ptr<Transporter> transport::make_transporter(EventCallbacks& callback, const char* path, int port)
{
    WsTransporter* ws = new WsTransporter(callback, path, port);
    return unique_ptr<Transporter> { ws };
}
