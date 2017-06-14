// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <librealsense/rs.hpp>
#include <rs_sdk.h>
#include <librealsense/slam/slam.h>
#include "rs/core/types.h"
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>

#include "transporter_proxy.hpp"

using namespace std;
using namespace rs::core;
using namespace rs::slam;
using nlohmann::json;

namespace web_display
{
class slam_web_display
{
public:
    slam_web_display(const char *path, int port, bool jpeg)
    {
        m_transporter_proxy = &(transporter_proxy::getInstance(path, port, jpeg));
    }

    void set_control_callbacks(display_controls controls)
    {
        m_transporter_proxy->set_control_callbacks(controls);

    }

    void on_reset_competed()
    {
        json msg;
        msg["type"] = "event";
        msg["event"] = "on_reset_completed";

        m_transporter_proxy->send_json_data(msg);
    }

    void on_pose(int tracking, float* pose)
    {
        json msg;
        msg["type"] = "tracking";
        msg["tracking"] = tracking;
        json& p = msg["pose"];
        for (int i = 0; i < 12; i++)
        {
            p += pose[i];
        }
        m_transporter_proxy->send_json_data(msg);

    }

    void on_occupancy(float scale, int size, const int* data)
    {
        m_transporter_proxy->on_occupancy(scale, size, data);

    }

    void on_fps(const char* type, float fisheye, float depth,
                float accelerometer, float gyroscope)
    {
        json msg;
        msg["type"] = "fps";
        msg["fps"]["type"] = type;
        msg["fps"]["fisheye"] = fisheye;
        msg["fps"]["depth"] = depth;
        msg["fps"]["accelerometer"] = accelerometer;
        msg["fps"]["gyroscope"] = gyroscope;
        m_transporter_proxy->send_json_data(msg);
    }

    void on_fisheye_frame(uint64_t ts_micros, int width, int height,
                          const void* data)
    {
        m_transporter_proxy->on_fisheye_frame(ts_micros, width, height, data);

    }

    void on_rgb_frame(uint64_t ts_micros, int width, int height, const void* data)
    {
        m_transporter_proxy->on_rgb_frame(ts_micros, width, height, data);
    }


private:
    transporter_proxy *m_transporter_proxy;
};

std::unique_ptr<web_display::slam_web_display> make_slam_web_display(string &sample_name, int port, bool jpeg_compression)
{
    size_t found = sample_name.find_last_of("/");
    string path = sample_name + "_browser";
    if(found != string::npos)
    {
        path = sample_name.substr(found+1) + "_browser";
    }

    // If file content not found locally, look for it under 'share'
    struct stat info;
    if( stat( path.c_str(), &info ) != 0 )
    {
        path = INSTALL_PREFIX "/share/librealsense/samples/" + path;
    }

    cout << endl << "web folder path: " << path << endl;

    return unique_ptr<slam_web_display> { new web_display::slam_web_display(path.c_str(), port, jpeg_compression) };
}

}
