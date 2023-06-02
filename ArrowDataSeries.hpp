//
// Created by weiqin xu on 04/11/2020.
//

#ifndef SUCCINCTEDGE_CATA_DATASERIES_HPP
#define SUCCINCTEDGE_CATA_DATASERIES_HPP

#include <arrow/array.h>
#include <arrow/array/builder_primitive.h>
#include <string>
#include <iostream>

using namespace std;
enum data_function {NONE, MAX, MIN, AVG};

template <typename T>
class ArrowDataSeries {
private:
    data_function func;
    int start = 0;
    int end = -1;
    int size = 0;
    arrow::NumericBuilder<T>  builder;
    std::shared_ptr<NumericArray<T>> queue = nullptr;
    T min, max, avg;
    bool opt_aggregation;
    T find_min(){
        T min = queue[0];
        for(int offset = 0; offset < queue.length(); offset++){
            if(min > queue[offset]){
                min = queue[offset];
            }
        }
        return min;
    }

    T find_max(){
        T max = queue[0];
        for(int offset = 0; offset < queue.length(); offset++){
            if(max < queue[offset]){
                max = queue[offset];
            }
        }
        return max;
    }

    T calc_avg(){
        T avg = 0;
        for(int offset = 0; offset < queue.length(); offset++){
            avg += queue[offset]/queue.length();
        }
        return avg;
    }
public:

    string out_put;
    ArrowDataSeries(data_function function = NONE, bool opt_aggre = true){
        opt_aggregation = opt_aggre;
        func = function;
    }

    ~DataSeries(){

    }

    void insert_element(T ele){
        if(queue == nullptr){
            builder.append(ele);
            end += 1;
        }
        else{
            if(start == 1){
                queue[0]=ele;
                end = 0;
            }
            else{
                end += 1;
                queue[end]=ele;
            }
        }

        if(opt_aggregation){
            if(end!=-1) {
                switch (func) {
                    case MIN:
                        min = min>ele? ele:min;
                        break;
                    case MAX:
                        max = max<ele? ele:max;
                        break;
                    case AVG:
                        avg = avg + (ele - avg)/(queue.length());
                        break;
                    default:
                        break;
                }

            }else{
                switch (func) {
                    case MIN:
                        min = ele;
                        break;
                    case MAX:
                        max = ele;
                        break;
                    case AVG:
                        avg = ele;
                        break;
                    default:
                        break;
                }
            }
        }
        refresh_output();
    }

    void remove_element(){

        if(end != -1){
            if(queue == nullptr){
                builder.Finish(queue);
            }

            T ele_pop = queue[start];

            if(start == queue.length()-1)
                start = 0;
            else
                start += 1;

            //cout << ele_pop << " " << func << " " << min << " " << max << " " << avg << " " << list_data->size() << endl;
            if(opt_aggregation){
                switch (func) {
                    case MIN:
                        min = (ele_pop == min)? find_min():min;
                        break;
                    case MAX:
                        max = (ele_pop == max)? find_max():max;
                        break;
                    case AVG:
                        avg = avg - (ele_pop - avg)/(queue.length());
                        break;
                    default:
                        break;
                }
            }

        }
        refresh_output();
    }

    void remove_all_element(){
        start = 0;
        end = -1;
        queue = nullptr;
        switch (func) {
            case MIN:
                break;
            case MAX:
                break;
            case AVG:
                avg = 0;
                break;
            default:
                break;
        }
        refresh_output();
    }

    void change_function(data_function function){
        //cout << "func:" << function << " ";
        func = function;
        if(end != -1){
            switch (func) {
                case MIN:
                    min = find_min();
                    break;
                case MAX:
                    max = find_max();
                    break;
                case AVG:
                    avg = calc_avg();
                    break;
                default:
                    break;
            }
        }

        //cout << "mode:" << func << " " << "size:" << list_data->size() << endl;
        refresh_output();
        //cout << "out:" << out_put << endl;
    }


    void refresh_output(){
        T out;
        //cout << "mode:" << func << " " << "size:" << list_data->size() << " ";
        if(end != -1){
            switch (func) {
                case MIN:
                    out = min;
                    break;
                case MAX:
                    out = max;
                    break;
                case AVG:
                    out = avg;
                    break;
                case NONE:
                    out = (queue[start]);
                default:
                    break;
            }
            //cout << "out" << out << endl;
            out_put = to_string(out);
            //out_put = to_string(out) + " " + to_string(list_data->size());
        }else{
            out_put = "NULL";
        }
        //cout << "out: " << out << endl;


    }

    string* get_output(){
        //cout << "out: " << out_put << endl;
        return &out_put;
    }

    void print_mode(){
        cout << func;
    }

    long long get_head_tail_difference(){
        return (queue[start] - queue[end]);
    }

    DataSeries<T>* get_data_series(){
        return this;
    }

    NumericArray<T> *get_data(){
        return queue;
    }

    bool not_null(){
        return (end != -1);
    }
};


#endif //SUCCINCTEDGE_CATA_DATASERIES_HPP
