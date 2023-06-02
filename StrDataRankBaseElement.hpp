//
// Created by weiqin xu on 04/02/2020.
//

#ifndef SUCCINCT_EDGE_STRDATARANKBASEELEMENT_HPP
#define SUCCINCT_EDGE_STRDATARANKBASEELEMENT_HPP

#include "Constant.hpp"
#include <memory>
#include <string>
#include <iostream>


using namespace std;

template <int SIZE>
class StrDataRankBaseElement {
private:
    ID_TYPE ele[SIZE];
    string* pt_data;
public:
    StrDataRankBaseElement(ID_TYPE e[], string* str){
        for(int i = 0; i < SIZE; ++i){
            ele[i] = e[i];
        }
        //cout << "bon" << endl;
        if(str == NULL)
            cerr << "str NULL!" << endl;
        pt_data = str;
    }

    string* get_str_pt(){
        return pt_data;
    }

    bool operator<(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            if(ele[i] > other.ele[i]){
                return false;
            }
            if(ele[i] < other.ele[i]){
                return true;
            }
        }
        if(*pt_data < *other.get_str_pt()){
            return true;
        }
        return false;
    }

    bool operator>(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            if(ele[i] < other.ele[i]){
                return false;
            }
            if(ele[i] > other.ele[i]){
                return true;
            }
        }
        if(*pt_data > *other.get_str_pt()){
            return true;
        }
        return false;
    }

    bool operator<=(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            if(ele[i] > other.ele[i]){
                return false;
            }
            if(ele[i] < other.ele[i]){
                return true;
            }
        }
        if(*pt_data > *other.get_str_pt()){
            return false;
        }
        return true;
    }

    bool operator>=(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            if(ele[i] < other.ele[i]){
                return false;
            }
            if(ele[i] > other.ele[i]){
                return true;
            }
        }
        if(*(pt_data) < *other.get_str_pt()){
            return false;
        }
        return true;
    }

    bool operator==(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            if(ele[i] != other.ele[i]){
                return false;
            }
        }
        if(*(pt_data) != *other.get_str_pt()){
            return false;
        }
        return true;
    }

    bool operator!=(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            if(ele[i] != other.ele[i]){
                return true;
            }
        }
        if(*(pt_data) != *other.get_str_pt()){
            return true;
        }
        return false;
    }

    void operator=(StrDataRankBaseElement& other){
        for(int i = 0; i < SIZE; ++i){
            ele[i] = other.ele[i];
        }
        pt_data = other.get_str_pt();
    }

    ID_TYPE& operator[](int i){
        if(i >= SIZE){
            cout << "Out of index\n";
            return ele[0];
        }else{
            return ele[i];
        }
    }

};


#endif //SUCCINCT_EDGE_STRDATARANKBASEELEMENT_HPP
