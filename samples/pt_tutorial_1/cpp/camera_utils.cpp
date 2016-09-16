// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "camera_utils.h"
#include <iostream>

#include <string>
#include <sys/stat.h>

using namespace std;

camera_utils::camera_utils() : m_mode(LIVE_STREAM)
{
}

camera_utils::~camera_utils()
{
}

rs::core::status camera_utils::init_camera(rs::core::video_module_interface::actual_module_config& actualConfig)
{
    rs::core::status st;

    //Initializing depth resolution and color resolution
    color_resolution = RESOLUTION_VGA;
    depth_resolution = RESOLUTION_QVGA;

    //create contex object
    switch(m_mode)
    {
    case RECORD:
        m_ctx.reset(new rs::record::context(m_filename.c_str()));
        break;

    case PLAYBACK:
        m_ctx.reset(new rs::playback::context(m_filename.c_str()));
        break;

    default:
    case LIVE_STREAM:
        m_ctx.reset(new rs::core::context);
        break;
    }
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

    m_dev->start();

    //enable auto exposure for color stream
    m_dev->set_option(rs::option::color_enable_auto_exposure, 1);

    //enable auto exposure for Depth camera stream
    m_dev->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);

    return rs::core::status_no_error;
}

void camera_utils::init_person_tracking(rs::person_tracking::person_tracking_video_module_interface **ptModule)
{
    wstring dataDir = GetDataFilesPath();
    *ptModule = rs::person_tracking::person_tracking_video_module_factory::create_person_tracking_video_module(dataDir.c_str());
}

void camera_utils::stop_camera()
{
    m_dev->stop();
}

int camera_utils::get_resolution_width(Resolution res)
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

int camera_utils::get_resolution_height(Resolution res)
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

wstring camera_utils::GetDataFilesPath()
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

bool camera_utils::IsStreamCompatible(rs::core::stream_type stream, rs::format  format, int width, int height, int frameRate)
{
    bool frameRateOK = (frameRate == 30);
    bool colorFormatOK = false;
    bool resolutionOK = false;
    switch (stream)
    {
    case rs::core::stream_type::color:
        colorFormatOK = (format == rs::format::rgb8);
        /**
                    colorFormatOK = (format == rs::format::bgr8 ||
                                     format == rs::format::bgra8||
                                     format == rs::format::rgb8 ||
                                     format == rs::format::rgba8);
        **/
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
rs::core::video_module_interface::actual_module_config camera_utils::ConfigureCamera (rs::device* device)
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
            /*
                        cout << "\nfound stream=" << static_cast<int32_t>(stream ) << "\t" << width <<
                             "x" << height << "x" << frame_rate << "format" <<
                             static_cast<underlying_type<rs::format>::type>(format) << endl;
            */
            if (IsStreamCompatible(stream, format, width, height, frame_rate))
            {
                cout << "enabling: stream:" << librealsenseStream << ", "<< width << "x" << height << "x" << frame_rate << ", format:" << format << endl;
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

int camera_utils::GetNextFrame(rs::core::correlated_sample_set& sample_set)
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

        std::unique_ptr<rs::core::metadata_interface> metadata(rs::core::metadata_interface::create_instance());
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
                                 m_dev->get_frame_number(librealsense_stream),
                                 metadata.get());

    }
    return 0;
}

int8_t camera_utils::get_pixel_size(rs::format format)
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
}

