//
// Created by weiqin xu on 26/08/2019.
//

#include "RDFStorage.hpp"
#include "RankBaseElement.hpp"
#include "StrDataRankBaseElement.hpp"
#include <sstream>
#include "Util.hpp"
#include "Constant.hpp"
#include "quickSortTS.hpp"
#include "JoinLine.hpp"
#include "JoinVariables.hpp"
#include <memory>



using namespace std;

RDFStorage::RDFStorage(){}

RDFStorage::RDFStorage(string &tbox_path, ifstream& abox_file, TripleStoreType ts_type){
    cout << "Properties: ";
    dict_properties.back_up_from_disk(tbox_path, "lite_mat_properties.txt");
    cout << "Concepts: ";
    dict_concepts.back_up_from_disk(tbox_path, "lite_mat_concepts.txt");
    load_facts(abox_file, ts_type);
    set_reason(true);
}

RDFStorage::RDFStorage(ifstream& abox_file, TripleStoreType ts_type){
    load_facts(abox_file, ts_type);
    //set_reason(false);
}

void RDFStorage::load_facts(ifstream& abox_file, TripleStoreType ts_type) {

    // comment if use rdf:type store
    //dict_properties.add("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>", 0);

    ID_TYPE last_instance_id = 0;
    ID_TYPE last_property_id = 0;
    ID_TYPE last_concept_id = 0;

    vector<ID_TYPE> s_vector;
    vector<ID_TYPE> p_vector;
    vector<ID_TYPE> o_vector;

    vector<ID_TYPE> str_s_vector;
    vector<ID_TYPE> str_p_vector;
    vector<string> str_o_vector;

    string str;
    vector<RankBaseElement<3>> triple_vector;
    vector<StrDataRankBaseElement<2>> str_triple_vector;

    // For each fact, if instance doesn't exist, ditribute an id_instance
    //                if concept doesn't exist, remind to check the TBOX
    //                if property doesn't exist, ditribute an id_property
    auto start = clock();
    while(getline(abox_file, str, '\n')){
        string subject, predicate, object;
        tie(subject, predicate, object) = triple_string_parser(str);
        ID_TYPE id_s, id_p, id_o;
        bool is_data_propety = dict_properties.is_data_type(predicate);

        // Update dictionary, search id_s
        try {
            id_s = dict_instances.look_up_instance(subject);
        }catch(DictException& excep){
            ++last_instance_id;
            dict_instances.add(subject, last_instance_id);
            id_s = last_instance_id;
        }

        // Can be optimized with LiteMat, search id_p
        if(predicate != RDF_TYPE) {
            try {
                id_p = dict_properties.look_up_instance(predicate);
            } catch (DictException &excep) {
                cerr << str << endl;
                cerr << subject << " " << predicate << " " << object << endl;
                /*
                ++last_property_id;
                dict_properties.add(predicate, last_property_id);
                id_p = last_property_id;
                 */
                cerr << "Property doesn't exist, load TBOX first!" << endl;
                continue;
            }
        }


        // If predicate == rdf:type, object is a concept, otherwise object is an instance. search id_o
        if(/*true*/predicate != RDF_TYPE){
            if(!is_data_propety){
                try {
                    id_o = dict_instances.look_up_instance(object);
                }catch(DictException& excep){
                    ++last_instance_id;
                    dict_instances.add(object, last_instance_id);
                    id_o = last_instance_id;
                }
                // In order of PSO or POS
                ID_TYPE tmp[3];
                if(ts_type == PSO){
                    tmp[0] = id_p;
                    tmp[1] = id_s;
                    tmp[2] = id_o;
                }else if(ts_type == POS){
                    tmp[0] = id_p;
                    tmp[1] = id_o;
                    tmp[2] = id_s;
                }else{
                    cerr << "Triple store type not defined!" << endl;
                    exit(1);
                }


                //cout << "new triple: " << id_s << " " << id_p << " " << id_o << endl;
                triple_vector.push_back(RankBaseElement<3>(tmp));
            }else{
                //Only PSO
                ID_TYPE tmp[2];
                tmp[0] = id_p;
                tmp[1] = id_s;

                string* pt_o =  new string(object);

                str_triple_vector.push_back(StrDataRankBaseElement<2>(tmp, pt_o));

                //cout << *pt_o << " " << pt_o << endl;
                //cout << *(str_triple_vector.back().get_str_pt()) << endl;
            }

        }else{ //Will be optimized with LiteMat

            try {
                id_o = dict_concepts.look_up_instance(object);
                try {
                    if (!type_store.contains(id_s, id_o)) {
                        type_store.add(id_s, id_o);
                    }
                }catch(RDFTypeException& except){
                    cout << "Is " << object << " the most precise concept of " << subject << "? Y/N" << endl;
                    char answer;
                    cin >> answer;
                    while(answer != 'Y' && answer != 'N' && answer != 'y' && answer != 'n'){
                        cout << "Please type Y/N!" << endl;
                        cin >> answer;
                    }
                    if(answer == 'Y' || answer == 'y'){
                        type_store.add(id_s, id_o, true);
                    }
                }
            }catch(DictException& excep){
                // Will be optimized with LiteMat
                cerr << subject << " " << predicate << " " << object << endl;
                cerr << "Concept doesn't exist! Load the TBOX first!" << endl;
                continue;
                /*
                ++last_concept_id;
                dict_concepts.add(object, last_concept_id);
                id_o = last_concept_id;
                type_store.add(id_s, id_o);
                 */
            }


        }
    }
    cout << "Parse each line to creat dictionary and store triples in a vector time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    cout << "Last instance: " << last_instance_id << endl;
    //cout << "Last proprety: " << last_property_id << endl;

    if(triple_vector.size() <= 0){
        cerr << "Triple vector size is 0!\n";
        exit(1);
    }
    start = clock();
    if(triple_vector.size()>0)
        quickSortTS<RankBaseElement<3>>(triple_vector, 0, triple_vector.size() - 1);
    if(str_triple_vector.size()>0)
        quickSortTS<StrDataRankBaseElement<2>>(str_triple_vector, 0, str_triple_vector.size() - 1);
    cout << "Qsort vector of triples time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;



    // Creat wt structure storage, In order of PSO or POS
    start = clock();
    if(ts_type == PSO){
        auto innerstart = clock();
        for(unsigned long i = 0; i < triple_vector.size(); ++i){
            if(i > 0 && triple_vector[i][0] == triple_vector[i-1][0] && triple_vector[i][1] == triple_vector[i-1][1] && triple_vector[i][2] == triple_vector[i-1][2])
                continue;
            p_vector.push_back(triple_vector[i][0]);
            s_vector.push_back(triple_vector[i][1]);
            o_vector.push_back(triple_vector[i][2]);
        }
        for(unsigned long i = 0; i < str_triple_vector.size(); ++i){
            if(i > 0 && str_triple_vector[i][0] == str_triple_vector[i-1][0] && str_triple_vector[i][1] == str_triple_vector[i-1][1] && str_triple_vector[i].get_str_pt() == str_triple_vector[i-1].get_str_pt())
                continue;
            str_p_vector.push_back(str_triple_vector[i][0]);
            str_s_vector.push_back(str_triple_vector[i][1]);
            str_o_vector.push_back(*str_triple_vector[i].get_str_pt());
        }

        cout << "   -Push each line into P,S,O vectors: " << (double)(clock() - innerstart)/CLOCKS_PER_SEC*1000 << endl;
        innerstart =clock();
        triple_store = TripleStore(p_vector, s_vector, o_vector, ts_type);
        data_triple_store = new DataTypeStore(str_p_vector, str_s_vector, str_o_vector);
        cout << "  | Creat wt structure with separate P,S,O vectors: " << (double)(clock() - innerstart)/CLOCKS_PER_SEC*1000 << endl;
    }else if(ts_type == POS){
        for(unsigned long i = 0; i < triple_vector.size(); ++i){
            p_vector.push_back(triple_vector[i][0]);
            o_vector.push_back(triple_vector[i][1]);
            s_vector.push_back(triple_vector[i][2]);
        }
        triple_store = TripleStore(p_vector, o_vector, s_vector, ts_type);
    }else{
        cerr << "Triple store type not defined!" << endl;
        exit(1);
    }
    cout << "Creat triple store time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
}




tuple<JoinVariables, vector<JoinLine>> RDFStorage::query_triple_pattern(string s, string p, string o, bool first_exe) {
    vector<JoinLine> results;
    JoinVariables variables;

    //cout << first_exe << endl;

    if(/*true*/ p != RDF_TYPE) { //Optimized with LiteMate

        // Doesn't support reason for 3 variables
        if (s[0] == '?' && p[0] == '?' && o[0] == '?') {
            results = triple_store.look_up_ele012();

            //cout << "s" << " " << "p" << " " << "o" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id(i[1]) << " "
                         << dict_properties.look_up_id(i[0])
                         << " " << dict_instances.look_up_id(i[2]) << "\n";
                }*/
                variables.add(p, PRO);
                variables.add(s, INS);
                variables.add(o, INS);
            }else if(triple_store.get_ts_type() == POS){
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id(i[2]) << " "
                         << dict_properties.look_up_id(i[0])
                         << " " << dict_instances.look_up_id(i[1]) << "\n";
                }*/
                variables.add(p, PRO);
                variables.add(o, INS);
                variables.add(s, INS);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }

            //cout << '\n' << "\n";
        } else if (s[0] == '?' && p[0] == '?') { //Doesn't support reason for ?p
            // In order of P S O
            //cout << "s" << " " << "p" << "\n";


            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele01(dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[1] ) << " "
                         << dict_properties.look_up_id( i[0] )
                         << "\n";
                }*/
                variables.add(p, PRO);
                variables.add(s, INS);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele02(dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[1] ) << " "
                         << dict_properties.look_up_id( i[0] )
                         << "\n";
                }*/
                variables.add(p, PRO);
                variables.add(s, INS);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (s[0] == '?' && o[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "s" << " " << "o" << "\n";


            if(show_sds_code && first_exe){
                cout << "--TP type: ?s P ?o" << endl;
                cout << "--Index_P = wt_p.select(1,P)" << endl;
                cout << "--Index_S_begin = bit_map_ps.select(index_P + 1, 1)" << endl;
                cout << "--Index_S_end = bit_map_ps.select(index0 + 2, 1)" << endl;
                cout << "--For each index_S between Index_S_begin and Index_S_end:" << endl;
                cout << "----Index_O_end = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "----Index_O_end = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "----Add each SO pairs to result set" << endl;
            }
            if(triple_store.get_ts_type() == PSO) {
                try {
                    // Reason or not
                    /*
                    if(show_sds_code){
                        cout << "--P: " << dict_properties.look_up_instance(p) << endl;
                    }*/
                    if(!reason){
                        results = dict_properties.is_data_type(p)?data_triple_store->look_up_ele12(dict_properties.look_up_instance(p)):triple_store.look_up_ele12(dict_properties.look_up_instance(p));
                    }else{
                        ID_TYPE p_begin, p_end;
                        tie(p_begin, p_end) = dict_properties.look_up_instance_range(p);
                        results = dict_properties.is_data_type(p)?data_triple_store->look_up_ele12_with_reason(p_begin, p_end):triple_store.look_up_ele12_with_reason(p_begin, p_end);
                    }

                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << " "
                         << dict_instances.look_up_id( i[1] )
                         << "\n";
                }*/
                variables.add(s, INS);
                dict_properties.is_data_type(p)? variables.add(o, STR_DATA):variables.add(o, INS);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele12(dict_properties.look_up_instance(p));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[1] ) << " "
                         << dict_instances.look_up_id( i[0] )
                         << "\n";
                }*/
                variables.add(o, INS);
                variables.add(s, INS);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (p[0] == '?' && o[0] == '?') {
            // In order of P S O
            //cout << "p" << " " << "o" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele02(dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << " "
                         << dict_instances.look_up_id( i[1] )
                         << "\n";
                }*/
                variables.add(p, PRO);
                variables.add(o, INS);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele01(dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << " "
                         << dict_instances.look_up_id( i[1] )
                         << "\n";
                }*/
                variables.add(p, PRO);
                variables.add(o, INS);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (p[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "p" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele0(dict_instances.look_up_instance(s),
                                                        dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }/*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << endl;
                }*/
                variables.add(p, PRO);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele0(dict_instances.look_up_instance(o),
                                                        dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << endl;
                }*/
                variables.add(p, PRO);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (s[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "s" << "\n";

            if(show_sds_code && first_exe){
                cout << "--TP type: ?s P O" << endl;
                cout << "--Index_P = wt_p.select(1,P)" << endl;
                cout << "--Index_S_begin = bit_map_ps.select(index_P + 1, 1)" << endl;
                cout << "--Index_S_end = bit_map_ps.select(index0 + 2, 1)" << endl;
                cout << "--For each index_S between Index_S_begin and Index_S_end:" << endl;
                cout << "----Index_O_begin = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "----Index_O_end = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "----if O existe between Index_O_begin and Index_O_end:" << endl << "------Add each S to result set" << endl;
            }
            if(triple_store.get_ts_type() == PSO) {
                try {
                    /*
                    if(show_sds_code){
                        cout << "--P: " << dict_properties.look_up_instance(p) << endl;
                        cout << "--O: " << dict_instances.look_up_instance(o) << endl;
                    }*/
                    // Reason or not
                    if(!reason){

                        results = dict_properties.is_data_type(p)?data_triple_store->look_up_ele1(dict_properties.look_up_instance(p), o):triple_store.look_up_ele1(dict_properties.look_up_instance(p), dict_instances.look_up_instance(o));
                    }else{
                        ID_TYPE p_begin, p_end;
                        tie(p_begin, p_end) = dict_properties.look_up_instance_range(p);
                        //cout << p_begin << " " << p_end << endl;
                        results = dict_properties.is_data_type(p)?data_triple_store->look_up_ele1_with_reason(p_begin, p_end, o):triple_store.look_up_ele1_with_reason(p_begin, p_end, dict_instances.look_up_instance(o));
                    }

                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.add(s, INS);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele2(dict_properties.look_up_instance(p), dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }/*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.add(s, INS);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (o[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "o" << "\n";

            if(show_sds_code && first_exe){
                cout << "--TP type: S P ?o" << endl;
                cout << "--Index_P = wt_p.select(1,P)" << endl;
                cout << "--Index_S_begin = bit_map_ps.select(index_P + 1, 1)" << endl;
                cout << "--Index_S_end = bit_map_ps.select(index0 + 2, 1)" << endl;
                cout << "--Index_S = range_search(Index_S_begin, Index_S_end - 1, S):" << endl;
                cout << "--Index_O_begin = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "--Index_O_end = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "--Add each O between Index_O_begin and Index_O_end into result set" << endl;
            }
            if(triple_store.get_ts_type() == PSO) {
                try {
                    /*
                    if(show_sds_code){
                        cout << "--S: " << dict_instances.look_up_instance(s) << endl;
                        cout << "--P: " << dict_properties.look_up_instance(p) << endl;
                    }*/
                    // Reason or not
                    if(!reason){
                        results = dict_properties.is_data_type(p)?data_triple_store->look_up_ele2(dict_properties.look_up_instance(p), dict_instances.look_up_instance(s)):triple_store.look_up_ele2(dict_properties.look_up_instance(p), dict_instances.look_up_instance(s));
                    }else{
                        ID_TYPE p_begin, p_end;
                        tie(p_begin, p_end) = dict_properties.look_up_instance_range(p);
                        results = dict_properties.is_data_type(p)?data_triple_store->look_up_ele2_with_reason(p_begin, p_end, dict_instances.look_up_instance(s)):triple_store.look_up_ele2_with_reason(p_begin, p_end, dict_instances.look_up_instance(s));
                    }

                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                dict_properties.is_data_type(p)? variables.add(o, STR_DATA):variables.add(o, INS);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele1(dict_properties.look_up_instance(p), dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.add(o, INS);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else {
            if(show_sds_code && first_exe){
                cout << "--TP type: S P O" << endl;
                cout << "--Index_P = wt_p.select(1,P)" << endl;
                cout << "--Index_S_begin = bit_map_ps.select(index_P + 1, 1)" << endl;
                cout << "--Index_S_end = bit_map_ps.select(index0 + 2, 1)" << endl;
                cout << "--Index_S = range_search(Index_S_begin, Index_S_end - 1, S):" << endl;
                cout << "--Index_O_begin = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "--Index_O_end = bit_map_so.select(index_S + 2, 1)" << endl;
                cout << "--if O existe between Index_O_begin and Index_O_end:" << endl << "----Return true" << endl;
            }
            if(triple_store.get_ts_type() == PSO) {
                try{
                    if(!reason) {
                        results = dict_properties.is_data_type(p) ? data_triple_store->verify_triple(
                                dict_properties.look_up_instance(p), dict_instances.look_up_instance(s), o)
                                                           : triple_store.verify_triple(
                                        dict_properties.look_up_instance(p), dict_instances.look_up_instance(s),
                                        dict_instances.look_up_instance(o));
                    }else{
                        ID_TYPE p_begin, p_end;
                        tie(p_begin, p_end) = dict_properties.look_up_instance_range(p);
                        results = dict_properties.is_data_type(p) ? data_triple_store->verify_triple_with_reason(p_begin, p_end, dict_instances.look_up_instance(s), o) : triple_store.verigy_triple_with_reason(p_begin, p_end, dict_instances.look_up_instance(s), dict_instances.look_up_instance(o));
                    }
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
            }else if(triple_store.get_ts_type() == POS){
                try{
                    results = triple_store.verify_triple(dict_properties.look_up_instance(p), dict_instances.look_up_instance(o), dict_instances.look_up_instance(s));
                }catch (DictException &except) {
                    return make_tuple(variables, results);
                }
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
        }
        return make_tuple(variables, results);
    }else{
        //cerr << "Reasoner doesn't exist!" << endl;
        if(s[0] != '?' && o[0] != '?'){
            ID_TYPE instance_id;
            try{
                instance_id = dict_instances.look_up_instance(s);
            }catch(DictException &except){
                return make_tuple(variables, results);
            }


            ID_TYPE abox_concept_id;
            try{
                abox_concept_id = dict_concepts.look_up_instance(o);
            }catch(RDFTypeException &except){
                return make_tuple(variables, results);
            }

            if(!reason){
                if(!type_store.contains(instance_id, abox_concept_id)){
                    return make_tuple(variables, results);
                }
                JoinLine temp_res;
                temp_res.add(abox_concept_id, CON);
                results.push_back(temp_res);
            }else{
                ID_TYPE target_concept_id = dict_concepts.look_up_instance(o);
                ID_TYPE last_bit = dict_concepts.look_up_last_bit_encoding(o);
                // Verify concepts relation.
                if(abox_concept_id >= target_concept_id && abox_concept_id < (target_concept_id + (1 << (dict_concepts.look_up_total_encoding_length() - last_bit)))){
                    JoinLine temp_res;
                    temp_res.add(instance_id, INS);
                    temp_res.add(target_concept_id, CON);
                    results.push_back(temp_res);
                }
            }

        }else if(s[0] == '?' && o[0] != '?'){
            ID_TYPE concept_id;
            try{
                concept_id = dict_concepts.look_up_instance(o);
            }catch(DictException &except){
                return make_tuple(variables, results);
            }

            variables.add(s, INS);

            try{
                if(!reason){
                    results = type_store.look_up_concept(concept_id);
                }else{
                    ID_TYPE last_bit = dict_concepts.look_up_last_bit_encoding(o);
                    ID_TYPE concept_range = (concept_id + (1 << (dict_concepts.look_up_total_encoding_length() - last_bit)));
                    results = type_store.look_up_concept_with_reason(concept_id, concept_range);
                }

            }catch(RDFTypeException){
                return make_tuple(variables, results);
            }

        }else if(s[0] != '?' && o[0] == '?'){
            ID_TYPE s_id;

            try{
                s_id = dict_instances.look_up_instance(s);
            }catch(DictException &except){
                return make_tuple(variables, results);
            }

            variables.add(o, CON);

            try{
                if(!reason){
                    results = type_store.look_up_subject(s_id);
                }else{
                    cerr << "Don't support A rdf:type ?c with reason!" << endl;
                }

            }catch(RDFTypeException){
                return make_tuple(variables, results);
            }


        }else if(s[0] == '?' && o[0] == '?'){
            cerr << "Don't support ?x rdf:type ?y reason!" << endl;
        }
        return make_tuple(variables, results);
    }
}

/*
int RDFStorage::selectivity(vector<string> &t) {
    if(t[0][0] == '?' && t[1][0] == '?' && t[2][0] == '?')
        return 8;
    else if(t[0][0] == '?' && t[2][0] == '?')
        return 7;
    else if(t[1][0] == '?' && t[2][0] == '?')
        return 6;
    else if(t[0][0] == '?' && t[1][0] == '?')
        return 5;
    else if(t[2][0] == '?')
        return 4;
    else if(t[0][0] == '?')
        return 3;
    else if(t[1][0] == '?')
        return 2;
    else
        return 1;
}*/

int RDFStorage::selectivity(vector<string> &t) {
    if(t[0][0] == '?' && t[1][0] == '?' && t[2][0] == '?')
        return 8;
    else if(t[0][0] == '?' && t[2][0] == '?')
        return 7;
    else if(t[1][0] == '?' && t[2][0] == '?')
        return 6;
    else if(t[0][0] == '?' && t[1][0] == '?')
        return 5;
    else if(t[2][0] == '?')
        return 4;
    else if(t[0][0] == '?')
        return 3;
    else if(t[1][0] == '?')
        return 2;
    else
        return 1;
}

unsigned long long RDFStorage::num_predicate_triples(string predicate){
    try{
        if(dict_properties.is_data_type(predicate))
            return data_triple_store->look_up_ele0_num(dict_properties.look_up_instance(predicate));
        else
            return triple_store.look_up_ele0_num(dict_properties.look_up_instance(predicate));
    }catch(DictException& excep){
        return 0;
    }
}


unsigned long long RDFStorage::num_rdftype_triples(vector<string>& tp){
    try{
        if(tp[0][0] == '?' && tp[2][0] != '?'){
            return type_store.get_number_of_con(dict_concepts.look_up_instance(tp[2]));
        }else if(tp[0][0] != '?' && tp[2][0] == '?'){
            return type_store.get_number_of_ins(dict_instances.look_up_instance(tp[0]));
        }else{
            return type_store.get_total_size();
        }
    }catch(DictException& excep){
        return 0;
    }
}

/*
bool RDFStorage::more_selective(vector<string>& first, vector<string>& second){
    if(selectivity(first) < selectivity(second)){
        return true;
    }else if(selectivity(first) > selectivity(second)){
        return false;
    }else if(first[1][0] != '?' && second[1][0] != '?'){
        if(num_predicate_triples(first[1]) < num_predicate_triples(second[1]))
            return true;
        else
            return false;
    }else{
        return false;
    }
}*/

bool RDFStorage::more_selective(vector<string>& first, vector<string>& second){
    /*
    unsigned long long first_num, second_num;
    if(selectivity(first) < selectivity(second)){
        return true;
    }else if(selectivity(first) > selectivity(second)){
        return false;
    }else if(first[1][0] != '?' && second[1][0] != '?'){
        if((first_num = (first[1] == RDF_TYPE? num_rdftype_triples(first):num_predicate_triples(first[1]))) < (second_num = (second[1] == RDF_TYPE? num_rdftype_triples(second):num_predicate_triples(second[1]))))
            return true;
        else
            return false;
    }else{
        return false;
    }
    */

    unsigned long long first_num, second_num;
    if(first[1] == RDF_TYPE && second[1] != RDF_TYPE){
        return true;
    }else if(first[1] != RDF_TYPE && second[1] == RDF_TYPE){
        return false;
    }else if(selectivity(first) < selectivity(second)){
        return true;
    }else if(selectivity(first) > selectivity(second)){
        return false;
    }else if(first[1][0] != '?' && second[1][0] != '?'){
        if((first_num = (first[1] == RDF_TYPE? num_rdftype_triples(first):num_predicate_triples(first[1]))) < (second_num = (second[1] == RDF_TYPE? num_rdftype_triples(second):num_predicate_triples(second[1]))))
            return true;
        else
            return false;
    }else{
        return false;
    }
}

vector<string> RDFStorage::find_first_triple_pattern(vector<vector<string>>& query_table){
    vector<string> first_tp = query_table[0];
    unsigned long id = 0;
    for(unsigned long i = 1; i < query_table.size(); ++i){
        if(!more_selective(first_tp, query_table[i])){
            first_tp = query_table[i];
            id = i;
        }
    }
    query_table.erase(query_table.begin() + id);
    return first_tp;
}

int RDFStorage::has_binding(JoinVariables& variables, vector<string>& tp){
    for(auto i=0; i < tp.size(); ++i)
        if(variables.contains(tp[i]))
            return i;
    return -1;
}

vector<string> RDFStorage::find_next_triple_pattern(JoinVariables& variables, vector<vector<string>>& query_table){
    vector<string> next_tp;
    long id = -1;
    bool trig = false;
    bool s_bind = false;
    int bind_num = -1;

    for(unsigned long i = 0; i < query_table.size(); ++i){
        if(has_binding(variables, query_table[i]) >= 0){
            if(trig){
                if(!s_bind || (s_bind && (query_table[i][0][0] == '?' && query_table[i][2][0] != '?'? true:false))){
                    if(has_binding(variables, query_table[i]) <= bind_num && !more_selective(next_tp, query_table[i])){
                        next_tp = query_table[i];
                        id = i;

                        s_bind = query_table[i][0][0] == '?' && query_table[i][2][0] != '?'? true:false;
                        bind_num = has_binding(variables, query_table[i]);
                    }
                }
            }else{
                trig = true;
                next_tp = query_table[i];
                s_bind = query_table[i][0][0] == '?' && query_table[i][2][0] != '?'? true:false;
                bind_num = has_binding(variables, query_table[i]);
                id = i;
            }
        }
    }

    //cout << "Next predicate: " << next_tp[1] << endl;
    //cout << "Number of triples in dataset with this predicate: " << num_predicate_triples(next_tp[1]) << endl;

    query_table.erase(query_table.begin() + id);

    return next_tp;
}

void RDFStorage::join(JoinVariables& variables, list<JoinLine>& table_a, vector<string> &triple_pattern){
    vector<int> join_var_index;


    for(int index_triple_pattern = 0; index_triple_pattern < triple_pattern.size(); ++index_triple_pattern){
        if(triple_pattern[index_triple_pattern][0] == '?'){
            if(variables.contains(triple_pattern[index_triple_pattern])){
                join_var_index.push_back(index_triple_pattern);
            }else{
                variables.add(triple_pattern[index_triple_pattern], (index_triple_pattern == 1? PRO:((index_triple_pattern == 2 && dict_properties.is_data_type(triple_pattern[1]))? STR_DATA:INS)));
            }
        }
    }

    if(show_sds_code){
        cout << "--Find all the variables which has binding with the previous results. " << endl;
        cout << "--Binded variables: ";
        for(auto i: join_var_index){
            cout << triple_pattern[i] << "  ";
        }
        cout << endl;
        cout << "--Replace the binded variables by values in previous results, for each new generated TP, execute the corresponding SDS code." << endl;
    }

    bool first_e = true;
    for(auto iter = table_a.begin(); iter != table_a.end(); ){
        vector<string> tmp_query_triple = triple_pattern;
        JoinVariables tmp_variables;
        vector<JoinLine> tmp_table;

        for(auto i:join_var_index){
            VariableType var_type;
            long index_var;
            tie(var_type, index_var) = variables.get_index_from_var(triple_pattern[i]);
            switch(var_type){
                case INS:
                    tmp_query_triple[i] = dict_instances.look_up_id(iter->get_id(var_type, index_var));
                    break;
                case PRO:
                    tmp_query_triple[i] = dict_properties.look_up_id(iter->get_id(var_type, index_var));
                    break;
                case CON:
                    tmp_query_triple[i] = dict_concepts.look_up_id(iter->get_id(var_type, index_var));
                    break;
                default:
                    cerr << "No valide type!" << endl;
                    cerr << var_type << endl;
            }

        }

        if(!first_e){
            tie(tmp_variables, tmp_table) = this -> query_triple_pattern(tmp_query_triple[0], tmp_query_triple[1], tmp_query_triple[2], false);
        }else{
            tie(tmp_variables, tmp_table) = this -> query_triple_pattern(tmp_query_triple[0], tmp_query_triple[1], tmp_query_triple[2]);
        }

        first_e = false;

        // If SPO are all constants, query_triple_pattern only verify the triple in tripleStore, in which case, tmp_variables.size() == 0 && tmp_table.size() > 0
        if(tmp_table.size() > 0){
            if(tmp_variables.is_empty()){
                ++iter;
                continue;
            }else{
                if(tmp_table.size() == 1){
                    iter->merge_simple(tmp_table[0]);
                }else{
                    JoinLine copy = *iter;
                    iter->merge_simple(tmp_table[0]);
                    for(auto i = 1; i < tmp_table.size(); ++i){
                        ++iter;
                        iter = table_a.insert(iter, copy);
                        iter->merge_simple(tmp_table[i]);
                    }
                }
                ++iter;
            }
        }else{
            iter = table_a.erase(iter);
        }

    }

}

bool RDFStorage::merge_join_condition(const vector<string> &variables, const vector<string> &triple_pattern) {
    // number of variables are included in this join
    int num_join_var = 0;
    for(auto i:triple_pattern){
        for(auto j:variables){
            if(i == j){
                ++num_join_var;
                break;
            }
        }
    }
    if(num_join_var == 1){
        if(triple_store.get_ts_type() == PSO){
            if(triple_pattern[0] == variables[0] && triple_pattern[1][0] != '?'){
                return true;
            }else if(triple_pattern[2] == variables[0] && triple_pattern[1][0] != '?' && triple_pattern[0][0] != '?'){
                return true;
            }else{
                return false;
            }
        }else if(triple_store.get_ts_type() == POS){
            if(triple_pattern[2] == variables[0] && triple_pattern[1][0] != '?'){
                return true;
            }else if(triple_pattern[0] == variables[0] && triple_pattern[1][0] != '?' && triple_pattern[2][0] != '?'){
                return true;
            }else{
                return false;
            }
        }else{
            cerr << "Triple store type not defined!" << endl;
            exit(1);
        }

    }else{
        return false;
    }
}

tuple<JoinVariables, list<JoinLine>> RDFStorage::query_graph_pattern(vector<string> triple_string_vector) {
    vector<vector<string>> query_table;
    for(string triple_pattern:triple_string_vector){
        string query_subject, query_predicate, query_object;
        tie(query_subject, query_predicate, query_object) = triple_string_parser(triple_pattern);
        vector<string> tmp;
        tmp.push_back(query_subject);
        tmp.push_back(query_predicate);
        tmp.push_back(query_object);
        query_table.push_back(tmp);
    }


    JoinVariables variables;
    list<JoinLine> results_table;
    vector<JoinLine> results_table_vector;

    vector<string> current_triple_pattern;
    if(active_plan){
        current_triple_pattern = find_first_triple_pattern(query_table);
    }else{
        current_triple_pattern = query_table[0];
        query_table.erase(query_table.begin());
    }

    cout << current_triple_pattern[0] << " " << current_triple_pattern[1] << " " << current_triple_pattern[2] << endl;

    auto start = clock();
    tie(variables, results_table_vector) = this -> query_triple_pattern(current_triple_pattern[0], current_triple_pattern[1], current_triple_pattern[2]);
    results_table.assign(results_table_vector.begin(), results_table_vector.end());
    cout << "time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    cout << "number of results: " << results_table.size() << endl << endl;

    if(show_results){
        variables.print_all();
        for(auto line:results_table){
            line.print_all();
        }
    }


    while(query_table.size() > 0 && results_table.size() > 0){
        if(active_plan){
            current_triple_pattern = find_next_triple_pattern(variables, query_table);
        }else{
            current_triple_pattern = query_table[0];
            query_table.erase(query_table.begin());
        }

        cout << current_triple_pattern[0] << " " << current_triple_pattern[1] << " " << current_triple_pattern[2] << endl;
        // The first column is always in order, we can profit from merge_join
        auto start = clock();
        /*
        if(merge_join_condition(variables, current_triple_pattern)){
            tie(variables, results_table) = merge_join(variables, results_table, current_triple_pattern);
        }else{
            join(variables, results_table, current_triple_pattern);
        }*/
        join(variables, results_table, current_triple_pattern);
        cout << "time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
        cout << "number of results: " << results_table.size() << endl;
        if(show_results){
            variables.print_all();
            for(auto line:results_table){
                line.print_all();
            }
        }

    }
    /*
    JoinResults res;
    // Naive join order
    for(int i = 0; i < query_table.size(); ++i){
        vector<vector<ID_TYPE>> tmp_table;
        vector<string> tmp_varibales;
        clock_t start, end;
        start = clock();
        tie(tmp_varibales, tmp_table) = this -> query_triple_pattern(query_table[i][0], query_table[i][1], query_table[i][2]);
        end = clock();
        cout << "Triple patter search time: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << " ms" << endl;
        if(i == 0){
            res = JoinResults(tmp_table, tmp_varibales);
        }else{
            res.join(tmp_table, tmp_varibales);
        }
    }*/

    return make_tuple(variables, results_table);
}

/*
void RDFStorage::merge_join(JoinVariables &variables, list<JoinLine> &table_a, vector<string> &triple_pattern) {

    if(triple_store.get_ts_type() == PSO){
        if(variables[0] == triple_pattern[0]){
            unsigned long long index0, index1_begin = 1, index1_end = 0;
            if(triple_pattern[2][0] != '?'){
                for(auto line: table_a){
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[0] = dict_instances.look_up_id(line[0]);
                    if(triple_store.verify_triple_merge_pos1(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[0]), dict_instances.look_up_instance(tmp_query_triple[2]), index0, index1_begin, index1_end)){
                        new_results_table.push_back(line);
                        if(index1_begin == index1_end){
                            break;
                        }
                    }

                }

            }else{
                new_variables.push_back(triple_pattern[2]);
                for (auto line: table_a) {
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[0] = dict_instances.look_up_id(line[0]);
                    vector<vector<ID_TYPE>> triple_results = triple_store.look_up_ele2_merge(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[0]), index0, index1_begin, index1_end);

                    for(auto res_line: triple_results){
                        vector<ID_TYPE> new_line = line;
                        for(auto ele: res_line){
                            new_line.push_back(ele);
                        }
                        new_results_table.push_back(new_line);
                    }
                }
            }

        }else if(variables[0] == triple_pattern[2]){
            unsigned long long index0, index1, index2_begin = 1, index2_end = 0;
            for(auto line: table_a){
                vector<string> tmp_query_triple = triple_pattern;
                tmp_query_triple[2] = dict_instances.look_up_id(line[0]);
                if(triple_store.verify_triple_merge_pos2(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[0]), dict_instances.look_up_instance(tmp_query_triple[2]), index0, index1, index2_begin, index2_end)){
                    new_results_table.push_back(line);
                    if(index2_begin == index2_end){
                        break;
                    }
                }

            }
        }
    }else if(triple_store.get_ts_type() == POS){
        if(variables[0] == triple_pattern[2]){

            unsigned long long index0, index1_begin = 1, index1_end = 0;
            if(triple_pattern[0][0] != '?'){
                for(auto line: table_a){
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[2] = dict_instances.look_up_id(line[0]);
                    if(triple_store.verify_triple_merge_pos1(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[2]), dict_instances.look_up_instance(tmp_query_triple[0]), index0, index1_begin, index1_end)){
                        new_results_table.push_back(line);
                        if(index1_begin == index1_end){
                            break;
                        }
                    }

                }
            }else{
                new_variables.push_back(triple_pattern[0]);
                for (auto line: table_a) {
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[2] = dict_instances.look_up_id(line[0]);
                    vector<vector<ID_TYPE>> triple_results = triple_store.look_up_ele2_merge(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[2]), index0, index1_begin, index1_end);

                    for(auto res_line: triple_results){
                        vector<ID_TYPE> new_line = line;
                        for(auto ele: res_line){
                            new_line.push_back(ele);
                        }
                        new_results_table.push_back(new_line);
                    }
                }
            }

        }else if(variables[0] == triple_pattern[0]){
            unsigned long long index0, index1, index2_begin = 1, index2_end = 0;
            for(auto line: table_a){
                vector<string> tmp_query_triple = triple_pattern;
                tmp_query_triple[0] = dict_instances.look_up_id(line[0]);
                if(triple_store.verify_triple_merge_pos2(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[2]), dict_instances.look_up_instance(tmp_query_triple[0]), index0, index1, index2_begin, index2_end)){
                    new_results_table.push_back(line);
                    if(index2_begin == index2_end){
                        break;
                    }
                }

            }
        }
    }else{
        cerr << "Triple store type not defined!" << endl;
        exit(1);
    }
    return make_tuple(new_variables, new_results_table);
}*/

void RDFStorage::store_to_disk(const string &path) {
    triple_store.store_to_disk(path);
    data_triple_store->store_to_disk(path);
    dict_instances.store_to_disk(path, "dict_instances");
    dict_properties.store_to_disk(path, "dict_properties");
    dict_concepts.store_to_disk(path, "dict_concepts");
    type_store.store_to_disk(path);
}

void RDFStorage::back_up_from_disk(const string &path){
    triple_store.back_up_from_disk(path);
}


tuple<string,string,string> RDFStorage::triple_string_parser(string &str) {
    unsigned long index_begin = 0, triple_index = 0;
    string triple[3];
    char last_sym = '0';
    for(unsigned long i = 0; i < str.size(); ++i){
        if((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')){
            continue;
        }else if(str[i] == '?' && last_sym == '0'){
            index_begin = i;
            last_sym = '?';
        }else if(str[i] == '<' && last_sym == '0'){
            index_begin = i;
            last_sym = '<';
        }else if(str[i] == '\"' && last_sym != '\"'){
            index_begin = i;
            last_sym = '\"';
        }else if(last_sym == '0' && str[i] >= '0' && str[i] <= '9'){
            index_begin = i;
            last_sym = '1';
        }else if(str[i] == '>' && last_sym == '<'){
            triple[triple_index] = str.substr(index_begin, i - index_begin + 1);
            ++triple_index;
            last_sym = '0';
        }else if(str[i] == '\"' && last_sym == '\"'){
            // in case of "qs dfza"^^xsd:string
            last_sym = 'e';
        }else if(str[i] == ' ' && (last_sym == 'e' || last_sym == '?' || last_sym == '1')){
            triple[triple_index] = str.substr(index_begin, i - index_begin);
            ++triple_index;
            last_sym = '0';
        }
        if(triple_index >=3)
            break;
    }
    return make_tuple(triple[0], triple[1], triple[2]);
}



void RDFStorage::load_concepts(ifstream &concepts_file) {
    string str;
    bool total_length_calculated = false;
    while(getline(concepts_file, str, '\n')){
        stringstream input(str);
        string concept, str_id, str_id_range, str_local_length, str_last_bit;
        input >> concept >> str_id >> str_id_range >> str_local_length >> str_last_bit;

        ID_TYPE id = stol(str_id);
        ID_TYPE local_length = stol(str_local_length);
        ID_TYPE last_bit = stol(str_last_bit);

        if(!total_length_calculated){
            ID_TYPE length = (ID_TYPE)(log((double)id)/log(2.0));
            dict_concepts.update_total_encoding_length(length);
            total_length_calculated = true;
        }

        dict_concepts.add(concept, id, local_length, last_bit);

    }

}

void RDFStorage::extend(string path, TripleStoreType ts_type) {
        ifstream extension_file;
        extension_file.open(path);


        // comment if use rdf:type store
        //dict_properties.add("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>", 0);

        ID_TYPE last_instance_id = 0;
        ID_TYPE last_property_id = 0;
        ID_TYPE last_concept_id = 0;

        vector<ID_TYPE> s_vector;
        vector<ID_TYPE> p_vector;
        vector<ID_TYPE> o_vector;

        vector<ID_TYPE> str_s_vector;
        vector<ID_TYPE> str_p_vector;
        vector<string> str_o_vector;

        string str;
        vector<RankBaseElement<3>> triple_vector;
        vector<StrDataRankBaseElement<2>> str_triple_vector;

        // For each fact, if instance doesn't exist, ditribute an id_instance
        //                if concept doesn't exist, remind to check the TBOX
        //                if property doesn't exist, ditribute an id_property
        auto start = clock();
        while(getline(extension_file, str, '\n')){
            string subject, predicate, object;
            tie(subject, predicate, object) = triple_string_parser(str);
            ID_TYPE id_s, id_p, id_o;
            bool is_data_propety = dict_properties.is_data_type(predicate);

            // Update dictionary, search id_s
            try {
                id_s = dict_instances.look_up_instance(subject);
            }catch(DictException& excep){
                ++last_instance_id;
                dict_instances.add(subject, last_instance_id);
                id_s = last_instance_id;
            }

            // Can be optimized with LiteMat, search id_p
            if(predicate != RDF_TYPE) {
                try {
                    id_p = dict_properties.look_up_instance(predicate);
                } catch (DictException &excep) {
                    cerr << str << endl;
                    cerr << subject << " " << predicate << " " << object << endl;
                    /*
                    ++last_property_id;
                    dict_properties.add(predicate, last_property_id);
                    id_p = last_property_id;
                     */
                    cerr << "Property doesn't exist, load TBOX first!" << endl;
                    continue;
                }
            }


            // If predicate == rdf:type, object is a concept, otherwise object is an instance. search id_o
            if(/*true*/predicate != RDF_TYPE){
                if(!is_data_propety){
                    try {
                        id_o = dict_instances.look_up_instance(object);
                    }catch(DictException& excep){
                        ++last_instance_id;
                        dict_instances.add(object, last_instance_id);
                        id_o = last_instance_id;
                    }
                    // In order of PSO or POS
                    ID_TYPE tmp[3];
                    if(ts_type == PSO){
                        tmp[0] = id_p;
                        tmp[1] = id_s;
                        tmp[2] = id_o;
                    }else if(ts_type == POS){
                        tmp[0] = id_p;
                        tmp[1] = id_o;
                        tmp[2] = id_s;
                    }else{
                        cerr << "Triple store type not defined!" << endl;
                        exit(1);
                    }


                    //cout << "new triple: " << id_s << " " << id_p << " " << id_o << endl;
                    triple_vector.push_back(RankBaseElement<3>(tmp));
                }else{
                    //Only PSO
                    ID_TYPE tmp[2];
                    tmp[0] = id_p;
                    tmp[1] = id_s;

                    string* pt_o =  new string(object);

                    str_triple_vector.push_back(StrDataRankBaseElement<2>(tmp, pt_o));

                    //cout << *pt_o << " " << pt_o << endl;
                    //cout << *(str_triple_vector.back().get_str_pt()) << endl;
                }

            }else{ //Will be optimized with LiteMat

                try {
                    id_o = dict_concepts.look_up_instance(object);
                    try {
                        if (!type_store.contains(id_s, id_o)) {
                            type_store.add(id_s, id_o);
                        }
                    }catch(RDFTypeException& except){
                        cout << "Is " << object << " the most precise concept of " << subject << "? Y/N" << endl;
                        char answer;
                        cin >> answer;
                        while(answer != 'Y' && answer != 'N' && answer != 'y' && answer != 'n'){
                            cout << "Please type Y/N!" << endl;
                            cin >> answer;
                        }
                        if(answer == 'Y' || answer == 'y'){
                            type_store.add(id_s, id_o, true);
                        }
                    }
                }catch(DictException& excep){
                    // Will be optimized with LiteMat
                    cerr << subject << " " << predicate << " " << object << endl;
                    cerr << "Concept doesn't exist! Load the TBOX first!" << endl;
                    continue;
                    /*
                    ++last_concept_id;
                    dict_concepts.add(object, last_concept_id);
                    id_o = last_concept_id;
                    type_store.add(id_s, id_o);
                     */
                }


            }
        }
        cout << "Parse each line to creat dictionary and store triples in a vector time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
        cout << "Last instance: " << last_instance_id << endl;
        //cout << "Last proprety: " << last_property_id << endl;

        if(triple_vector.size() <= 0){
            cerr << "Triple vector size is 0!\n";
            exit(1);
        }
        start = clock();
        if(triple_vector.size()>0)
            quickSortTS<RankBaseElement<3>>(triple_vector, 0, triple_vector.size() - 1);
        if(str_triple_vector.size()>0)
            quickSortTS<StrDataRankBaseElement<2>>(str_triple_vector, 0, str_triple_vector.size() - 1);
        cout << "Qsort vector of triples time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;

        extension_file.close();


    // Creat wt structure storage, In order of PSO or POS
    start = clock();
    if(ts_type == PSO){
        auto innerstart = clock();
        for(unsigned long i = 0; i < triple_vector.size(); ++i){
            if(i > 0 && triple_vector[i][0] == triple_vector[i-1][0] && triple_vector[i][1] == triple_vector[i-1][1] && triple_vector[i][2] == triple_vector[i-1][2])
                continue;
            p_vector.push_back(triple_vector[i][0]);
            s_vector.push_back(triple_vector[i][1]);
            o_vector.push_back(triple_vector[i][2]);
            //cout << triple_vector[i][0] << " " << triple_vector[i][1] << " " << triple_vector[i][2] << endl;
        }
        for(unsigned long i = 0; i < str_triple_vector.size(); ++i){
            if(i > 0 && str_triple_vector[i][0] == str_triple_vector[i-1][0] && str_triple_vector[i][1] == str_triple_vector[i-1][1] && str_triple_vector[i].get_str_pt() == str_triple_vector[i-1].get_str_pt())
                continue;
            str_p_vector.push_back(str_triple_vector[i][0]);
            str_s_vector.push_back(str_triple_vector[i][1]);
            str_o_vector.push_back(*str_triple_vector[i].get_str_pt());
            //cout << str_triple_vector[i][0] << " " << str_triple_vector[i][1] << " " << *str_triple_vector[i].get_str_pt() << endl;
        }

        cout << "   -Push each line into P,S,O vectors: " << (double)(clock() - innerstart)/CLOCKS_PER_SEC*1000 << endl;
        innerstart =clock();
        triple_store.extend(p_vector, s_vector, o_vector, ts_type);
        data_triple_store->extend(str_p_vector, str_s_vector, str_o_vector);
        cout << "  | Extend wt structure with separate P,S,O vectors: " << (double)(clock() - innerstart)/CLOCKS_PER_SEC*1000 << endl;
    }else{
        cerr << "Triple store type not supported!" << endl;
        exit(1);
    }
    cout << "Extend triple store time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    }
;
