// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <memory>
#include <string>
#include <stdint.h>

namespace transport
{

struct iovector
{
    void* iov_base;
    size_t iov_len;
};

class Transporter
{
public:
    virtual bool is_connected() = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void send_data(const iovector* iov, const int count) = 0;
    virtual void send_data(void *data, size_t len) = 0;
    virtual void send_data_string(std::string string) = 0;
    virtual ~Transporter() = default;
};

class EventCallbacks
{
public:
    virtual void on_data(Transporter& net, const uint8_t* data, size_t len) {};
    virtual void on_data_string(Transporter& net, std::string&& string) {};
    virtual void on_disconnect(Transporter& net) {};
    virtual ~EventCallbacks() = default;
};

std::unique_ptr<Transporter> make_transporter(EventCallbacks& callback,
        const char* path, int port);

}
