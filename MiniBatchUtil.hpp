//
// Created by weiqin xu on 12/12/2020.
//

#ifndef SUCCINCTEDGE_CATA_MINIBATCHUTIL_HPP
#define SUCCINCTEDGE_CATA_MINIBATCHUTIL_HPP

#include <vector>
#include <iostream>
#include "DataSeries.hpp"



using namespace std;

vector<double> calculate_mini_batch(vector<vector<double>>& mini_batch_data_window, data_function mini_batch_mode){
    vector<double> results;
    //cout << "batch size: " << mini_batch_data_window.size() << endl;
    results.push_back(mini_batch_data_window.back()[0]);
    for(long i = 1; i < mini_batch_data_window[0].size(); ++i){
        if(mini_batch_mode == MAX){
            double res_tmp = mini_batch_data_window[0][i];
            for(long j = 0; j < mini_batch_data_window.size(); ++j){
                if(mini_batch_data_window[j][i] > res_tmp){
                    res_tmp = mini_batch_data_window[j][i];
                }
            }
            results.push_back(res_tmp);
        }else if(mini_batch_mode == MIN){
            double res_tmp = mini_batch_data_window[0][i];
            for(long j = 0; j < mini_batch_data_window.size(); ++j){
                if(mini_batch_data_window[j][i] < res_tmp){
                    res_tmp = mini_batch_data_window[j][i];
                }
            }
            //cout << "push back " << res_tmp << endl;
            results.push_back(res_tmp);
        }else if(mini_batch_mode == AVG){
            double res_tmp = 0;
            for(long j = 0; j < mini_batch_data_window.size(); ++j){
                res_tmp += mini_batch_data_window[j][i]/(double)(mini_batch_data_window.size());
            }
            results.push_back(res_tmp);
        }else{
            results.push_back(mini_batch_data_window.back()[i]);
        }
    }
    return results;
}

#endif //SUCCINCTEDGE_CATA_MINIBATCHUTIL_HPP
