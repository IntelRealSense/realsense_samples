// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <jsoncpp/json/json.h>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "transporter.hpp"
#include "jpeg.hpp"
#include "concurrency.hpp"

using namespace std;
using namespace transport;
using namespace cv;

enum MsgType : uint8_t
{
    MapUpdate = 1,
    FishEye = 2,
    RGB = 3,
    PT = 4,
    OR = 5,
    MaxType = 6,
    Ack = 0xff
};

struct MsgAck
{
    MsgType type; // is ack
    MsgType type_ackd; //
};

struct MsgMapUpdate
{
    MsgType type;   ///< b[0]
    uint8_t _pad[1];///<  [1]
    uint16_t scale; ///< b[2-3] in millimeters
    int data[0];    ///< b[4]
};

enum MsgImageFormat : uint8_t
{
    Raw = 0,
    Jpeg = 1
};

struct MsgImage
{
    MsgType  type;    // b[0]
    uint8_t  format;  // b[1]
    uint16_t width;   // b[2-3]
    uint16_t height;  // b[4-5]
    uint8_t  _pad[2]; //  [6-7]
    uint64_t nanos;   // b[8-15]
    uint8_t  data[0]; // b[16-];
};

struct display_controls
{
    std::function<void()> reset;
    std::function<void()> stop;
    std::function<void(string)> track;
    std::function<void()> loading_rid_db;
};

static const string TAG = "Web UI: ";

class transporter_proxy : public transport::EventCallbacks
{
public:
    static transporter_proxy& getInstance(const char* path, int port, bool jpeg)
    {
        static transporter_proxy instance(path, port, jpeg); // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~transporter_proxy()
    {
        stop();
    }

    void stop()
    {
        image_queue.stop();
        transporter->disconnect();
    }

    void send_json_data(Json::Value msg)
    {
        transporter->send_data_string(json_writer.write(msg));
    }

    void set_control_callbacks(display_controls controls)
    {
        control_callbacks = controls;
    }

    void on_disconnect(Transporter& net)
    {
        control_callbacks.stop();
    }

    void on_data(Transporter& net, const uint8_t* data, size_t len)
    {
        if (len == 2 && data[0] == MsgType::Ack)
        {
            int type_ackd = data[1];
            if (type_ackd < MsgType::MaxType)
            {
                unacked_messages[type_ackd]--;
            }
        }
        else
        {
            cout << TAG << "received unhandled binary data: " << len << " bytes" << endl;
        }
    }

    void on_data_string(Transporter& net, std::string&& str)
    {
        Json::Value root;
        // check if json
        if (json_reader.parse(str, root, false))
        {
            // reset message:
            string&& type = root["type"].asString();
            if (type == "control")
            {
                string&& command = root["command"].asString();
                if (command == "reset")
                {
                    cout << "server: received reset message" << endl;
                    control_callbacks.reset();
                    return;
                }
                else if (command == "load_pt_db")
                {
                    cout << "server: received load_db message" << endl;
                    control_callbacks.loading_rid_db();
                    return;
                }
            }
            else if (type =="pt_track")
            {
                string&& command = root["command"].asString();
               // cout << TAG << "on_data_string: received request to track person " << command << endl;
                control_callbacks.track(command);
                return;
            }
            cout << TAG << "recieved unhandled valid JSON: \n<<< " << str << " >>>" << endl;
        }
        else
        {
            cout << TAG << "received non-JSON string: '" << str << "'" << endl;
        }
    }

    void on_occupancy(float scale, int count, const int* tiles)
    {
        unacked_messages[MsgType::MapUpdate]++;

        float mm = scale * 1000.0;
        if (mm - truncf(mm) != 0.0) std::cerr << "scale '" << scale << "'m not integral in mm!\n";

        MsgMapUpdate map;
        map.type = MsgType::MapUpdate;
        map.scale = mm;

        iovector iov[2] =
        {
            {&map, sizeof(map)},
            {(void *)tiles, sizeof(int) * count * 3}
        };

        transporter->send_data(iov, 2);
    }

    void on_fisheye_frame(uint64_t ts_micros, int width,
                          int height, const void* data)
    {
        static const int fps = 30;
        static const uint64_t frame_time_thresh = 1000.0 * 1000.0 / fps * 0.9;
        if (ts_micros - last_ts < frame_time_thresh)
        {
            cout << "very high frame rate" << endl;
            return; // skip high framerate
        }
        last_ts = ts_micros;

        unacked_messages[MsgType::FishEye]++;

        static const int scale_f = 2;
        const auto format = CompressionUtils::Format::RAW8;
        shared_ptr<char> scale_buf(new char[(width/scale_f+1) * (height/scale_f+1)]);
        uint16_t scale_w, scale_h;
        CompressionUtils::downscale(scale_f, format, (const char *)data,
                                    width, height, scale_buf.get(), scale_w, scale_h);

        image_queue.add([=]()
        {
            MsgImage md;
            md.type = MsgType::FishEye;
            md.format = MsgImageFormat::Raw;
            md.width = scale_w;
            md.height = scale_h;
            md.nanos = ts_micros;

            void * image_buf = scale_buf.get();
            size_t image_sz = scale_w * scale_h;

            // keep comp_buf on stack
            char comp_buf[use_jpeg ? image_sz : 0];
            if (use_jpeg)
            {
                md.format = MsgImageFormat::Jpeg;
                image_sz = jpeg_compressor.compress(scale_buf.get(), format, md.width, md.height,
                                                    comp_buf);
                image_buf = comp_buf;
            }

            iovector iov[2] =
            {
                {&md, sizeof(MsgImage)},
                {image_buf, image_sz}
            };
            transporter->send_data(iov, 2);
            std::this_thread::yield();
        });
    }


    void on_rgb_frame(uint64_t ts_micros, int width,
                      int height, const void* data)
    {
        int scale_f = 2;
        if(width == 320)
        {
            scale_f = 1;
        }
        const auto format = CompressionUtils::Format::RGB8;
        shared_ptr<char> scale_buf(new char[(width/scale_f+1) * (height/scale_f+1) * 3]);
        uint16_t scale_w, scale_h;

        scale_w = width/scale_f;
        scale_h = height/scale_f;

        if(scale_f == 1)
        {
            memcpy(scale_buf.get(), data, width * height * 3);
        }
        else
        {
            CompressionUtils::downscale(scale_f, format, (const char *)data,
                                        width, height, scale_buf.get(), scale_w, scale_h);
        }

        image_queue.add([=]()
        {
            MsgImage md;
            md.type = MsgType::RGB;
            md.format = MsgImageFormat::Raw;
            md.width = scale_w;
            md.height = scale_h;
            md.nanos = ts_micros;

            void * image_buf = scale_buf.get();
            size_t image_sz = scale_w * scale_h * 3;

            // keep comp_buf on stack
            char comp_buf[use_jpeg ? image_sz : 0];
            if (use_jpeg)
            {
                md.format = MsgImageFormat::Jpeg;
                image_sz = jpeg_compressor.compress(scale_buf.get(), format, md.width, md.height,
                                                    comp_buf);
                image_buf = comp_buf;
            }

            iovector iov[] =
            {
                {&md, sizeof(MsgImage)},
                {image_buf, image_sz}
            };
            transporter->send_data(iov, 2);
            std::this_thread::yield();
        });
    }


private:
    transporter_proxy(const char* path, int port, bool jpeg)
    {
        // Construct
        use_jpeg = jpeg;
        transporter = make_transporter(*this, path, port);
        jpeg_compressor.set_quality(80);

        // SStart transport
        start();
    }

    void start()
    {
        for (int i = 0; i < MaxType; i++) unacked_messages[i] = 0;
        transporter->connect();
        image_queue.start();
    }

    std::unique_ptr<Transporter> transporter;
    ConcurrencyUtils::WorkQueue image_queue;
    CompressionUtils::JpegCompressor jpeg_compressor;
    bool use_jpeg;

    // Server stuff
    display_controls control_callbacks;

    atomic_int unacked_messages[MaxType];
    const int kMaxUnackedFishEye = 3;
    const int kMaxUnackedMapUpdate = 3;

    Json::FastWriter json_writer;
    Json::Reader json_reader;

    uint64_t last_ts;
    uint64_t rgb_last_ts;


public:
    transporter_proxy(transporter_proxy const&) = delete;
    void operator=(transporter_proxy const&)    = delete;
};
