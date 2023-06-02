//
// Created by weiqin xu on 11/11/2019.
//

#include "JoinLine.hpp"

JoinLine::JoinLine(){}

JoinLine::~JoinLine() {
}


void JoinLine::add(ID_TYPE id, VariableType var_type) {
    switch(var_type){
        case INS:
            instances.push_back(id);
            break;
        case PRO:
            properties.push_back(id);
            break;
        case CON:
            concepts.push_back(id);
            break;
        default:
            cerr << "Wrong variable type!" << endl;
    }
}

void JoinLine::add(string* str, VariableType var_type) {
    if(var_type == STR_DATA){
        string_datas.push_back(str);
    }else{
        cerr << "Wrong variable type!" << endl;
    }
}

void JoinLine::add(DataSeries<double> *ds) {
    dataserie_list.push_back(ds);
}




vector<ID_TYPE> JoinLine::get_vector_of_id(VariableType var_type) const {
    switch(var_type){
        case INS:
            return instances;
        case PRO:
            return properties;
        case CON:
            return concepts;
        default:
            cerr << "Wrong variable type!" << endl;

    }
}

vector<string*> JoinLine::get_vector_of_str() const{
    return string_datas;
}


void JoinLine::merge_simple(JoinLine &line2) {
    for(auto id: line2.get_vector_of_id(INS)){
        this->instances.push_back(id);
    }
    for(auto id: line2.get_vector_of_id(PRO)){
        this->properties.push_back(id);
    }
    for(auto id: line2.get_vector_of_id(CON)){
        this->concepts.push_back(id);
    }
    for(auto str: line2.get_vector_of_str()){
        this->string_datas.push_back(str);
    }
    for(auto ds: line2.get_vector_of_data_series()){
        this->dataserie_list.push_back(ds);
    }
}

void JoinLine::merge_complexe(JoinLine &line2, vector<tuple<VariableType, long>> &index_list) {
    cerr << "Not implemented!" << endl;
}