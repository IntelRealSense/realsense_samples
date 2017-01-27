// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>
#include <vector>
#include <string>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <mutex>
#include <condition_variable>
#include <deque>

#include <librealsense/rs.hpp>
#include "or_data_interface.h"
#include "or_configuration_interface.h"
#include "or_video_module_impl.h"
#include "rs_sdk.h"

#define ESC_KEY 27

using namespace std;
using namespace rs::core;
using namespace rs::object_recognition;

class or_utils
{
public:
    or_utils()
    {

    }
    ~or_utils()
    {

    }

    // Handling all camera initialization (projection, camera configuration, MW initialization)
    rs::core::status init_camera(image_info& colorInfo, image_info& depthInfo,
        or_video_module_impl& impl, or_data_interface** or_data,
        or_configuration_interface** or_configuration, bool use_default_config=false)
    {
        rs::core::status st = rs::core::status_no_error;
        m_frame_number=0;

        // Create context object
        m_ctx.reset(new rs::core::context);
        if (m_ctx == nullptr)
            return rs::core::status_process_failed;

        int deviceCount = m_ctx->get_device_count();
        if (deviceCount  == 0)
        {
            cout << "Error: Device is null. " << "There are no RealSense devices connected." << endl << "Please connect a RealSense device and restart the application" << endl;
            return rs::core::status_process_failed;
        }

        // Get pointer to the camera
        m_dev = m_ctx->get_device(0);

        // Request the first (index 0) supported module config.
        rs::core::video_module_interface::supported_module_config cfg;
        if(use_default_config)
        {
            st = impl.query_supported_module_config(0, cfg);
        }
        else
        {
            st = query_supported_config(impl, cfg);

        }
        if (st != rs::core::status_no_error)
            return st;

        m_color_width = cfg.image_streams_configs[(int)rs::stream::color].size.width;
        m_color_height = cfg.image_streams_configs[(int)rs::stream::color].size.height;

        cout << "color size, width: " << m_color_width << " height :" << m_color_height << endl;

        int dw = cfg.image_streams_configs[(int)rs::stream::depth].size.width;
        int dh = cfg.image_streams_configs[(int)rs::stream::depth].size.height;
        cout << "depth size, width: " << dw << " height :" << dh << endl;


        // Enables streams according to the supported configuration
        m_dev->enable_stream(rs::stream::color, cfg.image_streams_configs[(int)rs::stream::color].size.width,
                             cfg.image_streams_configs[(int)rs::stream::color].size.height,
                             rs::format::rgb8,
                             cfg.image_streams_configs[(int)rs::stream::color].frame_rate);

        m_dev->enable_stream(rs::stream::depth, cfg.image_streams_configs[(int)rs::stream::depth].size.width,
                             cfg.image_streams_configs[(int)rs::stream::depth].size.height,
                             rs::format::z16,
                             cfg.image_streams_configs[(int)rs::stream::depth].frame_rate);

        // Handling color image info (for later using)
        colorInfo.height = cfg.image_streams_configs[(int)rs::stream::color].size.height;
        colorInfo.width = cfg.image_streams_configs[(int)rs::stream::color].size.width;
        colorInfo.format = rs::core::pixel_format::rgb8;
        colorInfo.pitch = colorInfo.width * 3;
        m_colorInfo = colorInfo;

        // Handling depth image info (for later using)
        depthInfo.height = cfg.image_streams_configs[(int)rs::stream::depth].size.height;
        depthInfo.width = cfg.image_streams_configs[(int)rs::stream::depth].size.width;
        depthInfo.format = rs::core::pixel_format::z16;
        depthInfo.pitch = depthInfo.width * 2;

        // Start Camera device
        m_dev->start();

        // Enable auto exposure for color stream
        m_dev->set_option(rs::option::color_enable_auto_exposure, 1);

        // Enable auto exposure for Depth camera stream
        m_dev->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);

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

        //1. copy the extrinsics
        memcpy(&actualConfig.image_streams_configs[(int)rs::stream::color].extrinsics, &ext, sizeof(rs::extrinsics));
        core_ext =  rs::utils::convert_extrinsics(ext);

        //2. copy the color intrinsics
        memcpy(&actualConfig.image_streams_configs[(int)rs::stream::color].intrinsics, &colorInt, sizeof(rs::intrinsics));
        core_colorInt = rs::utils::convert_intrinsics (colorInt);

        //3. copy the depth intrinsics
        memcpy(&actualConfig.image_streams_configs[(int)rs::stream::depth].intrinsics, &depthInt, sizeof(rs::intrinsics));
        core_depthInt = rs::utils::convert_intrinsics(depthInt);

        // Handling projection
        rs::core::projection_interface* proj = rs::core::projection_interface::create_instance(&core_colorInt, &core_depthInt, &core_ext);
        actualConfig.projection = proj;

        // Setting the selected configuration (after projection)
        st=impl.set_module_config(actualConfig);
        if (st != rs::core::status_no_error)
            return st;

        // Create or data object
        *or_data = impl.create_output();

        // Create or data object
        *or_configuration = impl.create_active_configuration();

        m_sample_set = new rs::core::correlated_sample_set();

        m_sample_set->images[(int)rs::stream::color]=nullptr;
        m_sample_set->images[(int)rs::stream::depth]=nullptr;

        return rs::core::status_no_error;

    }

    // Get rs::core::correlated_sample_set* from the camera (encapsulates all conversion staff)
    correlated_sample_set* get_sample_set(image_info& colorInfo,image_info& depthInfo)
    {
        m_dev->wait_for_frames();

        //get color and depth buffers
        const void* colorBuffer =  m_dev->get_frame_data(rs::stream::color);
        const void* depthBuffer = m_dev->get_frame_data(rs::stream::depth);

        // Release images from the previous frame
        release_images();

        // Create images from buffers
        rs::core::image_interface::image_data_with_data_releaser color_container(colorBuffer);
        rs::core::image_interface::image_data_with_data_releaser depth_container(depthBuffer);
        auto colorImg = rs::core::image_interface::create_instance_from_raw_data( &colorInfo, color_container, rs::core::stream_type::color, rs::core::image_interface::any,m_frame_number, (uint64_t)m_dev->get_frame_timestamp(rs::stream::color));
        auto depthImg = rs::core::image_interface::create_instance_from_raw_data( &depthInfo, depth_container, rs::core::stream_type::depth, rs::core::image_interface::any,m_frame_number, (uint64_t)m_dev->get_frame_timestamp(rs::stream::depth));

        // Build sample set with both images
        m_sample_set->images[(int)rs::stream::color] = colorImg;
        m_sample_set->images[(int)rs::stream::depth] = depthImg;
        m_frame_number++;

        return m_sample_set;
    }

    void copy_color_to_cvmat(cv::Mat& CVColor)
    {
        memcpy(CVColor.data, m_color_buffer, CVColor.elemSize() * CVColor.total());

        //opencv display working with BGR.
        cv::cvtColor(CVColor,CVColor,CV_RGB2BGR);
    }

    // Stop Camera device
    void stop_camera()
    {
        m_dev->stop();
        release_images();
        delete m_sample_set;
    }

    int get_frame_number()
    {
        return m_frame_number;
    }

    int get_color_width()
    {
        return m_color_width;
    }

    int get_color_height()
    {
        return m_color_height;
    }

    // Query suitable supported_module_config
    rs::core::status query_supported_config(or_video_module_impl& or_impl,
        video_module_interface::supported_module_config& supported_config)
    {
        rs::core::status  st = status::status_no_error;
        int i = 0;
        while(true)
        {
            rs::core::status st = or_impl.query_supported_module_config(i, supported_config);
            if (st != rs::core::status_no_error)
            {
                std::cout << "no more supported config available for OR" << std::endl;
                break;
            }

            if(supported_config.image_streams_configs[(int)rs::stream::color].size.width == 640
                    && supported_config.image_streams_configs[(int)rs::stream::depth].size.width == 320)
            {
                return st;
            }

            i++;
        }
        st = or_impl.query_supported_module_config(0, supported_config);

        return st;
    }

    // Query object name list from library
    void query_object_name_list(vector<string>& obj_name_list,
        rs::object_recognition::or_configuration_interface* or_configuration)
    {
        int i = 0;
        while(1)
        {
            string objName = or_configuration->query_object_name_by_id(i);
            if(i > 50 || objName.empty())
            {
                break;
            }
            else
            {
                obj_name_list.push_back(objName);
                i++;
            }
        }
    }

    bool user_request_exit(void)
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

        int bytesWaiting=0,data=0;
        ioctl(STDIN, FIONREAD, &bytesWaiting);
        if(bytesWaiting)
            data=getc(stdin);

        return data == ESC_KEY;
    }

protected:
    std::shared_ptr<rs::core::context_interface> m_ctx;
    rs::device* m_dev;
    rs::core::correlated_sample_set* m_sample_set;
    rs::core::image_info m_colorInfo;
    void* m_color_buffer;
    int m_frame_number;

    int m_color_width;
    int m_color_height;

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
};

// Implementation of a simple blocking concurrent queue
template <typename T>
class blocking_queue
{
public:
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            queue.push_front(value);
        }
        this->condition.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->condition.wait(lock, [=]{ return !this->queue.empty(); });
        T rc(std::move(this->queue.back()));
        this->queue.pop_back();
        return rc;
    }

private:
    std::mutex              mutex;
    std::condition_variable condition;
    std::deque<T>           queue;
};
