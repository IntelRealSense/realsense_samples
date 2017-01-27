// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <librealsense/rs.hpp>
#include "rs/core/types.h"
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <fstream>
#include <rs/utils/librealsense_conversion_utils.h>
#include <librealsense/slam/slam.h>
#include <signal.h>
#include "slam_stats.h"

#define ESC_KEY 27
using namespace std;
using namespace rs::core;
using namespace rs::slam;

// Converts a librealsense motion type to a RealSense SDK motion type
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

// Sets options and enables the required stream types (fisheye/depth)
void configure_camera_for_slam(rs::device* camera, video_module_interface::supported_module_config supported_slam_config)
{
    camera->set_option(rs::option::fisheye_strobe, 1); // Needed to align image timestamps to common clock-domain with the motion events. Required for SLAM.
    camera->set_option(rs::option::fisheye_external_trigger, 1); // This option causes the fisheye image to be aquired in-sync with the depth image. Required for SLAM.
    
    // Enable fisheye stream  
    auto &fisheye_stream_config = supported_slam_config[stream_type::fisheye];
    camera->enable_stream(rs::stream::fisheye, fisheye_stream_config.size.width, fisheye_stream_config.size.height, rs::format::raw8, fisheye_stream_config.frame_rate);
    
    // Enable depth stream
    auto &depth_stream_config = supported_slam_config[stream_type::depth];
    camera->enable_stream(rs::stream::depth, depth_stream_config.size.width, depth_stream_config.size.height, rs::format::z16, depth_stream_config.frame_rate);
}

// Create a callback for this stream type (depth/fisheye). This is where data from the camera gets passed to SLAM.
void set_callback_for_image_stream(rs::device* camera, stream_type streamType, std::unique_ptr<slam> const &slam, stream_stats& inputStreamStats)
{    
    function<void(rs::frame)> callback = [streamType, &slam, &inputStreamStats](rs::frame frame)
    {
        // Check for correct timestamp domain
        const auto timestampDomain = frame.get_frame_timestamp_domain();
        if(rs::timestamp_domain::microcontroller != timestampDomain)
        {
            cerr << "error: Junk time stamp in stream:" << (int)(streamType)<< "\twith frame counter:" << frame.get_frame_number() << endl;
            return;
        }

        // Construct a sample_set to pass to SLAM
        correlated_sample_set sampleSet = {};
        sampleSet[streamType] = image_interface::create_instance_from_librealsense_frame(frame, image_interface::flag::any);

        // Update input stream stats
        add_stream_samples(inputStreamStats, streamType, 1);

        // Pass the sample_set containing the image to SLAM
        if(slam->process_sample_set(sampleSet) < status_no_error)
        {
            cerr << "error: failed to process image sample" << endl;
        }

        // Now that the image has been processed, release it
        sampleSet[streamType]->release();
    };
    
    // Convert the stream type from a RealSense SDK type to a librealsense type.
    const rs::stream lrsStreamType = rs::utils::convert_stream_type(streamType); 
    
    // Set the callback for this stream type
    camera->set_frame_callback(lrsStreamType, callback);
}

// Set a callback to receive motion data. This is where the IMU data gets passed to the SLAM module.
// Unlike for the image streams, we use a single callback that handles both motion types (accel/gyro).
void set_callback_for_motion_streams(rs::device* camera, std::unique_ptr<slam> const &slam, stream_stats& inputStreamStats)
{
    std::function<void(rs::motion_data)> motion_callback = [&slam, &inputStreamStats](rs::motion_data entry)
    {
        // Convert the motion type from a librealsense type to a RealSense SDK type.
        const motion_type motionType = rs_to_sdk_motion_type(entry.timestamp_data.source_id); 

        // Construct a sample_set containing the motion data (either accel or gyro)
        correlated_sample_set sample_set = {};
        sample_set[motionType].timestamp = entry.timestamp_data.timestamp;
        sample_set[motionType].type = motionType;
        sample_set[motionType].frame_number = entry.timestamp_data.frame_number;
        sample_set[motionType].data[0] = entry.axes[0];
        sample_set[motionType].data[1] = entry.axes[1];
        sample_set[motionType].data[2] = entry.axes[2];

        // Update input motion sample stats
        add_motion_sensor_samples(inputStreamStats, motionType, 1);

        // Pass the sample_set to SLAM
        if(slam->process_sample_set(sample_set) < status_no_error)
        {
            cerr<<"error : failed to process motion sample" << endl;
        }
    };

    // An empty callback for timestamp data. Not used in this case.
    std::function<void(rs::timestamp_data)> timestamp_callback = [](rs::timestamp_data entry) { };

    // Set the device to enable motion data, and set the callbacks for it
    camera->enable_motion_tracking(motion_callback, timestamp_callback);
}

// Gets the stream config for a specific image stream type
video_module_interface::actual_image_stream_config get_stream_config(rs::device* device, stream_type streamType, video_module_interface::supported_module_config supported_slam_config)
{
    video_module_interface::supported_image_stream_config supported_stream_config = supported_slam_config[streamType];
    rs::stream lrsStreamType = rs::utils::convert_stream_type(streamType);
    intrinsics stream_intrinsics = rs::utils::convert_intrinsics(device->get_stream_intrinsics(lrsStreamType));
    
    video_module_interface::actual_image_stream_config actual_stream_config;
    actual_stream_config.size.width = supported_stream_config.size.width;
    actual_stream_config.size.height= supported_stream_config.size.height;
    actual_stream_config.frame_rate = supported_stream_config.frame_rate;
    actual_stream_config.intrinsics = stream_intrinsics;
    actual_stream_config.is_enabled = true;
    actual_stream_config.flags = rs::core::sample_flags::none;
    
    if (streamType == stream_type::fisheye)
    {
        // Read extrinsics from the device and set them in the fisheye stream config
        actual_stream_config.extrinsics_motion = rs::utils::convert_extrinsics(device->get_motion_extrinsics_from(rs::stream::fisheye));
        actual_stream_config.extrinsics = rs::utils::convert_extrinsics(device->get_extrinsics(rs::stream::depth, rs::stream::fisheye));
    }
    
    return actual_stream_config;
}

video_module_interface::actual_module_config get_slam_config(rs::device* device, std::unique_ptr<slam> const &slam, video_module_interface::supported_module_config supported_slam_config)
{
    // Construct the SLAM configuration that we will use
    video_module_interface::actual_module_config slam_config = {};

    // Copy the device name to the SLAM config
    auto device_name = device->get_name();
    memcpy(slam_config.device_info.name, device_name, std::strlen(device_name));
    
    // Set stream config for each image stream type
    slam_config[stream_type::fisheye] = get_stream_config(device, stream_type::fisheye, supported_slam_config);
    slam_config[stream_type::depth] = get_stream_config(device, stream_type::depth, supported_slam_config);
    
    // Enable accelerometer and gyroscope in the SLAM config
    slam_config[motion_type::accel].is_enabled = true;
    slam_config[motion_type::gyro].is_enabled = true;

    // Read intrinsics from the device and set them in the SLAM config
    auto motion_intrinsics = device->get_motion_intrinsics();
    slam_config[motion_type::accel].intrinsics = rs::utils::convert_motion_device_intrinsics(motion_intrinsics.acc);
    slam_config[motion_type::gyro].intrinsics = rs::utils::convert_motion_device_intrinsics(motion_intrinsics.gyro);

    return slam_config;
}

inline std::string tracking_accuracy_to_string(const tracking_accuracy accuracy)
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

inline bool is_esc_key_pressed(void)
{

    static const int STDIN = 0;
    static int initialized = 0;

    if (0 == initialized)
    {
        //Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting=0,data=0;
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
    bool is_esc_pressed = is_esc_key_pressed();
    if (is_esc_pressed)
        cout << endl << "Esc key pressed" << endl;
    if (!keep_running)
        cout << endl << "Ctrl-C pressed" << endl;

    return !keep_running || is_esc_pressed;
}

// Render map in opencv window
void render_image(std::string window_name, unsigned char *data, unsigned int width, unsigned int height)
{
    cv::Mat image(height, width, CV_8UC4, data);
    imshow(window_name, image);
    cv::waitKey(1);
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

