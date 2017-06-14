// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <librealsense/rs.hpp>
#include "or_data_interface.h"
#include "or_configuration_interface.h"
#include "or_video_module_impl.h"
#include <opencv2/core.hpp>
#include <memory>
#include <thread>
#include <mutex>
#include <future>

#include "../module_result_listener.h"

using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;

class or_module
{
public:
    or_module()
    {

    }
    or_module(module_result_listener_interface* module_listener) :
        m_module_listener(module_listener), or_data(nullptr), or_configuration(nullptr),
        or_initialized(false), or_is_running(false), m_mode(OR_RECOGNITION),
        m_stopped(false)
    {
    }

    ~or_module()
    {

    }
    rs::core::status init_OR(
        video_module_interface::supported_module_config& cfg, rs::device* dev)

    {
        rs::core::status st = rs::core::status_no_error;
        m_frame_number=0;
        m_dev = dev;

        // Handling color image info (for later using)
        colorInfo.height = cfg.image_streams_configs[(int)rs::stream::color].size.height;
        colorInfo.width = cfg.image_streams_configs[(int)rs::stream::color].size.width;
        colorInfo.format = rs::core::pixel_format::rgb8;//bgr8;
        colorInfo.pitch = colorInfo.width * 3;

        // Get the extrisics parameters from the camera
        rs::extrinsics ext  = m_dev->get_extrinsics(rs::stream::depth, rs::stream::color);
        rs::core::extrinsics core_ext;

        // Get color intrinsics
        rs::intrinsics colorInt = m_dev->get_stream_intrinsics(rs::stream::color);
        rs::core::intrinsics core_colorInt;

        // Get depth intrinsics
        rs::intrinsics depthInt = m_dev->get_stream_intrinsics(rs::stream::depth);
        rs::core::intrinsics core_depthInt;

        // After getting all parameters from the camera we need to set the actual_module_config
        rs::core::video_module_interface::actual_module_config actualConfig;

        // 1. copy the extrinsics
        memcpy(&actualConfig.image_streams_configs[(int)rs::stream::color].extrinsics, &ext, sizeof(rs::extrinsics));
        core_ext =  rs::utils::convert_extrinsics(ext);

        // 2. copy the color intrinsics
        memcpy(&actualConfig.image_streams_configs[(int)rs::stream::color].intrinsics, &colorInt, sizeof(rs::intrinsics));
        core_colorInt = rs::utils::convert_intrinsics (colorInt);

        // 3. copy the depth intrinsics
        memcpy(&actualConfig.image_streams_configs[(int)rs::stream::depth].intrinsics, &depthInt, sizeof(rs::intrinsics));
        core_depthInt = rs::utils::convert_intrinsics(depthInt);


        // Handling projection
        rs::core::projection_interface* proj = rs::core::projection_interface::create_instance(&core_colorInt, &core_depthInt, &core_ext);
        actualConfig.projection = proj;
        //setting the selected configuration (after projection)
        st=or_impl.set_module_config(actualConfig);
        if (st != rs::core::status_no_error)
        {
            cout << "OR set_module_config fail" << endl;
            return st;
        }

        // Create or data object
        or_data = or_impl.create_output();

        // Create or data object
        or_configuration = or_impl.create_active_configuration();

        m_sample_set = new rs::core::correlated_sample_set();

        m_sample_set->images[(int)rs::stream::color]=nullptr;
        m_sample_set->images[(int)rs::stream::depth]=nullptr;

        switch(m_mode)
        {
        case OR_LOCALIZATION:
            // Change mode to localization
            or_configuration->set_recognition_mode(rs::object_recognition::recognition_mode::LOCALIZATION);
            // Set the localization mechanism to use CNN
            or_configuration->set_localization_mechanism(rs::object_recognition::localization_mechanism::CNN);
            // Ignore all objects under 0.7 probability (confidence)
            or_configuration->set_recognition_confidence(0.7);
            // Enabling object center feature
            or_configuration->enable_object_center_estimation(true);
            break;

        case OR_RECOGNITION:
        default:
            // set roi for recognition
            int width = colorInfo.width;
            int height = colorInfo.height;
            or_configuration->set_roi(rs::core::rect{(int)(width/4),(int)(height/4),(int)(width/2),(int)(height/2)});
            break;
        }
        or_configuration->apply_changes();
        or_initialized = true;
        cout << "OR init complete" << endl;

        return st;
    }
    bool run_or_processing(rs::core::correlated_sample_set& or_sample_set)
    {
        std::unique_lock<std::mutex> mtx_lock(mtx);

        if(!or_initialized || or_is_running || m_stopped)
        {
            // Drop the frame
            mtx_lock.unlock();
            return false;
        }
        or_is_running = true;

        m_sample_set = &or_sample_set;

        if(m_mode == OR_LOCALIZATION)
        {
            // Run OR process in background thread
            std::thread localization_thread(&or_module::processing_localization, this);
            localization_thread.detach();
        }
        else if(m_mode == OR_RECOGNITION)
        {
            std::thread recognition_thread(&or_module::processing_recognition, this);
            recognition_thread.detach();
        }

        mtx_lock.unlock();

        return true;
    }

    void stop()
    {
        m_stopped = true;
    }

    int get_frame_number()
    {
        return m_frame_number;
    }

    int query_supported_config(rs::device* device,
                               video_module_interface::supported_module_config& supported_config)
    {
        int i = 0;
        while(true)
        {
            rs::core::status st = or_impl.query_supported_module_config(i, supported_config);
            if (st != rs::core::status_no_error)
            {
                cout << "no more supported config available for OR" << endl;
                return 0;
            }

            if(supported_config.image_streams_configs[(int)rs::stream::color].size.width == 640
                    && supported_config.image_streams_configs[(int)rs::stream::depth].size.width == 320)
            {
                supported_config[stream_type::color].is_enabled = true;
                supported_config[stream_type::depth].is_enabled = true;

                return 0;
            }

            i++;
        }

        return -1;
    }

    bool negotiate_supported_cfg(rs::core::video_module_interface::supported_module_config& slam_config)
    {
        bool found = false;
        int i = 5;
        int min_color_width = INT_MAX;
        int min_color_height = INT_MAX;

        // Check if the slam cfg is supported by OR
        while(i >= 0)
        {
            rs::core::video_module_interface::supported_module_config cfg = {};
            rs::core::status st = or_impl.query_supported_module_config(i, cfg);
            if (st != rs::core::status_no_error)
            {
                cout << "no more supported config available for OR" << endl;
                continue;
            }
            if(compare_config(slam_config, cfg))
            {
                found = true;
                int w = cfg.image_streams_configs[(int)rs::stream::color].size.width;
                int h = cfg.image_streams_configs[(int)rs::stream::color].size.height;

                if(w < min_color_width && h < min_color_height)
                {
                    min_color_width = w;
                    min_color_height = h;
                }
                break;

            }

            i--;
        }

        if(found)
        {
            slam_config.image_streams_configs[(int)rs::stream::color].size.width = min_color_width;
            slam_config.image_streams_configs[(int)rs::stream::color].size.height = min_color_height;

            slam_config[stream_type::color].size.height = min_color_width;
            slam_config[stream_type::color].size.height = min_color_height;
            slam_config[stream_type::color].is_enabled = true;
            slam_config[stream_type::color].frame_rate = 30.f;
        }
        return found;
    }

    enum ORMode { OR_RECOGNITION, OR_LOCALIZATION, OR_LOCALIZATION_TRACKING};
    void set_or_mode(enum ORMode mode)
    {
        m_mode = mode;
    }

    bool get_or_running()
    {
        return  or_is_running;
    }

    bool get_or_is_ready()
    {
        return or_initialized && !or_is_running && !m_stopped;
    }

protected:
    module_result_listener_interface* m_module_listener;
    or_data_interface* or_data;
    or_configuration_interface* or_configuration;
    bool or_initialized;
    bool or_is_running;
    ORMode m_mode = OR_RECOGNITION;
    bool m_stopped;

    rs::device* m_dev;
    rs::core::correlated_sample_set* m_sample_set;
    void* m_color_buffer;
    int m_frame_number;

    std::mutex mtx;
    or_video_module_impl or_impl;
    rs::core::image_info colorInfo;

    void release_images()
    {
        if (m_sample_set->images[(int)rs::stream::color])
        {
            m_sample_set->images[(int)rs::stream::color]->release();
            m_sample_set->images[(int)rs::stream::color]=nullptr;
        }

        if (m_sample_set->images[(int)rs::stream::depth])
        {
            m_sample_set->images[(int)rs::stream::depth]->release();
            m_sample_set->images[(int)rs::stream::depth] = nullptr;
        }
    }

    bool compare_config(
        video_module_interface::supported_module_config& cfg_1,
        video_module_interface::supported_module_config& cfg_2)
    {
        auto depth_1 =cfg_1.image_streams_configs[(int)rs::stream::depth];
        auto depth_2 =cfg_2.image_streams_configs[(int)rs::stream::depth];

        if(depth_1.size.width != depth_2.size.width)
        {
            return false;
        }
        if(depth_1.size.height != depth_2.size.height)
        {
            return false;
        }

        return true;
    }

    void processing_recognition()
    {
        rs::core::status st = rs::core::status_no_error;

        // Declare data structure and size for results
        rs::object_recognition::recognition_data* recognition_data = nullptr;
        int array_size = 0;
        {
            st = or_impl.process_sample_set(*m_sample_set);

            if (st != rs::core::status_no_error)
            {
                cout << "or_impl.process_sample_set_sync failed: " << st << endl;
                return ;
            }

            // Retrieve recognition data from the or_data object
            st =or_data->query_single_recognition_result(&recognition_data, array_size);
            if (st != rs::core::status_no_error)
                return ;

        }

        if(m_module_listener && !m_stopped)
        {
            m_module_listener->on_object_recgnition_finished(*m_sample_set,
                    recognition_data, array_size,or_configuration);
        }
        or_is_running = false;
    }

    void processing_localization()
    {
        rs::core::status st = rs::core::status_no_error;

        // Declare data structure and size for results
        rs::object_recognition::localization_data* localization_data = nullptr;
        int array_size=0;

        // After the sample is ready we can process the frame as well
        st = or_impl.process_sample_set(*m_sample_set);

        if (st != rs::core::status_no_error)
        {
            cout << "error: OR process_sample_set_sync fail" << endl;
            return ;
        }

        // Retrieve recognition data from the or_data object
        st =or_data->query_localization_result(&localization_data, array_size);
        if (st != rs::core::status_no_error)
            return ;

        if(m_module_listener && !m_stopped)
        {
            m_module_listener->on_object_localization_finished(*m_sample_set,localization_data, array_size,or_configuration);
        }

        or_is_running = false;
    }
};
