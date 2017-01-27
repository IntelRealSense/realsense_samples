// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#pragma once

#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <vector>
#include <cassert>


namespace CompressionUtils
{

template<typename T>
void downscale_nearest(uint16_t factor, T const* in, const uint16_t in_w, const uint16_t in_h, T *out, uint16_t& out_w, uint16_t& out_h)
{
    T *p = out;
    for (unsigned int j=0; j<in_h; j+=factor)
    {
        for (unsigned int i=0; i<in_w; i+=factor)
        {
            *(p++) = in[j*in_w + i];
        }
    }
    out_w = MAX(in_w/factor, 1);
    out_h = MAX(in_h/factor, 1);
}

template<size_t s>
struct Bytes
{
    unsigned char b[s];
};

enum Format
{
    RGB8,
    RAW8,
    Y8,
};

void downscale(uint16_t factor, Format format, char const* inbuf, uint16_t width, uint16_t height, char * outbuf, uint16_t& out_width, uint16_t& out_height)
{

    if (factor > 1)
    {
        if (format == RGB8)
        {
            downscale_nearest<Bytes<3>>(factor, (Bytes<3>*)inbuf, width, height, (Bytes<3>*)outbuf, out_width, out_height);
        }
        else if (format == RAW8 || format == Y8)
        {
            downscale_nearest<Bytes<1>>(factor, (Bytes<1>*)inbuf, width, height, (Bytes<1>*)outbuf, out_width, out_height);
        }
    }
}

class JpegCompressor
{
private:
    jpeg_compress_struct cinfo = {0};
    jpeg_error_mgr jerr = {0};
    int quality = 50;

public:

    JpegCompressor()
    {
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

    }

    ~JpegCompressor()
    {
        jpeg_destroy_compress(&cinfo);
    }

    void set_quality(int qual)
    {
        assert(qual >= 0 && qual <= 100);
        quality = qual;
    }


    // only support rgb8 or y8 or raw8 format;
    // taking form libjpeg example.c
    size_t downscale_and_compress(uint16_t scale_factor, char const* inbuf, Format format, uint16_t width, uint16_t height, char * outbuf)
    {

        size_t downscale_size = (width/scale_factor + 1) * (height/scale_factor + 1);
        char downscale_buf[downscale_size];
        uint16_t jpeg_w, jpeg_h;
        downscale(scale_factor, format, inbuf, width, height, downscale_buf, jpeg_w, jpeg_h);

        return compress(downscale_buf, format, jpeg_w, jpeg_h, outbuf);
    }

    size_t compress(char const* inbuf, Format format, uint16_t width, uint16_t height, char * outbuf)
    {
        J_COLOR_SPACE color_space;
        int input_components;

        switch (format)
        {
        case Format::RGB8:
            color_space = JCS_RGB;
            input_components = 3;
            break;
        case Format::Y8:
        case Format::RAW8:
            color_space = JCS_GRAYSCALE;
            input_components = 1;
            break;
        default:
            assert(false);
            return -1;
            // not supported
        }


        int row_stride = width*input_components;
        unsigned long outsize;
        jpeg_mem_dest(&cinfo, (unsigned char **)&outbuf, &outsize);

        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.in_color_space = color_space;
        cinfo.input_components = input_components;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);

        jpeg_start_compress(&cinfo, true);

        JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
        while (cinfo.next_scanline < cinfo.image_height)
        {
            row_pointer[0] = (JSAMPLE *)&inbuf[cinfo.next_scanline * row_stride];
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        jpeg_finish_compress(&cinfo);

        return outsize;
    }
};
}

