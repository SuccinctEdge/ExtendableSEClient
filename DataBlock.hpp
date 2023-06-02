//
// Created by weiqin xu on 04/11/2020.
//

#ifndef SUCCINCTEDGE_CATA_DATABLOCK_HPP
#define SUCCINCTEDGE_CATA_DATABLOCK_HPP

template<int block_size>
class DataBlock {
private:
    double max, min, avg;
    double data_seq;
public:
    DataBlock();
};


#endif //SUCCINCTEDGE_CATA_DATABLOCK_HPP
