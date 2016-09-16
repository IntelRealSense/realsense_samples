// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include "camera_utils.h"
#include "console_view.h"

int main(int argc,char* argv[])
{
    rs::core::status st;
    char keyPressed=0;
    camera_utils cu;
    Console_View console_view;

    //declare data structure and size for results
    rs::object_recognition::localization_data* localization_data = nullptr;
    int array_size=0;

    rs::core::image_info colorInfo,depthInfo;
    rs::object_recognition::or_video_module_impl impl;
    rs::object_recognition::or_data_interface* or_data = nullptr;
    rs::object_recognition::or_configuration_interface* or_configuration = nullptr;
    cu.init_camera(colorInfo,depthInfo,impl,&or_data,&or_configuration);

    //change mode to localization
    or_configuration->set_recognition_mode(rs::object_recognition::recognition_mode::LOCALIZATION);
    //set the localization mechnizm to use CNN
    or_configuration->set_localization_mechanism(rs::object_recognition::localization_mechanism::CNN);
    //ignore all objects under 0.7 probabilty (confidence)
    or_configuration->set_recognition_confidence(0.7);
    //enabeling object center feature
    or_configuration->enable_object_center_estimation(true);

    st = or_configuration->apply_changes();
    if (st != rs::core::status_no_error)
        return st;

    while (true)
    {
        rs::core::correlated_sample_set* sample_set = cu.get_sample_set(colorInfo,depthInfo);

        // after the sample is ready we can process the frame as well
        st = impl.process_sample_set_sync(sample_set);
        if (st != rs::core::status_no_error)
            return 1;

        //retrieve recognition data from the or_data object
        st =or_data->query_localization_result(&localization_data, array_size);
        if (st != rs::core::status_no_error)
            return 1;

        if(localization_data && array_size != 0)
        {
            console_view.draw_results(localization_data, array_size,or_configuration);
        }
    }

    cu.stop_camera();

    return 0;
}
