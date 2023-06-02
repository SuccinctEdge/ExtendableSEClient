//
// Created by weiqin xu on 20/01/2020.
//

#ifndef SUCCINCT_EDGE_DATATYPESTORE_HPP
#define SUCCINCT_EDGE_DATATYPESTORE_HPP

#include <sdsl/wavelet_trees.hpp>
#include <string>
#include "Constant.hpp"
#include "JoinLine.hpp"
#include "BitMap.hpp"
#include "DatastructureConstant.hpp"
#include "DataSeries.hpp"
#include "parquet/arrow/writer.h"
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <cstdint>
#include <iostream>
#include <vector>


using namespace std;



class DataTypeStore {
private:
    WT_TYPE wt_ele0;
    BitMap *wt_ele1_bit_map;
    WT_TYPE wt_ele1;
    BitMap *wt_ele2_bit_map;
    vector<string> *vec_ele2;
    vector<DataSeries<double>> vec_ele2_data;


    long long range_search_string_data(long long index2_begin, long long index2_end, const string& ele2){
        if((*vec_ele2)[index2_begin].compare(ele2) == 0){
            return (long long)index2_begin;
        }else{
            if(index2_end - index2_begin <= 1)
                return -1;
            else{
                if((*vec_ele2)[(index2_begin + index2_end)/2].compare(ele2) <= 0)
                    return range_search_string_data((index2_begin + index2_end)/2, index2_end, ele2);
                else
                    return range_search_string_data(index2_begin, (index2_begin + index2_end)/2, ele2);
            }
        }
    }


public:
    DataTypeStore();
    DataTypeStore(std::vector<ID_TYPE>&  ele0_vector, std::vector<ID_TYPE>& ele1_vector, std::vector<string>&  ele2_vector);

    void load(std::vector<ID_TYPE>&  ele0_vector, std::vector<ID_TYPE>& ele1_vector, std::vector<string>&  ele2_vector);
    void store_to_disk(const string &path);

    long long get_head_tail_difference_with_index(long index);
    void insert_data(vector<long>& data_indexes, vector<double>& data);
    void insert_data(vector<long>& data_indexes, vector<string>& data);
    void drop_data();
    void drop_data_with_index(long index);
    void drop_all_data();
    void drop_all_data_with_index(long index);
    void change_mode(int index, data_function function);
    void all_change_mode(data_function function);
    void persist_all();
    void series_to_arrow_table(const vector<DataSeries<double>> series, std::shared_ptr<arrow::Table>* table);

    std::vector<JoinLine> look_up_ele1(ID_TYPE ele0, const string& ele2);
    std::vector<JoinLine> look_up_ele1_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end, const string& ele2);

    std::vector<JoinLine> look_up_ele2(ID_TYPE ele0, ID_TYPE ele1);
    std::vector<JoinLine> look_up_ele2_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end, ID_TYPE ele1);

    std::vector<JoinLine> look_up_ele12(ID_TYPE ele0);
    std::vector<JoinLine> look_up_ele12_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end);

    std::vector<JoinLine> verify_triple(ID_TYPE ele0, ID_TYPE ele1, const string& ele2);
    std::vector<JoinLine> verify_triple_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end, ID_TYPE ele1, const string& ele2);

    unsigned long long look_up_ele0_num(ID_TYPE ele0);
    vector<long> get_indexes_with_data_names(vector<string> &cols){
        vector<long> indexes;
        for(long i = 0; i < cols.size(); ++i){
            bool find = false;
            for(long j = 0; j < vec_ele2->size(); ++j){
                if(cols[i] == (*vec_ele2)[j]){
                    indexes.push_back(j);
                    find = true;
                    break;
                }
            }
            if(!find){
                cerr << "Data name " << cols[i] << " doesn't exist!" << endl;
            }
        }
        return indexes;
    }

    bool data_serie_not_null(long index){
        return vec_ele2_data[index].not_null();
    }

    void extend(std::vector<ID_TYPE> &ele0_vector, std::vector<ID_TYPE> &ele1_vector,std::vector<string> &ele2_vector);
};


#endif //SUCCINCT_EDGE_DATATYPESTORE_HPP
