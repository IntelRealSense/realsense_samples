// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <librealsense/rs.hpp>
#include <opencv2/core.hpp>
#include <memory>
#include "rs_sdk.h"
#include "or_data_interface.h"
#include "or_configuration_interface.h"
#include "or_video_module_impl.h"
#include "person_tracking_video_module_factory.h"

#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <fstream>
#include <iostream>

#include <string>
#include <sys/stat.h>

using namespace std;


extern "C"
{
#ifndef PERSON_TRACKING_DATA_FILES
#define PERSON_TRACKING_DATA_FILES "/usr/share/librealsense/pt/data/"
#endif

#define ESC_KEY 27

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

    class pt_utils
    {
    public:
        pt_utils()
        {

        }
        ~pt_utils()
        {

        }

        std::wstring GetDataFilesPath()
        {
            struct stat stat_struct;
            if(stat(PERSON_TRACKING_DATA_FILES, &stat_struct) != 0)
            {
                cerr <<  "Failed to find person tracking data files at " <<  PERSON_TRACKING_DATA_FILES << endl;
                cerr <<  "Please check that you run sample from correct directory" << endl;
                exit(EXIT_FAILURE);
            }

            string person_tracking_data_files = PERSON_TRACKING_DATA_FILES;
            int size = person_tracking_data_files.length();
            wchar_t wc_person_tracking_data_files[size + 1];
            mbstowcs(wc_person_tracking_data_files, person_tracking_data_files.c_str(), size + 1);

            return wstring(wc_person_tracking_data_files);
        }

        bool IsStreamCompatible(rs::core::stream_type stream, rs::format  format,
                                int width, int height, int frameRate)
        {
            bool frameRateOK = (frameRate == 30);
            bool colorFormatOK = false;
            bool resolutionOK = false;
            switch (stream)
            {
            case rs::core::stream_type::color:
                colorFormatOK = (format == rs::format::rgb8);
                resolutionOK = (width == get_resolution_width(color_resolution)) && (height == get_resolution_height(color_resolution)); //VGA resolution for color
                break;
            case rs::core::stream_type::depth:
                colorFormatOK =  (format == rs::format::z16);
                resolutionOK = (width == get_resolution_width(depth_resolution)) && (height == get_resolution_height(depth_resolution)); //QVGA resolution for depth
                break;
            default:
                return  false;
            }

            return  colorFormatOK && resolutionOK && frameRateOK;
        }

        // Config Camera device with recommended configuration from PT
        rs::core::video_module_interface::actual_module_config ConfigureCamera (rs::device* device)
        {
            rs::core::video_module_interface::actual_module_config actualModuleConfig = {};

            vector<rs::core::stream_type> possible_streams = {rs::core::stream_type::depth,
                                                              rs::core::stream_type::color //person tracking uses only color & depth
                                                             };
            for (auto &stream : possible_streams)
            {
                rs::stream librealsenseStream = rs::utils::convert_stream_type(stream);
                int streamModeCount = device->get_stream_mode_count(librealsenseStream);
                int streamModeNb = 0;
                for(streamModeNb = 0; streamModeNb < streamModeCount; ++streamModeNb)
                {
                    //add logic for choosing of stream mode
                    int width, height, frame_rate;
                    rs::format format;
                    device->get_stream_mode(librealsenseStream, streamModeNb, width, height, format, frame_rate);

                    if (IsStreamCompatible(stream, format, width, height, frame_rate))
                    {
                        cout << "\nenabling: stream:" << librealsenseStream << ", "<< width << "x" << height << "x" << frame_rate << ", format:" << format << endl;
                        device->enable_stream(librealsenseStream, width, height, format, frame_rate);
                        rs::core::video_module_interface::actual_image_stream_config &actualStreamConfig = actualModuleConfig[stream];
                        actualStreamConfig.size.width = width;
                        actualStreamConfig.size.height = height;
                        actualStreamConfig.frame_rate = frame_rate;
                        actualStreamConfig.intrinsics = rs::utils::convert_intrinsics(
                                                            device->get_stream_intrinsics(librealsenseStream));
                        actualStreamConfig.extrinsics = rs::utils::convert_extrinsics(
                                                            device->get_extrinsics(rs::stream::depth, librealsenseStream));
                        actualStreamConfig.is_enabled = true;
                        break;
                    }

                }
                if (streamModeNb >= streamModeCount)
                {
                    throw  runtime_error("camera is not support requested format");
                }
            }
            return actualModuleConfig;
        }

        int GetNextFrame(rs::core::correlated_sample_set& sample_set)
        {
            m_dev->wait_for_frames();

            for(auto &stream :
                    {
                        rs::core::stream_type::color,  rs::core::stream_type::depth
                    })
            {
                rs::stream librealsense_stream = rs::utils::convert_stream_type(stream);
                int height = m_dev->get_stream_height(librealsense_stream);
                int width = m_dev->get_stream_width(librealsense_stream);
                rs::core::image_info info =
                {
                    width,
                    height,
                    rs::utils::convert_pixel_format(m_dev->get_stream_format(librealsense_stream)),
                    get_pixel_size(m_dev->get_stream_format(librealsense_stream)) * width
                };

                const void* frameData = m_dev->get_frame_data(librealsense_stream);
                if (!frameData)
                {
                    cerr << "frame data for " << librealsense_stream << "is null" << endl;
                    return -1;
                }

                sample_set[stream] = rs::core::image_interface::create_instance_from_raw_data(
                                         &info,
                                         frameData,
                                         stream,
                                         rs::core::image_interface::flag::any,
                                         m_dev->get_frame_timestamp(librealsense_stream),
                                         m_dev->get_frame_number(librealsense_stream));

            }
            return 0;
        }

        int8_t get_pixel_size(rs::format format)
        {
            switch(format)
            {
            case rs::format::any:
                return 0;
            case rs::format::z16:
                return 2;
            case rs::format::disparity16:
                return 2;
            case rs::format::xyz32f:
                return 4;
            case rs::format::yuyv:
                return 2;
            case rs::format::rgb8:
                return 3;
            case rs::format::bgr8:
                return 3;
            case rs::format::rgba8:
                return 4;
            case rs::format::bgra8:
                return 4;
            case rs::format::y8:
                return 1;
            case rs::format::y16:
                return 2;
            case rs::format::raw8:
                return 1;
            case rs::format::raw10:
                return 0;//not supported
            case rs::format::raw16:
                return 2;
            }
            return 0;
        }

        rs::core::status init_camera(rs::core::image_info& colorInfo, rs::core::image_info& depthInfo,rs::core::video_module_interface::actual_module_config& actualModuleConfig, rs::object_recognition::or_video_module_impl& impl,rs::object_recognition::or_data_interface** or_data, rs::object_recognition::or_configuration_interface** or_configuration)
        {
            //Initializing depth resolution and color resolution
            color_resolution = RESOLUTION_VGA;
            depth_resolution = RESOLUTION_QVGA;

            rs::core::status st;
            m_frame_number=0;

            //create context object
            m_ctx.reset(new rs::core::context);

            if (m_ctx == nullptr)
                return rs::core::status_process_failed;

            int deviceCount = m_ctx->get_device_count();
            if (deviceCount  == 0)
            {
                printf("No RealSense device connected.\n\n");
                return rs::core::status_process_failed;
            }

            //get pointer the the camera
            m_dev = m_ctx->get_device(0);

            //request the first (index 0) supported module config.
            rs::core::video_module_interface::supported_module_config cfg;
            st = impl.query_supported_module_config(0, cfg);
            if (st != rs::core::status_no_error)
            {
                printf("Failed to query OR supported module config.\n");
                exit(EXIT_FAILURE);
            }

            //configure camera to fit supported MW configuration
            actualModuleConfig = ConfigureCamera(m_dev);

            //configure projection
            rs::core::intrinsics color_intrin = rs::utils::convert_intrinsics(m_dev->get_stream_intrinsics(rs::stream::color));
            rs::core::intrinsics depth_intrin = rs::utils::convert_intrinsics(m_dev->get_stream_intrinsics(rs::stream::depth));
            rs::core::extrinsics extrinsics = rs::utils::convert_extrinsics(m_dev   ->get_extrinsics(rs::stream::depth, rs::stream::color));
            actualModuleConfig.projection = rs::core::projection_interface::create_instance(&color_intrin, &depth_intrin, &extrinsics);

            //setting the selected configuration (after projection)
            st=impl.set_module_config(actualModuleConfig);
            if (st != rs::core::status_no_error)
                return st;

            //create or data object
            *or_data = impl.create_output();

            //create or data object
            *or_configuration = impl.create_active_configuration();

            //handling color image info (for later using)
            colorInfo.width = get_resolution_width(color_resolution);
            colorInfo.height = get_resolution_height(color_resolution);
            colorInfo.format = rs::core::pixel_format::rgb8;
            colorInfo.pitch = colorInfo.width * 3;
            m_colorInfo = colorInfo;

            //handling depth image info (for later using)
            depthInfo.width = get_resolution_width(depth_resolution);
            depthInfo.height = get_resolution_height(depth_resolution);
            depthInfo.format = rs::core::pixel_format::z16;
            depthInfo.pitch = depthInfo.width * 2;

            m_dev->start();

            //enable auto exposure for color stream
            m_dev->set_option(rs::option::color_enable_auto_exposure, 1);

            //enable auto exposure for Depth camera stream
            m_dev->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);

            m_sample_set = new rs::core::correlated_sample_set();

            m_sample_set->images[(int)rs::stream::color]=nullptr;
            m_sample_set->images[(int)rs::stream::depth]=nullptr;

            return rs::core::status_no_error;
        }


        rs::core::status init_camera( rs::core::video_module_interface::actual_module_config& actualConfig)
        {
            //Initializing depth resolution and color resolution
            color_resolution = RESOLUTION_QVGA;
            depth_resolution = RESOLUTION_QVGA;

            //create context object
            m_ctx.reset(new rs::core::context);

            if (m_ctx == nullptr)
                return rs::core::status_process_failed;

            int deviceCount = m_ctx->get_device_count();
            if (deviceCount  == 0)
            {
                cerr << endl << "There are no RealSense devices connected" << endl;
                return rs::core::status_process_failed;
            }

            m_dev = m_ctx->get_device(0); //device memory managed by the context

            //configure camera and get parameters for person tracking video module
            actualConfig = ConfigureCamera(m_dev);

            //configure projection
            rs::core::intrinsics color_intrin = rs::utils::convert_intrinsics(m_dev->get_stream_intrinsics(rs::stream::color));
            rs::core::intrinsics depth_intrin = rs::utils::convert_intrinsics(m_dev->get_stream_intrinsics(rs::stream::depth));
            rs::core::extrinsics extrinsics = rs::utils::convert_extrinsics(m_dev->get_extrinsics(rs::stream::depth, rs::stream::color));
            actualConfig.projection = rs::core::projection_interface::create_instance(&color_intrin, &depth_intrin, &extrinsics);

            //enable auto exposure for color stream
            m_dev->set_option(rs::option::color_enable_auto_exposure, 1);

            //enable auto exposure for Depth camera stream
            m_dev->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);

            return rs::core::status_no_error;
        }

        void init_person_tracking(rs::person_tracking::person_tracking_video_module_interface **ptModule)
        {
            wstring dataDir = GetDataFilesPath();
            *ptModule = rs::person_tracking::person_tracking_video_module_factory::create_person_tracking_video_module(dataDir.c_str());
        }

        int user_request_exit(void)
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

        //start the device
        void start_camera()
        {
            m_dev->start();
        }

        //stop the device
        void stop_camera()
        {
            m_dev->stop();
        }

        rs::core::correlated_sample_set* get_sample_set(rs::core::image_info& colorInfo,rs::core::image_info& depthInfo)
        {
            m_dev->wait_for_frames();

            //get color and depth buffers
            const void* colorBuffer =  m_dev->get_frame_data(rs::stream::color);
            const void* depthBuffer = m_dev->get_frame_data(rs::stream::depth);
            m_color_buffer = const_cast<void*>(colorBuffer);

            //release images from the previous frame
            release_images();

            //create images from buffers
            rs::core::image_interface::image_data_with_data_releaser color_container(colorBuffer);
            rs::core::image_interface::image_data_with_data_releaser depth_container(depthBuffer);
            auto colorImg = rs::core::image_interface::create_instance_from_raw_data( &colorInfo, color_container, rs::core::stream_type::color, rs::core::image_interface::any,m_frame_number, (uint64_t)m_dev->get_frame_timestamp(rs::stream::color) );
            auto depthImg = rs::core::image_interface::create_instance_from_raw_data( &depthInfo, depth_container, rs::core::stream_type::depth, rs::core::image_interface::any,m_frame_number, (uint64_t)m_dev->get_frame_timestamp(rs::stream::depth) );

            //create sample from both images
            m_sample_set->images[(int)rs::stream::color] = colorImg;
            m_sample_set->images[(int)rs::stream::depth] = depthImg;
            m_frame_number++;

            return m_sample_set;
        }

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

        void copy_color_to_cvmat(cv::Mat& CVColor)
        {
            memcpy(CVColor.data,m_color_buffer, CVColor.elemSize() * CVColor.total());
        }

    protected:
        std::shared_ptr<rs::core::context_interface> m_ctx;
        rs::device* m_dev;
        rs::core::correlated_sample_set* m_sample_set;
        rs::core::image_info m_colorInfo;
        int m_frame_number;
        void* m_color_buffer;

        enum Resolution {RESOLUTION_QVGA, RESOLUTION_VGA, RESOLUTION_HD, RESOLUTION_FULLHD};
        Resolution depth_resolution, color_resolution;
        std::string m_filename;
        int get_resolution_width(Resolution res)
        {
            switch(res)
            {
            case RESOLUTION_QVGA:
                return 320;
            case RESOLUTION_VGA:
                return 640;
            case RESOLUTION_HD:
                return 1280;
            case RESOLUTION_FULLHD:
                return 1920;
            default:
                throw std::runtime_error("unknown value for resolution");
            }
        }
        int get_resolution_height(Resolution res)
        {
            switch(res)
            {
            case RESOLUTION_QVGA:
                return 240;
            case RESOLUTION_VGA:
                return 480;
            case RESOLUTION_HD:
                return 720;
            case RESOLUTION_FULLHD:
                return 1080;
            default:
                throw std::runtime_error("unknown value for resolution");
            }
        }
    };

}
