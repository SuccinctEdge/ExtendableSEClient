//
// Created by weiqin xu on 19/11/2020.
//

#ifndef SUCCINCTEDGE_CATA_DATAWINDOWUTIL_HPP
#define SUCCINCTEDGE_CATA_DATAWINDOWUTIL_HPP

#include "RDFStorage.hpp"
#include <unistd.h>

#define CHECK_TIME_INTERVAL_US 100

enum WindowMode {SLIDING, TUMBLING};

void dataWindowControl(RDFStorage &test, mutex &data_update_lock, long& window_size, bool &stop_signal, WindowMode &mode, bool& tumbling_out_flag){

    while(!(stop_signal)){
        if(mode == SLIDING){
            /*
            while(test.get_data_head_tail_difference_with_index(0) > window_size){

                test.drop_numeric_data(data_update_lock);
                //cout << "difference: " << test.get_data_head_tail_difference_with_index(0) << endl;
            }*/
            test.check_windows_and_drop_data(window_size, data_update_lock);

        }else if(mode == TUMBLING){
            //cout << test.get_data_head_tail_difference_with_index(0) << " " << window_size << endl;
            /*
            if(test.get_data_head_tail_difference_with_index(0) > window_size){
                tumbling_out_flag = true;
                while(tumbling_out_flag){
                    //usleep(100);
                }
                test.drop_all_numeric_data(data_update_lock);
            }*/
            test.check_windows_and_drop_all_data(window_size,data_update_lock, tumbling_out_flag);
        }


        //cout << "difference: " << test.get_data_head_tail_difference_with_index(0) << endl;


        usleep(CHECK_TIME_INTERVAL_US);
    }
}

#endif //SUCCINCTEDGE_CATA_DATAWINDOWUTIL_HPP
