// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include "stats.h"
#include <librealsense/rs.hpp>
#include "rs/core/types.h"
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <signal.h>

#include "slam.h"

#define ESC_KEY 27

using namespace cv;
using namespace rs::slam;
using namespace std;


inline void add_motion_sensor_samples(stream_stats & streamStats, const rs::core::motion_type motion, const int count)
{
    if(motion == rs::core::motion_type::accel)
    {
        streamStats.add_acceleromter_samples(count);
    }
    else
    {
        if(motion == rs::core::motion_type::gyro)
        {
            streamStats.add_gyroscope_samples(count);
        }
    }
}

inline void add_stream_samples(stream_stats & streamStats, const rs::core::stream_type stream, const int count)
{
    if(stream == rs::core::stream_type::depth)
    {
        streamStats.add_depth_samples(count);
    }
    else
    {
        if(stream == rs::core::stream_type::fisheye)
        {
            streamStats.add_fisheye_samples(count);
        }
    }
}

inline bool is_user_press_esc(void)
{
    static const int STDIN = 0;
    static int initialized = 0;

    if (0 == initialized)
    {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting, data=0;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    if(bytesWaiting)
        data=getc(stdin);

    return data == ESC_KEY;
}

bool keep_running = true;

void signal_handler(int signal)
{
    if (keep_running)
        keep_running = false;
    else
        exit(1);
}

// Set up handler for user pressing Ctrl-C
inline void install_signal_handler()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

// To check if user pressed Ctrl-C or ESC key to exit app
inline bool is_key_pressed(void)
{
    bool is_esc_pressed = is_user_press_esc();
    if (is_esc_pressed)
        cout << endl << "Esc key pressed" << endl;
    if (!keep_running)
        cout << endl << "Ctrl-C pressed" << endl;

    return !keep_running || is_esc_pressed;
}


inline rs::core::motion_type rs_to_sdk_motion_type(const rs_event_source motionSource)
{
    rs::core::motion_type motionType = rs::core::motion_type::max;
    switch(motionSource)
    {
    case RS_EVENT_IMU_ACCEL:
        motionType = rs::core::motion_type::accel;
        break;

    case RS_EVENT_IMU_GYRO:
        motionType = rs::core::motion_type::gyro;
        break;
    case RS_EVENT_IMU_DEPTH_CAM:
    case RS_EVENT_IMU_MOTION_CAM:
    case RS_EVENT_G0_SYNC:
    case RS_EVENT_G1_SYNC:
    case RS_EVENT_G2_SYNC:
    case RS_EVENT_SOURCE_COUNT:
        break;
    }
    return motionType;
}

inline void clear_streams(rs::core::video_module_interface::supported_module_config &supported_config)
{
    for(int i = 0; i < (int) rs::core::stream_type::max; ++i)
    {
        supported_config[rs::core::stream_type(i)].is_enabled = false;
    }

    for(int i = (int) rs::core::motion_type::accel; i < (int) rs::core::motion_type::max; ++i)
    {
        supported_config[rs::core::motion_type(i)].is_enabled = false;
    }
}

inline rs::source get_source_type(rs::core::video_module_interface::supported_module_config &supported_config)
{
    rs::source active_sources = static_cast<rs::source>(0);
    for(int i = 0; i < (int) rs::core::stream_type::max; ++i)
    {
        if(supported_config[rs::core::stream_type(i)].is_enabled)
        {
            active_sources = rs::source::video;
            break;
        }
    }

    for(int i = (int) rs::core::motion_type::accel; i < (int) rs::core::motion_type::max; ++i)
    {
        if(supported_config[rs::core::motion_type(i)].is_enabled)
        {
            if(active_sources == rs::source::video)
            {
                active_sources = rs::source::all_sources;
            }
            else
            {
                active_sources = rs::source::motion_data;
            }
            break;
        }
    }
    return active_sources;
}

inline bool check_motion_sensor_capability_if_required(rs::device *device, rs::source requestedSource)
{
    if(requestedSource == rs::source::all_sources || rs::source::motion_data == requestedSource)
        return device->supports(rs::capabilities::motion_events);
    return true;
}

inline bool is_motion_stream_requested(const rs::source requestedSource)
{
    return (requestedSource == rs::source::all_sources || rs::source::motion_data == requestedSource);
}
