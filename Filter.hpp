//
// Created by pavell on 11/5/21.
//

#ifndef SUCCINCTEDGE_CATA_FILTER_H
#define SUCCINCTEDGE_CATA_FILTER_H

#include <string>
#include <stdexcept>

using namespace std;

enum FilterOp {
    LT,
    GT,
    LTE,
    GTE,
    E,
    FILTER_NONE,
};

FilterOp opFromString(const string& op);

class Filter {
private:
    string variable;
    FilterOp op;
    double value{};

public:
    Filter(const string& variable, FilterOp op, double value);
    Filter();

    string getVariable() const;

    bool filter(double filter_val);

    bool isValid();

    bool filtering(string basicString);
};


#endif //SUCCINCTEDGE_CATA_FILTER_H
