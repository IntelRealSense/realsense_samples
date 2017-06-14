// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

//
// slam_tutorial_1_gui: This app illustrates the use of the librealsense and librealsense_slam libraries.
// The librealsense library streams depth, fish eye, and IMU data from the ZR300 camera which are used
// as input to the librealsense_slam library. The librealsense_slam library outputs the camera pose
// (position and orientation) and occupancy map. The occupancy map is a 2D map of the surroundings
// that shows which areas are occupied by obstacles to navigation.
//

#include <iostream>
#include <iomanip>
#include <map>
#include <thread>

#include <librealsense/rs.hpp>
#include <rs_sdk.h>
#include <librealsense/slam/slam.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "version.h"
#include "slam_utils.hpp"

using namespace std;
using namespace rs::core;
using namespace rs::slam;

const std::string WINDOW_NAME = "map_window";
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// Version number of the samples
extern constexpr auto rs_sample_version = concat("VERSION: ",RS_SAMPLE_VERSION_STR);

//--------------------------------------------------------------------------------------
// This event handler receives updates from the SLAM module
//--------------------------------------------------------------------------------------

class slam_event_handler : public rs::core::video_module_interface::processing_event_handler
{
    shared_ptr<occupancy_map> occ_map;
public:
    void module_output_ready(rs::core::video_module_interface * sender, correlated_sample_set * sample)
    {
        // Get a handle to the slam module
        auto slam = dynamic_cast<rs::slam::slam*>(sender);

        // Create an occupancy map container if we don't already have one.
        if (!occ_map)
        {
            // This will only happen on the first execution of this callback.
            occ_map = slam->create_occupancy_map(50 * 50);
        }

        // Get the camera pose
        PoseMatrix4f pose;
        slam->get_camera_pose(pose);

        // Get the tracking accuracy as a string
        const std::string trackingAccuracy = tracking_accuracy_to_string(slam->get_tracking_accuracy());

        // Get the number of occupancy map tiles that were updated
        slam->get_occupancy_map_update(occ_map);
        int count_updated_tiles = occ_map->get_tile_count(); // This will only be > 0 when the tracking accuracy is > low.

        // Print the camera pose and number of updated occupancy map tiles to the console
        cout << fixed << setprecision(2) << "Translation:(X=" << pose.m_data[3] << ", Y=" << pose.m_data[7] << ", Z=" << pose.m_data[11] << ")    Accuracy:" << trackingAccuracy << "    Tiles_Updated:" << count_updated_tiles << "\r" << flush;

        // Get the occupany map (with trajectory) as an image and render it in the map window
        unsigned char *map_image;
        unsigned int width, height;
        rs::core::status status = slam->get_occupancy_map_as_rgba(&map_image, &width, &height, true, true);
        if (status == status_no_error)
        {
            render_image(WINDOW_NAME, map_image, width, height);
        }
    }
};

int main (int argc, char* argv[])
{
    // Handle Ctrl-C
    install_signal_handler();

    //--------------------------------------------------------------------------------------
    // Setup a librealsense context and camera device
    //--------------------------------------------------------------------------------------

    std::unique_ptr<context_interface> context;
    if (argc<2)
    {
        // Create a regular context for streaming live data from the camera
        context.reset(new rs::core::context());
        if(context->get_device_count() == 0)
        {
            cout << "Error: Device is null." << "There are no RealSense devices connected." << endl << "Please connect a RealSense device and restart the application" << endl;
            return -1;
        }
    }
    else
    {
        // Create a playback context if a filename was provided as an argument
        context.reset(new rs::playback::context(argv[1]));
        if(context->get_device_count() == 0)
        {
            cerr<<"error : can't open file" << endl;
            return -1;
        }
    }

    rs::device *device = context->get_device(0);

    //--------------------------------------------------------------------------------------
    // Setup an instance of the SLAM module
    //--------------------------------------------------------------------------------------

    std::unique_ptr<slam> slam(new rs::slam::slam());
    slam->set_occupancy_map_resolution(0.025);

    // Register a slam_event_handler (defined above) to receive the output of the SLAM module
    slam_event_handler slamEventHandler;
    slam->register_event_handler(&slamEventHandler);

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
        cerr<<"error : current device does not support motion events" << endl; // If you get this, unplug and reconnect the camera
        return -1;
    }

    //--------------------------------------------------------------------------------------
    // Configure the camera device for SLAM
    //--------------------------------------------------------------------------------------

    configure_camera_for_slam(device, supported_slam_config);

    stream_stats inputStreamStats; // Not used in this example
    set_callback_for_image_stream(device, stream_type::fisheye, slam, inputStreamStats);
    set_callback_for_image_stream(device, stream_type::depth, slam, inputStreamStats);
    set_callback_for_motion_streams(device, slam, inputStreamStats);

    //--------------------------------------------------------------------------------------
    // Set the SLAM configuration
    //--------------------------------------------------------------------------------------

    // Construct the SLAM configuration using a handy helper function
    video_module_interface::actual_module_config slam_config = get_slam_config(device, slam, supported_slam_config);

    if(slam->set_module_config(slam_config) < status_no_error)
    {
        cerr<<"error : failed to set the SLAM configuration" << endl;
        return -1;
    }

    //--------------------------------------------------------------------------------------
    // Setup the map window
    //--------------------------------------------------------------------------------------

    cv::namedWindow(WINDOW_NAME, 0);
    cv::resizeWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);

    //--------------------------------------------------------------------------------------
    // Start streaming data from the camera device
    //--------------------------------------------------------------------------------------

    cout << endl << "Starting SLAM..." << endl;
    device->start(active_sources);

    //--------------------------------------------------------------------------------------
    // Run until a key is pressed
    //--------------------------------------------------------------------------------------

    cout << "Press Esc key to exit" << endl;
    while(!is_key_pressed() && device->is_streaming())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    slam->unregister_event_handler(&slamEventHandler); // Not strictly necessary, but stops printing pose data to console

    //--------------------------------------------------------------------------------------
    // Stop SLAM module and close camera device
    //--------------------------------------------------------------------------------------

    cout << endl << "Stopping..." << endl;
    slam->flush_resources();
    device->stop(active_sources);

    //--------------------------------------------------------------------------------------
    // Save the occupancy map to disk
    //--------------------------------------------------------------------------------------

    cout << "Saving occupancy map to disk..." << endl;
    slam->save_occupancy_map_as_ppm("occupancy.ppm", true);

    return 0;
}
