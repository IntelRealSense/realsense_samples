// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <librealsense/rs.hpp>
#include <opencv2/core.hpp>
#include <memory>
#include "rs_sdk.h"
#include "person_tracking_video_module_factory.h"

extern "C"
{
    // Forward declarations
    namespace rs
    {
    namespace core
    {
    class context_interface;
    class correlated_sample_set;
    }
    class device;
    }

    class camera_utils
    {
    public:
        camera_utils();
        ~camera_utils();

        // handling all camera initilzation (projection, camera configuration, MW intilization)
        std::wstring GetDataFilesPath();
        bool IsStreamCompatible(rs::core::stream_type stream, rs::format  format, int width, int height, int frameRate);
        rs::core::video_module_interface::actual_module_config ConfigureCamera (rs::device* device);
        int GetNextFrame(rs::core::correlated_sample_set& sample_set);
        int8_t get_pixel_size(rs::format format);

        rs::core::status init_camera( rs::core::video_module_interface::actual_module_config& actualConfig);
        void init_person_tracking(rs::person_tracking::person_tracking_video_module_interface **ptModule);

        //stop the device
        void stop_camera();

    protected:
        std::shared_ptr<rs::core::context_interface> m_ctx;
        rs::device* m_dev;
        int m_frame_number;
        //int frame_rate;

        enum CameraMode { LIVE_STREAM, PLAYBACK, RECORD};
        enum Resolution {RESOLUTION_QVGA, RESOLUTION_VGA, RESOLUTION_HD, RESOLUTION_FULLHD};
        CameraMode m_mode;
        Resolution depth_resolution, color_resolution;
        std::string m_filename;
        int get_resolution_width(Resolution res);
        int get_resolution_height(Resolution res);
    };

}
