// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <iomanip>

#include "camera_utils.h"
#include "console_view.h"

using namespace std;

int main(int argc,char* argv[])
{
    rs::core::status st;
    camera_utils cu;
    Console_View console_view;

    rs::core::image_info colorInfo,depthInfo;
    rs::object_recognition::or_video_module_impl impl;
    rs::object_recognition::or_data_interface* or_data = nullptr;
    rs::object_recognition::or_configuration_interface* or_configuration = nullptr;
    cu.init_camera(colorInfo,depthInfo,impl,&or_data,&or_configuration);

    //set roi for recognition
    or_configuration->set_roi(rs::core::rectF32{0.25,0.25,0.5,0.5});
    or_configuration->apply_changes();

    //declare data structure and size for results
    rs::object_recognition::recognition_data* recognition_data = nullptr;
    int array_size = 0;

    while (true)
    {
        rs::core::correlated_sample_set* sample_set = cu.get_sample_set(colorInfo,depthInfo);

        // after the sample is ready we can process the frame
        if (cu.get_frame_number()%50 == 0)
        {
            st = impl.process_sample_set_sync(sample_set);
            if (st != rs::core::status_no_error)
                return 1;

            //retrieve recognition data from the or_data object
            st =or_data->query_single_recognition_result(&recognition_data, array_size);
            if (st != rs::core::status_no_error)
                return 1;
        }

        //print out the recognition results
        if (recognition_data)
        {
            console_view.draw_results(recognition_data, array_size,or_configuration);
        }

    }
    cu.stop_camera();

    return 0;
}
