// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <map>
#include <thread>
#include <librealsense/rs.hpp>
#include <rs_sdk.h>
#include <librealsense/slam/slam.h>

#include "slam_web_display.hpp"
#include "slam_utils.hpp"
#include "version.h"

using namespace std;
using namespace rs::core;
using namespace rs::slam;

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

unique_ptr<web_display::slam_web_display>  web_view;

stream_stats inputStreamStats;
stream_stats processStreamStats;

class slam_tracking_event_handler : public tracking_event_handler
{
public:
    slam_tracking_event_handler()
    {

    }

    void on_restart()
    {
        web_view->on_reset_competed();
        cout << "Restart--------------------------------" << endl;
        fflush(stdout);
    }
    ~slam_tracking_event_handler()
    {

    }
};


class slam_event_handler : public video_module_interface::processing_event_handler
{
public:
    slam_event_handler() {}
    ~slam_event_handler() = default;

    void report_timestamp_fps(correlated_sample_set * sample)
    {
        add_stream_samples(processStreamStats, rs::core::stream_type::fisheye, 1);
        add_stream_samples(processStreamStats, rs::core::stream_type::depth, (sample->images[(int)rs::core::stream_type::depth]) ? 1 : 0);
        add_motion_sensor_samples(processStreamStats, rs::core::motion_type::accel, (int)sample->motion_samples[(int)rs::core::motion_type::accel].frame_number);
        add_motion_sensor_samples(processStreamStats, rs::core::motion_type::gyro, (int)sample->motion_samples[(int)rs::core::motion_type::gyro].frame_number);
    }

    void module_output_ready(video_module_interface* sender,
                             correlated_sample_set* sample)
    {
        report_timestamp_fps(sample);

        slam* slam_module = dynamic_cast<slam*>(sender);

        // 1. Camera pose update
        PoseMatrix4f pose;
        slam_module->get_camera_pose(pose);
        auto trackingAccuracy = slam_module->get_tracking_accuracy();
        web_view->on_pose((int)(trackingAccuracy), pose.m_data);

        if (!occupancy)
        {
            //
            occupancy = slam_module->create_occupancy_map(50 * 50);
            occupancy_res = slam_module->get_occupancy_map_resolution();
        }

        // 2. Occupancy map update
        auto occ_status = slam_module->get_occupancy_map_update(occupancy);
        int count = occupancy->get_tile_count();
        if (occ_status >= status::status_no_error && count > 0)
        {
            const int* tiles = occupancy->get_tile_coordinates();
            web_view->on_occupancy(slam_module->get_occupancy_map_resolution(), count, tiles);
        }

        // 3. send fisheye
        auto fisheye = sample->images[(int)rs::core::stream_type::fisheye];
        auto fish_info = fisheye->query_info();
        uint64_t micros = fisheye->query_time_stamp() * 1000.0*1000.0;
        web_view->on_fisheye_frame(micros, fish_info.width, fish_info.height, fisheye->query_data());
    }

private:
    shared_ptr<occupancy_map> occupancy;
    float occupancy_res = -1;
};

int main(int argc, char* argv[])
{
    // Create and start remote(Web) view
    string sample_name = argv[0];
    web_view = move(web_display::make_slam_web_display(sample_name, 8000, true));

    // Handle Ctrl-C
    install_signal_handler();

    //--------------------------------------------------------------------------------------
    // Setup a librealsense context and device
    //--------------------------------------------------------------------------------------

    std::unique_ptr<rs::context> context(new rs::context());
    if(context->get_device_count() == 0)
    {
        cout << "Error: Device is null. " << "There are no RealSense devices connected." << endl << "Please connect a RealSense device and restart the application" << endl;
        return -1;
    }

    rs::device *device = context->get_device(0);

    //--------------------------------------------------------------------------------------
    // Setup an instance of the SLAM module
    //--------------------------------------------------------------------------------------

    std::unique_ptr<slam> slam(new rs::slam::slam());
    slam->set_occupancy_map_resolution(0.025); // This affects how large the map window is drawn

    // Register a slam_event_handler (defined above) to receive the output of the SLAM module
    slam_event_handler slamEventHandler;
    slam->register_event_handler(&slamEventHandler);

    // Register a tracking_event_handler (defined above) to receive tracking event update
    slam_tracking_event_handler trackingEventHandler;
    slam->register_tracking_event_handler(&trackingEventHandler);

    //--------------------------------------------------------------------------------------
    // Do some sanity checking
    //--------------------------------------------------------------------------------------

    // Ask the SLAM module what configuration it supports
    video_module_interface::supported_module_config supported_slam_config = {};
    if (slam->query_supported_module_config(0, supported_slam_config) < status_no_error)
    {
        cerr<<"error : failed to query the first supported module configuration" << endl;
        return false;
    }

    // Check to make sure the current device name (ex: Intel RealSense ZR300) is one that the SLAM module supports
    auto device_name = device->get_name();
    auto is_current_device_valid = (strcmp(device_name, supported_slam_config.device_name) == 0);
    if (!is_current_device_valid)
    {
        cerr<<"error : current device is not supported by the current module configuration" << endl;
        return -1;
    }

    // Check to make sure that the current device supports motion data
    rs::source active_sources = get_source_type(supported_slam_config);
    if(!check_motion_sensor_capability_if_required(device, active_sources))
    {
        cerr<<"error : current device does not support motion events" << endl; // if you get this, unplug and reconnect the camera
        return -1;
    }

    //--------------------------------------------------------------------------------------
    // Configure the camera for SLAM
    //--------------------------------------------------------------------------------------

    configure_camera_for_slam(device, supported_slam_config);

    stream_stats inputStreamStats; // Not used in this example
    set_callback_for_image_stream(device, stream_type::fisheye, slam, inputStreamStats);
    set_callback_for_image_stream(device, stream_type::depth, slam, inputStreamStats);
    set_callback_for_motion_streams(device, slam, inputStreamStats);

    //--------------------------------------------------------------------------------------
    // Construct the SLAM configuration using a handy helper function
    //--------------------------------------------------------------------------------------

    video_module_interface::actual_module_config slam_config = get_slam_config(device, slam, supported_slam_config);

    //--------------------------------------------------------------------------------------
    // Set the SLAM configuration
    //--------------------------------------------------------------------------------------

    if(slam->set_module_config(slam_config) < status_no_error)
    {
        cerr<<"error : failed to set the SLAM configuration" << endl;
        return -1;
    }

    //--------------------------------------------------------------------------------------
    // Start streaming data from the camera
    //--------------------------------------------------------------------------------------
    cout << endl << "-------- Starting SLAM.  Press Esc key to exit --------" << endl << endl;
    device->start(active_sources);

    //--------------------------------------------------------------------------------------
    // Set up the remote display
    //--------------------------------------------------------------------------------------

    atomic_bool ui_request_stop { false };

    display_controls controls;
    controls.reset = [&]()
    {
        std::cout << "rs_sp: calling reset" << endl;
        slam->restart();
    };
    controls.stop = [&]()
    {
        ui_request_stop = true;
    };

    // Set control callback to remote display
    web_view->set_control_callbacks(controls);


    auto send_fps_stats = [&](const char* type, const stream_stats& stats)
    {
        web_view->on_fps(type,
                         stats.get_fisheye_fps(),
                         stats.get_depth_fps(),
                         stats.get_acceleromter_fps(),
                         stats.get_gyroscope_fps());
    };

    //--------------------------------------------------------------------------------------
    // Run until a key is pressed
    //--------------------------------------------------------------------------------------

    while (!is_key_pressed() && device->is_streaming())
    {
        this_thread::sleep_for(chrono::milliseconds(500));
        send_fps_stats("input", inputStreamStats);
        send_fps_stats("tracking", processStreamStats);
    }

    //--------------------------------------------------------------------------------------
    // Stop SLAM module and close camera device
    //--------------------------------------------------------------------------------------

    cout << endl << "Stopping..." << endl;
    slam->flush_resources();
    device->stop(active_sources);

    return 0;
}

