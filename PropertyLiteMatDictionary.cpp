//
// Created by weiqin xu on 04/02/2020.
//

#include "PropertyLiteMatDictionary.hpp"
#include <iostream>
#include "Util.hpp"
#include <fstream>
#include <sstream>
#include <string>

void PropertyLiteMatDictionary::add(string instance, ID_TYPE id, ID_TYPE length_encoding_local,
                                    ID_TYPE last_bit_encoding, ID_TYPE is_data_type_property) {
    instance2id[instance].push_back(id);
    instance2id[instance].push_back(length_encoding_local);
    instance2id[instance].push_back(last_bit_encoding);
    instance2id[instance].push_back(is_data_type_property);
    id2instance[id] = instance;
}

bool PropertyLiteMatDictionary::is_data_type(string p) {
    if(instance2id.count(p) == 0)
        return false;
    if(instance2id[p][DATA_TYPE] == 1)
        return true;
    else
        return false;
}

void PropertyLiteMatDictionary::back_up_from_disk(const string &path, const string &file_name) {
    ifstream file;
    string full_file_path = path[path.size() - 1] == '/'? path + file_name: path + "/" + file_name;
    file.open(full_file_path);
    string line;
    total_encoding_length = 0;
    while(getline(file, line, '\n')){
        istringstream is(line);
        string ins;
        ID_TYPE id, local_length, last_bit_encoding, temp;
        is >> ins >> id >> local_length >> last_bit_encoding >> temp;
        this->add(ins, id, local_length, last_bit_encoding, is.eof()?1:0);
        if(last_bit_encoding > total_encoding_length)
            total_encoding_length = last_bit_encoding;
    }
    cout << instance2id.size() << " elements!" << endl;
}

void PropertyLiteMatDictionary::store_to_disk(const string &path, const string &file_name) {
    string file_path;

    if(path[path.length() - 1] == '/'){
        file_path = path + file_name + ".dict";
    }else{
        file_path = path + "/" + file_name + ".dict";
    }

    ofstream out(file_path);

    for(auto line: instance2id){
        out << line.first << " " << (line.second)[0] << " " << (line.second)[1] << " " << (line.second)[2] << (line.second)[3] << endl;
    }
}
