//
// Created by weiqin xu on 26/08/2019.
//

#ifndef TRIPLE_STORE_RDFSTORAGE_HPP
#define TRIPLE_STORE_RDFSTORAGE_HPP

#include "TripleStore.hpp"
#include "DataTypeStore.hpp"
#include "Dictionary.hpp"
#include "RDFTypeStore.hpp"
#include "LiteMatDictionary.hpp"
#include "PropertyLiteMatDictionary.hpp"
#include "JoinLine.hpp"
#include "JoinVariables.hpp"
#include <iostream>
#include <fstream>
#include "Constant.hpp"
#include <math.h>
#include <sstream>

using namespace std;


class RDFStorage {
private:
    bool show_sds_code = false;
    bool reason = false;
    bool show_results = false;
    bool active_plan = true;
    TripleStore triple_store;
    DataTypeStore *data_triple_store;
    // Storage for: s rdf:type concept
    RDFTypeStore type_store;
    Dictionary dict_instances;
    PropertyLiteMatDictionary dict_properties;   // Can be optimized with LiteMat
    LiteMatDictionary dict_concepts;  // Can be optimized with LiteMat
    map<string, vector<long>> topic_cols_map;


    unsigned long long num_predicate_triples(string predicate);

    unsigned long long num_rdftype_triples(vector<string> &tp);

    tuple<string, string, string> triple_string_parser(string &str);

    bool more_selective(vector<string> &first, vector<string> &second);

    vector<string> find_first_triple_pattern(vector<vector<string>> &query_table);

    vector<string> find_next_triple_pattern(JoinVariables &variables, vector<vector<string>> &query_table);

    void join(JoinVariables &variables, list<JoinLine> &table_a, vector<string> &triple_pattern);

    //void merge_join(JoinVariables& variables, list<JoinLine>& table_a, vector<string> &triple_pattern);
    int has_binding(JoinVariables &variables, vector<string> &tp);

    int selectivity(vector<string> &t);

    bool merge_join_condition(const vector<string> &variables, const vector<string> &triple_pattern);

public:
    RDFStorage();

    RDFStorage(string &tbox_path, ifstream &abox_file, TripleStoreType triple_store_type);

    RDFStorage(ifstream &abox_file, TripleStoreType triple_store_type);


    TripleStoreType get_triple_store_type() {
        return triple_store.get_ts_type();
    }

    void set_reason(bool set_var) {
        reason = set_var;
    }

    void set_show_results(bool sr) {
        show_results = sr;
    }

    void set_active_plan(bool act_p) {
        active_plan = act_p;
    }

    void store_to_disk(const string &path);

    void back_up_from_disk(const string &path);

    void load_facts(ifstream &abox_file, TripleStoreType triple_store_type);

    void load_concepts(ifstream &concepts_file);

    tuple<JoinVariables, vector<JoinLine>> query_triple_pattern(string s, string p, string o, bool first_exe = true);

    tuple<JoinVariables, list<JoinLine>> query_graph_pattern(vector<string> triple_string_vector);

    string look_up_id_instance(ID_TYPE id) {
        return dict_instances.look_up_id(id);
    }

    string look_up_id_property(ID_TYPE id) {
        return dict_properties.look_up_id(id);
    }

    string look_up_id_concept(ID_TYPE id) {
        return dict_concepts.look_up_id(id);
    }


    void insert_numeric_data(vector<long> &data_indexes, vector<double> &data, mutex &lock) {
        lock.lock();
        //cout << "insert data" << endl;
        data_triple_store->insert_data(data_indexes, data);
        lock.unlock();
    }


    void insert_numeric_data(vector<long> &data_indexes, vector<string> &data, mutex &lock) {
        lock.lock();
        data_triple_store->insert_data(data_indexes, data);
        lock.unlock();
    }

    void insert_numeric_data_with_topic(string &topic, vector<double> &data, mutex &lock) {
        lock.lock();
        data_triple_store->insert_data(topic_cols_map[topic], data);
        lock.unlock();
    }

    void insert_numeric_data_with_topic(string &topic, vector<string> &data, mutex &lock) {
        lock.lock();
        data_triple_store->insert_data(topic_cols_map[topic], data);
        lock.unlock();
    }

    void drop_numeric_data(mutex &lock) {
        lock.lock();
        data_triple_store->drop_data();
        lock.unlock();
    }

    void drop_numeric_data_with_index(long index) {
        data_triple_store->drop_data_with_index(index);
    }

    void drop_all_numeric_data(mutex &lock) {
        lock.lock();
        data_triple_store->drop_all_data();
        lock.unlock();
    }

    void drop_all_numeric_data_with_index(long index) {
        data_triple_store->drop_all_data();
    }

    void change_data_mode(int index, data_function function) {
        data_triple_store->change_mode(index, function);
    }

    void all_change_data_mode(data_function function) {
        data_triple_store->all_change_mode(function);
    }

    long long get_data_head_tail_difference_with_index(long index) {
        return data_triple_store->get_head_tail_difference_with_index(index);
    }

    vector<long> get_indexes_with_data_names(vector<string> &col) {
        return data_triple_store->get_indexes_with_data_names(col);
    }

    bool data_serie_not_null(long index) {
        return data_triple_store->data_serie_not_null(index);
    }

    void init_topic_cols_map(string &file_path) {
        ifstream input_file(file_path);
        string str;
        while (getline(input_file, str, '\n')) {
            string ele, key;
            vector<string> vec_cols;
            istringstream line_stream(str);
            line_stream >> key;
            while (line_stream >> ele) {
                vec_cols.push_back(ele);
            }
            topic_cols_map[key] = get_indexes_with_data_names(vec_cols);

        }
    }


    void check_windows_and_drop_data(long window_size, mutex &lock) {
        map<string, vector<long>>::iterator iter;
        for (iter = topic_cols_map.begin(); iter != topic_cols_map.end(); ++iter) {
            while (get_data_head_tail_difference_with_index((iter->second)[0]) > window_size) {
                lock.lock();
                for (long i = 0; i < (iter->second).size(); ++i) {
                    drop_numeric_data_with_index((iter->second)[i]);
                }
                lock.unlock();
            }
        }
    }

    void check_windows_and_drop_all_data(long window_size, mutex &lock, bool &tumbling_out_flag) {
        //Tumbling out flag
        map<string, vector<long>>::iterator iter;
        for (iter = topic_cols_map.begin(); iter != topic_cols_map.end(); ++iter) {
            if (get_data_head_tail_difference_with_index((iter->second)[0]) >= window_size) {
                tumbling_out_flag = true;
                while (tumbling_out_flag) {
                    //usleep(100);
                }
                drop_all_numeric_data(lock);
                break;
            }
        }
    }

    void extend(string path, TripleStoreType ts_type);
};

#endif //TRIPLE_STORE_RDFSTORAGE_HPP
