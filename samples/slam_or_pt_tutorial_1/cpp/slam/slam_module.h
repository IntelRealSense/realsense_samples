
// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <librealsense/rs.hpp>
#include "rs_sdk.h"
#include "slam.h"
#include "utils.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;
using namespace rs::core;
using namespace rs::slam;


class slam_tracking_event_handler : public tracking_event_handler
{
public:
    slam_tracking_event_handler(module_result_listener_interface* listener) : m_module_listener(listener)
    {
    }

    void on_restart()
    {
        m_module_listener->on_slam_restart();
    }

    ~slam_tracking_event_handler()
    {
    }
private:
    module_result_listener_interface* m_module_listener;
};


class slam_event_handler : public rs::core::video_module_interface::processing_event_handler
{
public:
    std::shared_ptr<occupancy_map> occ_map;

    slam_event_handler(module_result_listener_interface* listener)
        : m_module_listener(listener)
    {
    }

    void module_output_ready(rs::core::video_module_interface * sender, correlated_sample_set * sample)
    {
        // 1. Camera pose update
        slam *pSP = dynamic_cast<slam *>(sender);

        const tracking_accuracy trackingAccuracy = pSP->get_tracking_accuracy();

        PoseMatrix4f cameraPose;
        pSP->get_camera_pose(cameraPose);

        // Get depth frame id
        if(sample->images[(int)rs::core::stream_type::depth])
        {
            m_current_depth_frame_id = sample->images[(int)rs::core::stream_type::depth]->query_frame_number();
        }

        m_module_listener->on_slam_pose_update(trackingAccuracy, cameraPose, m_current_depth_frame_id);

        if (!occ_map)
        {
            occ_map = pSP->create_occupancy_map(50 * 50);
            occupancy_res = pSP->get_occupancy_map_resolution();
            std::cout << " creating occupancy map: resolution: " << pSP->get_occupancy_map_resolution() << std::endl;
        }

        // 2. Occupancy map update
        int status = pSP->get_occupancy_map_update(occ_map);
        int count = occ_map->get_tile_count();
        if (status >= 0 && count > 0)
        {
            const int32_t* tiles = occ_map->get_tile_coordinates();
            m_module_listener->on_slam_occupancy_update(pSP->get_occupancy_map_resolution(), count, tiles, occupancy_res);
        }

        // 3. fisheye data
        auto fisheye = sample->images[(int)rs::core::stream_type::fisheye];
        auto fish_info = fisheye->query_info();
        m_module_listener->on_slam_fisheye_update(10, fish_info.width, fish_info.height, fisheye->query_data());
    }

    ~slam_event_handler()
    {

    }

private:
    module_result_listener_interface* m_module_listener;
    float occupancy_res = -1;
    int m_current_depth_frame_id = -1;
};


class slam_module
{
public:
    slam_module(module_result_listener_interface* module_listener)
        : m_module_listener(module_listener), m_slam(new rs::slam::slam()),
          m_initialized(false), actual_slam_config( {})
    {
    }


    void stop()
    {
        m_initialized = false;
        m_slam->reset_config();

        cout << "Saving occupancy map to disk..." << endl;
        m_slam->save_occupancy_map_as_ppm("occupancy.ppm", true);
    }

    int query_supported_config(rs::device* device,
                               video_module_interface::supported_module_config& supported_config)
    {
        m_dev = device;

        clear_streams(supported_config);

        if (m_slam->query_supported_module_config(0, supported_config) < status_no_error)
        {
            cerr<<"error : failed to query the first supported module configuration" << endl;
            return -1;
        }

        auto device_name = device->get_name();
        auto is_current_device_valid = (strcmp(device_name, supported_config.device_name) == 0);
        if (!is_current_device_valid)
        {
            cerr<<"error : current device is not supported by the current supported module configuration" << endl;
            return -1;
        }

        memcpy(actual_slam_config.device_info.name, supported_config.device_name, std::strlen(supported_config.device_name));

        rs::source active_sources = get_source_type(supported_config);
        if(!check_motion_sensor_capability_if_required(device, active_sources))
        {
            cerr<<"error : current device is not supported motion events" << endl;
            return -1;
        }

        return 0;
    }

    void construct_slam_actual_config(video_module_interface::supported_module_config& supported_config)
    {
        for(int i = 0; i < (int) stream_type::max; ++i)
        {
            stream_type stream = stream_type(i);
            auto &supported_stream_config = supported_config[stream];

            if(!supported_stream_config.is_enabled)
            {
                continue;
            }

            rs::stream librealsense_stream = rs::utils::convert_stream_type(stream);

            video_module_interface::actual_image_stream_config &actual_stream_config = actual_slam_config[stream];
            actual_slam_config[stream].size.width = supported_stream_config.size.width;
            actual_slam_config[stream].size.height= supported_stream_config.size.height;
            actual_stream_config.frame_rate = supported_stream_config.frame_rate;

            actual_stream_config.intrinsics = rs::utils::convert_intrinsics(m_dev->get_stream_intrinsics(librealsense_stream));
            actual_stream_config.is_enabled = true;

        }

        rs::source active_sources = get_source_type(supported_config);
        if(is_motion_stream_requested(active_sources))
        {

            actual_slam_config[motion_type::accel].is_enabled = true;
            actual_slam_config[motion_type::gyro].is_enabled = true;

            auto _motion_intrinsics = m_dev->get_motion_intrinsics();
            actual_slam_config[motion_type::accel].intrinsics =
                rs::utils::convert_motion_device_intrinsics(_motion_intrinsics.acc);
            actual_slam_config[motion_type::gyro].intrinsics =
                rs::utils::convert_motion_device_intrinsics(_motion_intrinsics.gyro);
        }

        actual_slam_config[stream_type::fisheye].extrinsics_motion = rs::utils::convert_extrinsics(
                    m_dev->get_motion_extrinsics_from(rs::stream::fisheye));

        actual_slam_config[stream_type::fisheye].extrinsics = rs::utils::convert_extrinsics(
                    m_dev->get_extrinsics(rs::stream::depth, rs::stream::fisheye));

    }

    int init_slam(video_module_interface::supported_module_config& common_camera_config)
    {
        construct_slam_actual_config(common_camera_config);

        if(m_slam->set_module_config(actual_slam_config) < status_no_error)
        {
            cerr<<"error : failed to set the enabled module configuration" << endl;
            return -1;
        }
        m_slam->set_occupancy_map_height_of_interest(-0.5f, 1.0f);
        m_slam->set_auto_occupancy_map_building(true);

        slam_event_handler* scenePerceptionEventHandler = new slam_event_handler(m_module_listener);
        m_slam->register_event_handler(scenePerceptionEventHandler);

        slam_tracking_event_handler* trackingEventHandler = new slam_tracking_event_handler(m_module_listener);
        m_slam->register_tracking_event_handler(trackingEventHandler);

        m_initialized = true;

        return 0;
    }

    status process_sample_set_async(correlated_sample_set& sample_set)
    {
        return m_slam->process_sample_set(sample_set);
    }

    void restart()
    {
        m_slam->restart();
    }

    void get_camera_pose(PoseMatrix4f& pose)
    {
        m_slam->get_camera_pose(pose);
    }

    bool get_is_initialized()
    {
        return m_initialized;
    }

private:
    module_result_listener_interface* m_module_listener;
    std::unique_ptr<slam> m_slam;
    bool m_initialized;
    video_module_interface::actual_module_config actual_slam_config;

    rs::device* m_dev;
};
