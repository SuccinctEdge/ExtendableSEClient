//
// Created by pavell on 11/5/21.
//

#include "Filter.hpp"

FilterOp opFromString(const string& op){
    if (op == "<") return LT;
    else if (op == ">") return GT;
    else if (op == "<=") return LTE;
    else if (op ==">=") return GTE;
    else if (op == "==") return E;
    else throw std::runtime_error("Invalid operator for filter");
}

Filter::Filter(const string& variable, FilterOp op, double value) {
    this->variable = variable;
    this->op = op;
    this->value = value;
}

bool Filter::filter(double filter_val) {
    switch (op){
        case LT:
            return value > filter_val;
        case GT:
            return value < filter_val;
        case LTE:
            return value >= filter_val;
        case GTE:
            return value <= filter_val;
        case E:
            return value == filter_val;
        default:return true;
    }
}

string Filter::getVariable() const {
    return variable;
}

Filter::Filter() {
    this->op = FILTER_NONE;
}

bool Filter::isValid() {
    return op != FILTER_NONE;
}

bool Filter::filtering(string var) {
    return var == variable;
}
