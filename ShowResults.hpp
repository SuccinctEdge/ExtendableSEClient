//
// Created by weiqin xu on 19/11/2020.
//

#ifndef SUCCINCTEDGE_CATA_SHOWRESULTS_HPP
#define SUCCINCTEDGE_CATA_SHOWRESULTS_HPP

#include "JoinVariables.hpp"
#include "JoinLine.hpp"
#include "RDFStorage.hpp"
#include "TCPReceiver.hpp"
#include "Filter.hpp"
#include "Query.h"
#include <list>


void showResults(RDFStorage &test, JoinVariables &variables, list<JoinLine> &res_table, Filter &filter){
    variables.print_all();
    cout << endl;

    for (auto i: res_table) {


        //Anomalie detection, to be improved
        string data_variable = "?Value";
        long data_index = variables.get_data_index(data_variable);

        //if(stod(*((i.get_vector_of_str())[data_index])) < 5 && stod(*((i.get_vector_of_str())[data_index])) > 4){
        //    continue;
        //}


        double time = std::chrono::system_clock::now().time_since_epoch() /std::chrono::milliseconds(1);
        cout << "latency: " << time - stod(*i.get_vector_of_str()[variables.get_data_variables_indexes("?TimeStamp")]) << endl;



        vector<ID_TYPE> vec_ins = i.get_vector_of_id(INS);
        for (auto j: vec_ins) {
            cout << test.look_up_id_instance(j) << " ";
        }

        vector<ID_TYPE> vec_pro = i.get_vector_of_id(PRO);
        for (auto j: vec_pro) {
            cout << test.look_up_id_property(j) << " ";
        }

        vector<ID_TYPE> vec_con = i.get_vector_of_id(CON);
        for (auto j: vec_con) {
            cout << test.look_up_id_concept(j) << " ";
        }


        vector<string*> vec_str = i.get_vector_of_str();
        for (auto j: vec_str) {
            cout << *j << " ";
        }


        cout << endl;
    }
}

void sendResults(RDFStorage &test, JoinVariables &variables, list<JoinLine> &res_table, Filter &filter){
    for (auto i: res_table) {

        //Anomalie detection, to be improved
        string data_variable = "?Value";
        long data_index = variables.get_data_index(data_variable);
        string str_val = *((i.get_vector_of_str())[data_index]);

        double val = stod(str_val);

        if (!filter.isValid() || (filter.filtering(data_variable) && filter.filter(val))) {
            cout << "Received " << val << " and " << *(i.get_vector_of_str()[variables.get_data_variables_indexes("?TimeStamp")]) << endl;

            time_reception = std::chrono::system_clock::now().time_since_epoch() /std::chrono::milliseconds(1);
            std::cout << time_reception - stoll(*i.get_vector_of_str()[variables.get_data_variables_indexes("?TimeStamp")])  << "ms" << std::endl;

            report_anomaly(*i.get_vector_of_str()[variables.get_data_variables_indexes("?TimeStamp")] + "," + *((i.get_vector_of_str())[data_index]));
        }
        else {
            cout << "FILTERED " << val << endl;
        }
    }
}

void displayAllQueryResults(const vector<Query>& queries){
    ostringstream temp;
    for(const auto& query : queries){
        for(auto & res_line : query.getTable()){
            temp << "{";
            for(auto i = 0; i+query.getSpread() < query.getDataVariablesIndexes().size(); i++){
                temp << "\"" << query.getListDataVariables()[i] << "\":\"" << *res_line.get_vector_of_str()[query.getDataVariablesIndexes()[i]] << "\"" << endl;
                if(i < query.getDataVariablesIndexes().size() - 1)
                    temp << ", ";
                }
            temp << "}";
            report_anomaly(temp.str());
            temp.clear();
        }
    }
}


#endif //SUCCINCTEDGE_CATA_SHOWRESULTS_HPP
