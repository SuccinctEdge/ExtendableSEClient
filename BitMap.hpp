//
// Created by weiqin xu on 29/12/2019.
//

#ifndef TRIPLE_STORE_BITMAP_HPP
#define TRIPLE_STORE_BITMAP_HPP

#include <sdsl/rrr_vector_extendable.hpp>
#include <string>


using namespace sdsl;
using namespace std;

class BitMap {
private:
    rrr_vector_extendable<63> v_rrr;
    rrr_vector_extendable<>::rank_1_type v_rank;
    rrr_vector_extendable<>::select_1_type v_select;
public:
    BitMap(bit_vector& v);
    unsigned long long rank(unsigned long long index, int num);
    unsigned long long select(unsigned long long i, int num);
    unsigned long long size() { return v_rrr.size(); }
    void store_to_disk(const string &path);

    void extend(bit_vector vector);
};


#endif //TRIPLE_STORE_BITMAP_HPP
