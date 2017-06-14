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
#include <iomanip>

using namespace std;
using namespace rs::core;
using namespace rs::slam;

namespace console_display
{
class slam_console_display
{
public:
    slam_console_display()
    {

    }

    void on_pose(const tracking_accuracy tracking, float* pose_data)
    {
        cout << "tracking: accuracy=" << tracking_to_string(tracking).c_str()
             << "\t trans=(" << setw(5) << fixed << setprecision(2)
             << pose_data[3] << ", " // x
             << pose_data[7] << ", " // y
             << pose_data[11] << ")" // z
             << endl;
    }

    void on_occupancy(int count, const int* tiles, float occupancy_res)
    {
        float x_coord = tiles[0] * occupancy_res; // x coordinate in meters
        float z_coord = tiles[1] * occupancy_res; // z coordinate in meters
        int occ_pct = tiles[2]; // %occupancy

        cout << "occupancy map: tiles updated=" << count
             << "\ttile_0: pos=(" << x_coord
             << ", " << z_coord
             << ") occupancy=" << occ_pct << "%%"
             << endl;
    }

private:
    std::string tracking_to_string(const tracking_accuracy accuracy)
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
};

std::unique_ptr<console_display::slam_console_display> make_console_slam_display()
{
    return unique_ptr<slam_console_display> { new console_display::slam_console_display() };
}
}
