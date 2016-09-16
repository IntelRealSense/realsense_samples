// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

//#include "rs/core/video_module_interface.h"
#include <librealsense/rs.hpp>
#include "rs/core/types.h"
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <fstream>

using namespace rs::slam;

std::string tracking_tostring(const tracking_accuracy accuracy)
{
    switch (accuracy)
    {
    case tracking_accuracy::failed:
        return "fail";
    case tracking_accuracy::low:
        return "low";
    case tracking_accuracy::medium:
        return "med";
    case tracking_accuracy::high:
        return "high";
    default:
        return "  ";
    }
}

inline int kbhit(void)
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

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

inline rs::core::motion_type rs_to_sdk_motion_type(const rs_event_source motionSource)
{
    rs::core::motion_type motionType = rs::core::motion_type::max;
    switch (motionSource)
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
    case RS_EVENT_SOURCE_MAX_ENUM:
        break;
    }
    return motionType;
}

inline void clear_streams(rs::core::video_module_interface::supported_module_config& supported_config)
{
    for (int i = 0; i < (int)rs::core::stream_type::max; ++i)
    {
        supported_config[rs::core::stream_type(i)].is_enabled = false;
    }

    for (int i = (int)rs::core::motion_type::accel; i < (int)rs::core::motion_type::max; ++i)
    {
        supported_config[rs::core::motion_type(i)].is_enabled = false;
    }
}

inline rs::source get_source_type(rs::core::video_module_interface::supported_module_config& supported_config)
{
    rs::source active_sources = static_cast<rs::source>(0);
    for (int i = 0; i < (int)rs::core::stream_type::max; ++i)
    {
        if (supported_config[rs::core::stream_type(i)].is_enabled)
        {
            active_sources = rs::source::video;
            break;
        }
    }

    for (int i = (int)rs::core::motion_type::accel; i < (int)rs::core::motion_type::max; ++i)
    {
        if (supported_config[rs::core::motion_type(i)].is_enabled)
        {
            if (active_sources == rs::source::video)
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

inline bool check_motion_sensor_capability_if_required(rs::device* device, rs::source requestedSource)
{
    if (requestedSource == rs::source::all_sources || rs::source::motion_data == requestedSource)
        return device->supports(rs::capabilities::motion_events);
    return true;
}

inline bool is_motion_stream_requested(const rs::source requestedSource)
{
    return (requestedSource == rs::source::all_sources || rs::source::motion_data == requestedSource);
}
