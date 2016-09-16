// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <map>
#include <thread>

#include <librealsense/rs.hpp>

#include <rs_sdk.h>
#include <librealsense/slam/slam.h>
#include "utils.h"

using namespace std;
using namespace rs::core;
using namespace rs::slam;

class slam_event_handler : public video_module_interface::processing_event_handler
{
    shared_ptr<occupancy_map> occupancy;
    float occupancy_res = -1;

public:
    slam_event_handler() {}
    ~slam_event_handler() = default;

    void process_sample_complete(video_module_interface* sender,
                                 correlated_sample_set* sample)
    {
        slam* slam_module = dynamic_cast<slam*>(sender);

        // 1. Camera pose update
        PoseMatrix4f pose;
        slam_module->get_camera_pose(pose);
        // translation is 4th column vector of camera pose
        printf("tracking: accuracy=%s,\ttrans=(% 5.2f, % 5.2f, % 5.2f)\n",
               tracking_tostring(slam_module->get_tracking_accuracy()).c_str(),
               pose.m_data[3], // x
               pose.m_data[7], // y
               pose.m_data[11]); // z

        if (!occupancy)
        {
        	//
            occupancy = slam_module->create_occupancy_map(50 * 50);
            occupancy_res = slam_module->get_occupancy_map_resolution();
            printf("occupancy map: creating update storage\n");
        }

        // 2. Occupanc map update
        auto occ_status = slam_module->get_occupancy_map_update(occupancy);
        int count = occupancy->get_tile_count();
        if (occ_status >= status::status_no_error && count > 0)
        {
        	const int* tiles = occupancy->get_tile_coordinates();
        	float x_coord = tiles[0] * occupancy_res; // x coordinate in meters
        	float z_coord = tiles[1] * occupancy_res; // z coordinate in meters
        	int occ_pct = tiles[2]; // %occupancy
        	printf("occ. map: tiles updated=%d\ttile_0: pos=(% 5.2f, % 5.2f) occupancy=%2d%%\n",
        			count, x_coord, z_coord, occ_pct);
        }
        fflush(stdout);
    }
};

int RunAsyncSample(int argc, char* argv[])
{
    rs::context ctx;
    if (ctx.get_device_count() == 0)
    {
        cerr << "error : can't find devices" << endl;
        return -1;
    }

    rs::device* device = ctx.get_device(0);

    video_module_interface::supported_module_config supported_config = {};
    video_module_interface::actual_module_config actual_config = {};

    slam slam_module;
    if (slam_module.query_supported_module_config(0, supported_config) < status_no_error)
    {
        cerr << "error : failed to query the first supported module configuration" << endl;
        return -1;
    }

    auto device_name = device->get_name();
    auto is_current_device_valid = (strcmp(device_name, supported_config.device_name) == 0);
    if (!is_current_device_valid)
    {
        cerr << "error : current device '" << device_name << "' is not supported by"
             << " the current supported module configuration" << endl;
        return -1;
    }

    memcpy(actual_config.device_info.name, supported_config.device_name,
           strlen(supported_config.device_name));

    rs::source active_sources = get_source_type(supported_config); // static_cast<rs::source>(0);
    if (!check_motion_sensor_capability_if_required(device, active_sources))
    {
        cerr << "error : current device is not supported motion events" << endl;
        return -1;
    }

    map<stream_type, function<void(rs::frame)>> stream_callback_per_stream;
    for (int i = 0; i < (int)stream_type::max; ++i)
    {
        stream_type stream = stream_type(i);
        auto& supported_stream_config = supported_config[stream];

        if (!supported_stream_config.is_enabled)
            continue;

        rs::format stream_format = rs::format::any;
        if (stream == stream_type::depth)
        {
            stream_format = rs::format::z16;
        }
        else
        {
            if (stream == stream_type::fisheye)
            {
                stream_format = rs::format::raw8;
            }
        }
        rs::stream librealsense_stream = rs::utils::convert_stream_type(stream);
        device->enable_stream(
            librealsense_stream, supported_stream_config.ideal_size.width,
            supported_stream_config.ideal_size.height, stream_format,
            supported_stream_config.ideal_frame_rate);

        video_module_interface::actual_image_stream_config& actual_stream_config = actual_config[stream];
        actual_stream_config.size.width = supported_stream_config.ideal_size.width;
        actual_stream_config.size.height = supported_stream_config.ideal_size.height;
        actual_stream_config.frame_rate = supported_stream_config.ideal_frame_rate;
        actual_stream_config.intrinsics = rs::utils::convert_intrinsics(
                                              device->get_stream_intrinsics(librealsense_stream));
        actual_stream_config.is_enabled = true;

        stream_callback_per_stream[stream] = [stream, &slam_module](rs::frame frame)
        {
            const auto timestampDomain = frame.get_frame_timestamp_domain();
            if (rs::timestamp_domain::microcontroller != timestampDomain)
            {
                cerr << "error: Junk time stamp in stream:" << (int)(stream)
                     << "\twith frame counter:" << frame.get_frame_number() << endl;
                return;
            }

            correlated_sample_set sample_set = {};
            sample_set[stream] = image_interface::create_instance_from_librealsense_frame(
                                     frame, image_interface::flag::any, nullptr);

            if (slam_module.process_sample_set_async(&sample_set) < status_no_error)
            {
                cerr << "error: failed to process sample" << endl;
            }
            sample_set[stream]->release();
        };

        device->set_frame_callback(librealsense_stream, stream_callback_per_stream[stream]);
    }

    // define callback to the motion events and set it, the callback lifetime
    // assumes the module is available.
    function<void(rs::motion_data)> motion_callback;
    function<void(rs::timestamp_data)> timestamp_callback;
    if (is_motion_stream_requested(active_sources))
    {
        actual_config[motion_type::accel].is_enabled = true;
        actual_config[motion_type::gyro].is_enabled = true;
        motion_callback = [&slam_module](rs::motion_data entry)
        {
            motion_type motionType = rs_to_sdk_motion_type(entry.timestamp_data.source_id);
            if (motionType != motion_type::max)
            {
                correlated_sample_set sample_set = {};
                sample_set[motionType].timestamp = entry.timestamp_data.timestamp;
                sample_set[motionType].type = motionType;
                sample_set[motionType].frame_number = entry.timestamp_data.frame_number;
                sample_set[motionType].data[0] = entry.axes[0];
                sample_set[motionType].data[1] = entry.axes[1];
                sample_set[motionType].data[2] = entry.axes[2];

                if (slam_module.process_sample_set_async(&sample_set) < status_no_error)
                {
                    cerr << "error : failed to process sample" << endl;
                }
            }
        };

        timestamp_callback = [](rs::timestamp_data entry) {};
        device->enable_motion_tracking(motion_callback, timestamp_callback);

        auto _motion_intrinsics = device->get_motion_intrinsics();
        actual_config[motion_type::accel].intrinsics.acc = rs::utils::convert_motion_device_intrinsic(_motion_intrinsics.acc);
        actual_config[motion_type::gyro].intrinsics.gyro = rs::utils::convert_motion_device_intrinsic(_motion_intrinsics.gyro);
    }

    actual_config[stream_type::fisheye].extrinsics = rs::utils::convert_extrinsics(
                device->get_motion_extrinsics_from(rs::stream::fisheye));
    actual_config[stream_type::depth].extrinsics = rs::utils::convert_extrinsics(
                device->get_extrinsics(rs::stream::depth, rs::stream::fisheye));

    if (slam_module.set_module_config(actual_config) < status_no_error)
    {
        cerr << "error : failed to set the enabled module configuration" << endl;
        return -1;
    }

    slam_module.set_occupancy_map_height_of_interest(-0.5f, 1.0f);
    slam_event_handler slamEventHandler;
    slam_module.register_event_hander(&slamEventHandler);

    // enable fisheye strobe
    device->set_option(rs::option::fisheye_strobe, 1);
    device->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);
    //enable auto exposure for fisheye camera stream
    device->set_option(rs::option::fisheye_color_auto_exposure, 1);


    cout << endl << "---- start ---- " << endl;
    device->start(active_sources);
    cout << "Press Enter key to exit" << endl;

    while (!kbhit())
    {
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    slam_module.query_video_module_control()->reset();
    cout << "saving occupancy map image: "
         << slam_module.save_occupancy_map_as_ppm("trajectory.ppm", true) << endl;

    device->stop(active_sources);
    cout << endl << "---- stop ---- " << endl;

    return 0;
}

int main(int argc, char* argv[])
{
    return RunAsyncSample(argc, argv);
}
