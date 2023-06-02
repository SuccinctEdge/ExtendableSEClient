//
// Created by pasyj on 8/25/2022.
//

#include "Query.h"

#include <utility>

Query::Query(const vector<long> &dataVariablesIndexes, vector<string> listDataVariables, string signal,
             Filter filter, list<JoinLine> table, int spread)
             : data_variables_indexes(dataVariablesIndexes), list_data_variables(std::move(listDataVariables)), signal(std::move(signal)),
               filter(std::move(filter)), table(std::move(table)), spread(spread) {}

const Filter &Query::getFilter() const {
    return filter;
}

const string &Query::getSignal() const {
    return signal;
}

const vector<long> &Query::getDataVariablesIndexes() const {
    return data_variables_indexes;
}

const vector<string> &Query::getListDataVariables() const {
    return list_data_variables;
}

const list<JoinLine> &Query::getTable() const {
    return table;
}

const int &Query::getSpread() const{
    return spread;
}
