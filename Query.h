//
// Created by pasyj on 8/25/2022.
//

#ifndef SUCCINCTEDGE_CATA_QUERY_H
#define SUCCINCTEDGE_CATA_QUERY_H


#include <vector>
#include "Filter.hpp"
#include "JoinLine.hpp"

class Query {
public:
    Query(const vector<long> &dataVariablesIndexes, vector<string> listDataVariables, string signal,
          Filter filter, list<JoinLine> table, int spread);

    const Filter &getFilter() const;

    const string &getSignal() const;

    const vector<long> &getDataVariablesIndexes() const;

    const vector<string> &getListDataVariables() const;

    const list<struct JoinLine> & getTable() const;

    const int & getSpread() const;

private:
    Filter filter;
    string signal;
    list<JoinLine> table;
    vector<long> data_variables_indexes;
    vector<string> list_data_variables;
    int spread;
};


#endif //SUCCINCTEDGE_CATA_QUERY_H
