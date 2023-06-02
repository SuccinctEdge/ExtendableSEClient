//
// Created by weiqin xu on 04/02/2020.
//

#ifndef SUCCINCT_EDGE_PROPERTYLITEMATDICTIONARY_HPP
#define SUCCINCT_EDGE_PROPERTYLITEMATDICTIONARY_HPP

#include "LiteMatDictionary.hpp"

#define DATA_TYPE 3

class PropertyLiteMatDictionary: public LiteMatDictionary {
public:
    void add(string instance, ID_TYPE id, ID_TYPE length_encoding_local, ID_TYPE last_bit_encoding, ID_TYPE is_data_type_property);
    bool is_data_type(string p);
    void store_to_disk(const string &path, const string &file_name);
    void back_up_from_disk(const string &path, const string &file_name);
};


#endif //SUCCINCT_EDGE_PROPERTYLITEMATDICTIONARY_HPP
