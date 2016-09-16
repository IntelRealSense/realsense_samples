// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "camera_utils.h"
#include "rs_sdk.h"
camera_utils::camera_utils() : m_mode(LIVE_STREAM)
{
}

camera_utils::~camera_utils()
{
}

rs::core::status camera_utils::init_camera(rs::core::image_info& colorInfo, rs::core::image_info& depthInfo,rs::object_recognition::or_video_module_impl& impl,
        rs::object_recognition::or_data_interface** or_data, rs::object_recognition::or_configuration_interface** or_configuration)
{
    rs::core::status st;
    m_frame_number=0;

    //create contex object
    if(m_mode == LIVE_STREAM)
    {
        m_ctx.reset(new rs::core::context);
    }
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

    //request the first (index 0) supproted module config.
    rs::core::video_module_interface::supported_module_config cfg;
    st = impl.query_supported_module_config(0, cfg);
    if (st != rs::core::status_no_error)
        return st;

    //enables streams according to the supported configuration
    m_dev->enable_stream(rs::stream::color, cfg.image_streams_configs[(int)rs::stream::color].min_size.width,
                         cfg.image_streams_configs[(int)rs::stream::color].min_size.height,
                         rs::format::bgr8,
                         cfg.image_streams_configs[(int)rs::stream::color].minimal_frame_rate);

    m_dev->enable_stream(rs::stream::depth, cfg.image_streams_configs[(int)rs::stream::depth].min_size.width,
                         cfg.image_streams_configs[(int)rs::stream::depth].min_size.height,
                         rs::format::z16,
                         cfg.image_streams_configs[(int)rs::stream::depth].minimal_frame_rate);

    //handling color image info (for later using)
    colorInfo.height = cfg.image_streams_configs[(int)rs::stream::color].min_size.height;
    colorInfo.width = cfg.image_streams_configs[(int)rs::stream::color].min_size.width;
    colorInfo.format = rs::core::pixel_format::bgr8;
    colorInfo.pitch = colorInfo.width * 3;
    m_colorInfo = colorInfo;

    //handling depth image info (for later using)
    depthInfo.height = cfg.image_streams_configs[(int)rs::stream::depth].min_size.height;
    depthInfo.width = cfg.image_streams_configs[(int)rs::stream::depth].min_size.width;
    depthInfo.format = rs::core::pixel_format::z16;
    depthInfo.pitch = depthInfo.width * 2;

    m_dev->start();

    //enable auto exposure for color stream
    m_dev->set_option(rs::option::color_enable_auto_exposure, 1);

    //enable auto exposure for Depth camera stream
    m_dev->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);

    // get the extrisics paramters from the camera
    rs::extrinsics ext  = m_dev->get_extrinsics(rs::stream::depth, rs::stream::color);
    rs::core::extrinsics core_ext;

    //get color intrinsics
    rs::intrinsics colorInt = m_dev->get_stream_intrinsics(rs::stream::color);
    rs::core::intrinsics core_colorInt;

    //get depth intrinsics
    rs::intrinsics depthInt = m_dev->get_stream_intrinsics(rs::stream::depth);
    rs::core::intrinsics core_depthInt;

    // after getting all parameters from the camera we need to set the actual_module_config
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


    // handling projection
    rs::core::projection_interface* proj = rs::core::projection_interface::create_instance(&core_colorInt, &core_depthInt, &core_ext);
    actualConfig.projection = proj;
    //setting the selected configuration (after projection)
    st=impl.set_module_config(actualConfig);
    if (st != rs::core::status_no_error)
        return st;

    //create or data object
    *or_data = impl.create_output();

    //create or data object
    *or_configuration = impl.create_active_configuration();

    m_sample_set = new rs::core::correlated_sample_set();

    m_sample_set->images[(int)rs::stream::color]=nullptr;
    m_sample_set->images[(int)rs::stream::depth]=nullptr;
}

rs::core::correlated_sample_set* camera_utils::get_sample_set(rs::core::image_info& colorInfo,rs::core::image_info& depthInfo)
{
    m_dev->wait_for_frames();

    //get color and depth buffers
    const void* colorBuffer =  m_dev->get_frame_data(rs::stream::color);
    const void* depthBuffer = m_dev->get_frame_data(rs::stream::depth);
    m_color_buffer = const_cast<void*>(colorBuffer);

    // release images from the prevoius frame
    release_images();
    //create images from buffers
    auto colorImg = rs::core::image_interface::create_instance_from_raw_data( &colorInfo, colorBuffer, rs::core::stream_type::color, rs::core::image_interface::any,m_frame_number, (uint64_t)m_dev->get_frame_timestamp(rs::stream::color), nullptr);
    auto depthImg = rs::core::image_interface::create_instance_from_raw_data( &depthInfo, depthBuffer, rs::core::stream_type::depth, rs::core::image_interface::any,m_frame_number, (uint64_t)m_dev->get_frame_timestamp(rs::stream::depth), nullptr);

    //create sample from both images

    m_sample_set->images[(int)rs::stream::color] = colorImg;
    m_sample_set->images[(int)rs::stream::depth] = depthImg;
    m_frame_number++;

    return m_sample_set;
}

void camera_utils::stop_camera()
{
    m_dev->stop();
    release_images();
    delete m_sample_set;
}
void camera_utils::copy_color_to_cvmat(cv::Mat& CVColor)
{
    memcpy(CVColor.data,m_color_buffer, CVColor.elemSize() * CVColor.total());
}

int camera_utils::get_frame_number()
{
    return m_frame_number;
}

void camera_utils::setFileIO(const std::string& filename, bool isRecord)
{
    m_filename = filename;
    m_mode = isRecord ? CameraMode::RECORD : CameraMode::PLAYBACK;
}

void camera_utils::release_images()
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
