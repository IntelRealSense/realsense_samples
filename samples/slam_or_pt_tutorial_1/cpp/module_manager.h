// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <iostream>
#include <functional>
#include <map>
#include <librealsense/rs.hpp>

#include "or/or_module.h"
#include "pt/pt_module.hpp"
#include "slam/slam_module.h"
#include "module_result_listener.h"

using namespace std;
using namespace rs::core;

class camera_data_listener
{
public:
    camera_data_listener() {}
    virtual ~camera_data_listener() {}

    virtual void on_camera_device(rs::device* dev) = 0;
    virtual void on_image_update(const stream_type stream, image_interface* image) = 0;
    virtual void on_motion_update(const motion_type motionType, correlated_sample_set& sample_set) = 0;
};

class camera_config_interface
{
public:
    camera_config_interface() {}
    virtual ~camera_config_interface() {}

    virtual bool is_slam_enabled() = 0;
    virtual video_module_interface::supported_module_config query_requested_camera_config() = 0;

};

class Camera_manager
{
public:
    Camera_manager(camera_data_listener* listener)
    {
        m_camera_listener = listener;
        m_ctx.reset(new rs::context());
        if (m_ctx->get_device_count() == 0)
        {
            // This class does not handle absence of camera. Callers should check for camera first by calling
            // Camera_manager::has_camera()
            cout << "Error: There are no RealSense devices connected." << endl << "Please connect a RealSense device and restart the application" << endl;
            m_camera_device = nullptr;
        }
        else
        {
            m_camera_device = m_ctx->get_device(0);
        }

        m_camera_listener->on_camera_device(m_camera_device);
    }

    static bool has_camera()
    {
        std::unique_ptr<rs::context> ctx;
        ctx.reset(new rs::context());
        return ctx->get_device_count() > 0;
    }

    void start_camera()
    {
        // Handle ctrl-c
        install_signal_handler();
        // Get the requirements to camera device from module_manager
        video_module_interface::supported_module_config m_common_camera_config =
            m_camera_configer->query_requested_camera_config();

        // Initialize camera stream based on MW requirement, and set up camera callbacks
        active_sources = get_source_type(m_common_camera_config);

        camera_data_listener* camera_data_listener = m_camera_listener;
        map<stream_type, function<void(rs::frame)>> stream_callback_per_stream;

        for(int i = 0; i < (int) stream_type::max; ++i)
        {
            stream_type stream = stream_type(i);
            auto &supported_stream_config = m_common_camera_config[stream];

            if(!supported_stream_config.is_enabled)
            {
                if(stream == stream_type::color)
                {
                    cout << "color stream not enabled" << endl;
                }
                if(stream == stream_type::fisheye)
                {
                    cout << "fisheye stream not enabled" << endl;
                }
                if(stream == stream_type::depth)
                {
                    cout << "depth stream not enabled" << endl;
                }

                continue;
            }
            else
            {
                if(stream == stream_type::color)
                {
                    cout << "color stream enabled" << endl;
                }
                if(stream == stream_type::depth)
                {
                    cout << "depth stream enabled" << endl;
                }
            }

            rs::format stream_format = rs::format::any;
            if(stream == stream_type::depth)
            {
                stream_format = rs::format::z16;
            }
            else
            {
                if(stream == stream_type::fisheye)
                {
                    stream_format = rs::format::raw8;
                }
                if(stream == stream_type::color)
                {
                    stream_format = rs::format::rgb8;//bgr8;
                }
            }
            rs::stream librealsense_stream = rs::utils::convert_stream_type(stream);
            m_camera_device->enable_stream(librealsense_stream, supported_stream_config.size.width,
                                           supported_stream_config.size.height, stream_format, supported_stream_config.frame_rate);

            bool slam_enabled = m_camera_configer->is_slam_enabled();

            stream_callback_per_stream[stream] = [=](rs::frame frame)
            {
                const auto timestampDomain = frame.get_frame_timestamp_domain();
                if(slam_enabled && stream != stream_type::color && rs::timestamp_domain::microcontroller != timestampDomain)
                {
                    cerr << "error: Junk time stamp in stream:" << (int)(stream)<< "\twith frame counter:" << frame.get_frame_number() << endl;
                    return;
                }

                auto image = image_interface::create_instance_from_librealsense_frame(
                                 frame, image_interface::flag::any);

                if(camera_data_listener)
                {
                    camera_data_listener->on_image_update(stream, image);
                }

                image->release();

            };

            m_camera_device->set_frame_callback(librealsense_stream, stream_callback_per_stream[stream]);

        }

        m_camera_device->set_option(rs::option::fisheye_strobe, 1);
        m_camera_device->set_option(rs::option::fisheye_external_trigger, 1);

        // Define callback to the motion events and set it, the callback lifetime assumes the module is available.
        function<void(rs::motion_data)> motion_callback;
        function<void(rs::timestamp_data)> timestamp_callback;
        if(is_motion_stream_requested(active_sources))
        {
            cout << "motion enabled " << endl;
            motion_callback = [camera_data_listener](rs::motion_data entry)
            {
                motion_type motionType = rs_to_sdk_motion_type(entry.timestamp_data.source_id);
                if(motionType != motion_type::max)
                {
                    correlated_sample_set sample_set = {};
                    sample_set[motionType].timestamp = entry.timestamp_data.timestamp;
                    sample_set[motionType].type = motionType;
                    sample_set[motionType].frame_number = entry.timestamp_data.frame_number;
                    sample_set[motionType].data[0] = entry.axes[0];
                    sample_set[motionType].data[1] = entry.axes[1];
                    sample_set[motionType].data[2] = entry.axes[2];

                    if(camera_data_listener)
                    {
                        camera_data_listener->on_motion_update(motionType, sample_set);
                    }

                }
            };

            timestamp_callback = [](rs::timestamp_data entry)
            {

            };

            m_camera_device->enable_motion_tracking(motion_callback, timestamp_callback);

        }
        else
        {
            cout << "Motion not enabled" << endl;
        }

        // Enable auto exposure for color stream
        m_camera_device->set_option(rs::option::color_enable_auto_exposure, 1);

        // Enable auto exposure for IR camera stream
        m_camera_device->set_option(rs::option::r200_lr_auto_exposure_enabled, 1);

        // Enable auto exposure for fisheye camera stream
        m_camera_device->set_option(rs::option::fisheye_color_auto_exposure, 1);

        m_camera_device->start(active_sources);
    }

    void set_camera_listener(camera_data_listener* listener)
    {
        m_camera_listener = listener;
    }

    void set_camera_configer(camera_config_interface* camera_configer)
    {
        m_camera_configer = camera_configer;
    }

    rs::device* get_camera_device()
    {
        return m_camera_device;
    }

private:
    camera_data_listener* m_camera_listener;
    camera_config_interface* m_camera_configer;
    std::unique_ptr<rs::context> m_ctx;
    rs::device *m_camera_device;
    rs::source active_sources;
};

class Module_manager : public camera_data_listener, camera_config_interface
{
public:
    Module_manager() : m_bOR_enabled(false), m_bPT_enalbed(false),
        m_bSLAM_enabled(false), m_stopped(false)
    {
    }

    void on_camera_device(rs::device* dev) override
    {
        m_device = dev;
    }

    video_module_interface::supported_module_config query_requested_camera_config() override
    {
        return m_common_camera_config;
    }

    bool is_slam_enabled() override
    {
        return m_bSLAM_enabled;
    }

    void on_image_update(const stream_type stream, image_interface* image) override
    {
        if(m_stopped) return;

        // slam
        if(m_bSLAM_enabled && m_slam_module->get_is_initialized() && stream != stream_type::color)
        {
            correlated_sample_set sample_set = {};
            sample_set[stream] = image;
            if(m_slam_module->process_sample_set_async(sample_set) < status_no_error)
            {
                cerr << "error: failed to process sample" << endl;
            }
        }

        if(stream == stream_type::depth || stream == stream_type::color)
        {
            if(m_bOR_enabled && m_or_module->get_or_is_ready())
            {
                if(m_or_sample_set[stream] == nullptr)
                {
                    m_or_sample_set[stream] = image;
                    m_or_sample_set[stream]->add_ref();

                    if(!m_stopped && m_or_sample_set[stream_type::color] != nullptr && m_or_sample_set[stream_type::depth] != nullptr)
                    {
                        int depth_frame_number = m_or_sample_set[stream_type::depth]->query_frame_number();
                        m_module_listener.on_object_recgnition_started(depth_frame_number);
                        m_or_module->run_or_processing(m_or_sample_set);
                    }
                }

            }

            if(m_bPT_enalbed && m_pt_module->get_pt_is_ready())
            {
                if(m_pt_sample_set[stream] == nullptr)
                {
                    m_pt_sample_set[stream] = image;
                    m_pt_sample_set[stream]->add_ref();

                    if(!m_stopped && m_pt_sample_set[stream_type::color] != nullptr && m_pt_sample_set[stream_type::depth] != nullptr)
                    {
                        m_pt_module->process_pt(m_pt_sample_set);
                    }
                }
            }

        }

    }

    void on_motion_update(const motion_type motionType, correlated_sample_set& sample_set) override
    {
        if(m_stopped || !m_bSLAM_enabled || !m_slam_module->get_is_initialized()) return;

        if(m_slam_module->process_sample_set_async(sample_set) < status_no_error)
        {
            cerr<<"error : failed to process sample" << endl;
        }
    }

    void set_slam_enabled(bool enabled)
    {
        m_bSLAM_enabled = enabled;
    }

    void set_object_recognition_enabled(bool enabled)
    {
        m_bOR_enabled = enabled;
    }

    void set_person_tracking_enabled(bool enabled)
    {
        m_bPT_enalbed = enabled;
    }

    void config_modules()
    {
        // Create MW wraps
        if(m_bOR_enabled)
        {
            m_or_module.reset(new or_module(&m_module_listener));

            m_or_sample_set[stream_type::depth] = nullptr;
            m_or_sample_set[stream_type::color] = nullptr;
        }

        if(m_bPT_enalbed)
        {
            m_pt_module.reset(new PT_module(&m_module_listener));

            m_pt_sample_set[stream_type::depth] = nullptr;
            m_pt_sample_set[stream_type::color] = nullptr;
        }

        if(m_bSLAM_enabled)
        {
            m_slam_module.reset(new slam_module(&m_module_listener));
        }

        m_common_camera_config = {};

    }

    // Do it to get m_common_camera_config, then pass it to camera manager later
    int init()
    {
        // Query camera config based on MWs enable situation
        if(m_bSLAM_enabled)
        {
            if(m_slam_module->query_supported_config(m_device, m_common_camera_config) == -1)
            {
                cerr << "init slam failed" << endl;
                return -1;
            }
            if(m_bOR_enabled)
            {
                // Set color stream config from OR module
                if(!m_or_module->negotiate_supported_cfg(m_common_camera_config))
                {
                    cout << "error: no common config found for slam and OR" << endl;
                    return -1;
                }
            }
            else if (m_bPT_enalbed)
            {
                // Set color stream config from PT module
                if(!m_pt_module->negotiate_supported_cfg(m_common_camera_config))
                {
                    cout << "error: no common config found for slam and PT" << endl;
                    return -1;
                }

            }
        }
        else if (m_bOR_enabled)
        {
            // Set color stream config from OR module
            if(m_or_module->query_supported_config(m_device, m_common_camera_config) == -1)
            {
                cout << "error: no common config found for PT and OR" << endl;
                return -1;
            }
        }
        else if (m_bPT_enalbed)
        {
            if(m_pt_module->query_supported_config(m_device, m_common_camera_config) == -1)
            {
                cout << "error: no common config found for PT" << endl;
                return -1;
            }
        }
        else
        {
            // Error: no mw enabled
            cout << "No MW enabled" << endl;
            return -1;
        }

        return 0;
    }

    int run()
    {
        cout << endl << "-------- Press Esc key to exit --------" << endl << endl;

        m_module_listener.start();

        if(m_bSLAM_enabled)
        {
            if(m_slam_module->init_slam(m_common_camera_config) == -1)
            {
                cerr<<"error : failed to set the enabled slam module configuration" << endl;
                return -1;
            }
        }

        if(m_bOR_enabled)
        {
            m_or_module->set_or_mode(or_module::ORMode::OR_LOCALIZATION);
            m_or_module->init_OR(m_common_camera_config, m_device);
        }

        if(m_bPT_enalbed)
        {
            m_pt_module->init_pt(m_common_camera_config, m_device);
        }


        if(m_module_listener.get_ui_request_restart())
        {
            m_slam_module->restart();
        }

        while(!is_key_pressed())
        {
            this_thread::sleep_for(chrono::milliseconds(50));
        }

        stop();

        return 0;
    }

    void stop()
    {
        cout << endl << "Stopping..." << endl << endl;
        m_stopped = true;

        // Stop all modules
        if(m_bPT_enalbed)
        {
            m_pt_module->stop();
        }

        if(m_bOR_enabled)
        {
            m_or_module->stop();
        }


        if(m_bSLAM_enabled)
        {
            m_slam_module->stop();
        }

        rs::source active_sources = get_source_type(m_common_camera_config);
        // Waiting all background work thread complete
        while((m_bPT_enalbed && m_pt_module->get_pt_running()) || (m_bOR_enabled && m_or_module->get_or_running()))
        {
            this_thread::sleep_for(chrono::milliseconds(50));
        }
        // Release sample set images after work thread completed
        release_images();

        // Stop Camera device
        m_device->stop(active_sources);

        m_module_listener.stop();

        // Recycle resources
        if(m_bOR_enabled)
        {
            m_or_module.reset(nullptr);
        }

        if(m_bSLAM_enabled)
        {
            m_slam_module.reset(nullptr);
        }
        if(m_bPT_enalbed)
        {
            m_pt_module.reset(nullptr);
        }
    }

    void release_images()
    {
        if(m_bPT_enalbed)
        {
            if (m_pt_sample_set[stream_type::color])
                m_pt_sample_set[stream_type::color]->release();
            if (m_pt_sample_set[stream_type::depth])
                m_pt_sample_set[stream_type::depth]->release();

            m_pt_sample_set[stream_type::color] = nullptr;
            m_pt_sample_set[stream_type::depth] = nullptr;
        }

        if(m_bOR_enabled)
        {
            if (m_or_sample_set[stream_type::color])
                m_or_sample_set[stream_type::color]->release();
            if (m_or_sample_set[stream_type::depth])
                m_or_sample_set[stream_type::depth]->release();

            m_or_sample_set[stream_type::color] = nullptr;
            m_or_sample_set[stream_type::depth] = nullptr;
        }
    }

private:
    module_consumer m_module_listener;
    std::unique_ptr<or_module> m_or_module;
    std::unique_ptr<PT_module> m_pt_module;
    std::unique_ptr<slam_module> m_slam_module;

    bool m_bOR_enabled;
    bool m_bPT_enalbed;
    bool m_bSLAM_enabled;
    bool m_stopped;

    correlated_sample_set m_or_sample_set;
    correlated_sample_set m_pt_sample_set;

    rs::device *m_device;

    video_module_interface::supported_module_config m_common_camera_config;
};
