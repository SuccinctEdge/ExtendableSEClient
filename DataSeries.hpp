//
// Created by weiqin xu on 04/11/2020.
//

#ifndef SUCCINCTEDGE_CATA_DATASERIES_HPP
#define SUCCINCTEDGE_CATA_DATASERIES_HPP

#include <list>
#include <string>
#include <iostream>

using namespace std;
enum data_function {NONE, MAX, MIN, AVG};

template <typename T>
class DataSeries {
private:
    list<T> *list_data;
    data_function func;

    T min, max, avg;
    bool opt_aggregation;
    T find_min(){
        T min = *(list_data->begin());
        for(auto iter = list_data->begin(); iter != list_data->end(); ++iter){
            if(min > *iter){
                min = *iter;
            }
        }
        return min;
    }

    T find_max(){
        T max = *(list_data->begin());
        for(auto iter = list_data->begin(); iter != list_data->end(); ++iter){
            if(max < *iter){
                max = *iter;
            }
        }
        return max;
    }

    T calc_avg(){
        T avg = 0;
        for(auto iter = list_data->begin(); iter != list_data->end(); ++iter){
            avg += (*iter)/list_data->size();
        }
        return avg;
    }
public:

    string out_put;
    DataSeries(data_function function = NONE, bool opt_aggre = true){
        list_data = new list<T>();
        opt_aggregation = opt_aggre;
        func = function;
    }

    ~DataSeries(){

    }

    void insert_element(T ele){


        if(opt_aggregation){
            if(!(list_data->empty())) {
                switch (func) {
                    case MIN:
                        min = min>ele? ele:min;
                        break;
                    case MAX:
                        max = max<ele? ele:max;
                        break;
                    case AVG:
                        avg = avg + (ele - avg)/(list_data->size());
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
        list_data->push_front(ele);
        //cout << "mode:" << func << " " << "size:" << list_data->size() << endl;
        //cout << "function" << func;
        refresh_output();
    }

    void remove_element(){
        if(list_data->size() > 0){
            T ele_pop = list_data->back();
            list_data->pop_back();
            //cout << ele_pop << " " << func << " " << min << " " << max << " " << avg << " " << list_data->size() << endl;
            if(opt_aggregation){
                if(list_data->size() > 0){
                    switch (func) {
                        case MIN:
                            min = (ele_pop == min)? find_min():min;
                            break;
                        case MAX:
                            max = (ele_pop == max)? find_max():max;
                            break;
                        case AVG:
                            avg = avg - (ele_pop - avg)/(list_data->size());
                            break;
                        default:
                            break;
                    }
                }
            }

        }
        refresh_output();
    }

    void remove_all_element(){
        list_data->clear();
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
        if(!(list_data->empty())){
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
        if(!(list_data->empty())){
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
                    out = (list_data->front());
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
        //cout << (long long)(list_data->back()) << " " << (long long)(list_data->front()) << " " << ((long long)(list_data->front())-(long long)(list_data->back())) << endl;
        //cout << "size: " << list_data->size() << endl;
        return ((long long)(list_data->front())-(long long)(list_data->back()));
    }

    DataSeries<T>* get_data_series(){
        return this;
    }
    
    list<T> *get_data(){
        return list_data;
    }

    bool not_null(){
        return (list_data->size() > 0);
    }
};


#endif //SUCCINCTEDGE_CATA_DATASERIES_HPP
