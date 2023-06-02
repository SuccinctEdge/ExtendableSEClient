#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#include <iostream>
#include "TripleStore.hpp"
#include "RDFStorage.hpp"
#include <ctime>
#include <bitset>
#include "Filter.hpp"
#include "TCPReceiver.hpp"
#include "ShowResults.hpp"
#include "DataWindowUtil.hpp"
#include "MiniBatchUtil.hpp"
#include "Query.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
using namespace sdsl;
using namespace std;

mutex data_update_lock;
vector<Query> running_queries;
mutex query_lock;

void query_reader(data_function mini_batch_mode, bool mini_batch, long window_size, int sliding_step, WindowMode window_mode, RDFStorage test) {

    vector<string> query_strings;
    vector<string> list_data_variables;
    vector<data_function> list_data_mode;
    vector<vector<string>> queries_list;
    vector<string> queries = {};
    vector<long> data_variables_indexes;
    vector<long> variable_indexes;
    JoinVariables variables;
    list<JoinLine> res_table;
    Filter filter;
    string signal;
    clock_t start;
    clock_t end;
    int spread = 0;

    while(true) {
        cout << "Waiting for Query" << endl;
        tie(signal,queries) = getQueries();

        cout << "Received Query, signal : " << signal << endl;

        while (queries.size()>0) {
            string tmp;
            tmp = queries.at(0);
            queries.erase(queries.begin());
            //cout << " tmp: " << tmp << endl;
            if (tmp[0] == 's' || tmp[0] == 'S') {
                istringstream is(tmp);
                string clause_select;
                bool time_range_trigger = false;
                while (is >> clause_select) {
                    if (clause_select.back() == ']' && clause_select.front() == '?') {
                        list_data_variables.push_back(clause_select.substr(0, clause_select.find('[')));
                        if (clause_select.substr(clause_select.find('[')) == "[MAX]") {
                            if (mini_batch) {
                                mini_batch_mode = MAX;
                                list_data_mode.push_back(NONE);
                            } else {
                                list_data_mode.push_back(MAX);
                            }
                        } else if (clause_select.substr(clause_select.find('[')) == "[MIN]") {
                            if (mini_batch) {
                                mini_batch_mode = MIN;
                                list_data_mode.push_back(NONE);
                            } else {
                                list_data_mode.push_back(MIN);
                            }
                        } else if (clause_select.substr(clause_select.find('[')) == "[AVG]") {
                            if (mini_batch) {
                                mini_batch_mode = AVG;
                                list_data_mode.push_back(NONE);
                            } else {
                                list_data_mode.push_back(AVG);
                            }
                        } else {
                            cerr << "Variable " << clause_select.substr(0, clause_select.find('[')) << " unkown mode!"
                                 << endl;
                            list_data_mode.push_back(NONE);
                        }
                    } else if (clause_select[0] == '[') {
                        string str_range, str_number, str_unit, str_window_mode;
                        if (clause_select.size() > 1) {
                            str_range = clause_select.substr(1);
                            is >> str_number >> str_unit >> str_window_mode;
                        } else {
                            is >> str_range >> str_number >> str_unit >> str_window_mode;
                        }

                        //verify range clause
                        transform(str_range.begin(), str_range.end(), str_range.begin(), ::tolower);
                        if (str_range != "range") {
                            cerr << "Must use RANGE to indicate time window size!" << endl;
                        }

                        //Window size number
                        if (str_number[0] >= '0' && str_number[0] <= '9') {
                            window_size = stol(str_number);
                        } else {
                            cerr << "Window size must be a number!" << endl;
                            window_size = 7000;
                        }

                        //Window size Unit
                        transform(str_unit.begin(), str_unit.end(), str_unit.begin(), ::tolower);
                        if (str_unit == "min") {
                            window_size *= 60000;
                        } else if (str_unit == "sec") {
                            window_size *= 1000;
                        } else if (str_unit == "ms") {
                        } else if (str_unit == "h") {
                            window_size *= 3600000;
                        } else {
                            cerr << "Time unit unknown!" << endl;
                            window_size = 7000;
                        }

                        //Window mode
                        if (str_window_mode.back() == ']') {
                            str_window_mode.pop_back();
                        }
                        transform(str_window_mode.begin(), str_window_mode.end(), str_window_mode.begin(), ::tolower);
                        if (str_window_mode == "tumbling") {
                            window_mode = TUMBLING;
                        } else if (str_window_mode == "step") {
                            string sliding_step_str, sliding_step_unit;
                            window_mode = SLIDING;
                            is >> sliding_step_str >> sliding_step_unit;
                            if (sliding_step_unit.back() == ']') {
                                sliding_step_unit.pop_back();
                            }
                            sliding_step = stoll(sliding_step_str);
                            transform(sliding_step_unit.begin(), sliding_step_unit.end(), sliding_step_unit.begin(),
                                      ::tolower);
                            if (sliding_step_unit == "min") {
                                sliding_step *= 60000;
                            } else if (sliding_step_unit == "sec") {
                                sliding_step *= 1000;
                            } else if (sliding_step_unit == "ms") {
                            } else if (sliding_step_unit == "h") {
                                sliding_step *= 3600000;
                            } else {
                                cerr << "Step time unit unknown!" << endl;
                                sliding_step = 1000;
                            }
                        } else {
                            cerr << "Window mode unknown!" << endl;
                        }


                    }

                }

            } else if (tmp[0] == '\t')
                if (tmp.find("FILTER") != string::npos) {
                    size_t start_clause = tmp.find('(') + 1;
                    size_t end_clause = tmp.find(')');
                    string filter_clause = tmp.substr(start_clause, end_clause);

                    vector<string> tokens;
                    size_t pos;
                    while ((pos = tmp.find(' ')) != string::npos) {
                        tokens.push_back(tmp.substr(0, pos));
                        tmp.erase(0, pos + 1);
                    }
                    tokens.push_back(tmp);
                    tokens[1] = tokens[1].substr(1, tokens[1].size());
                    tokens[3] = tokens[3].substr(0, tokens[3].size() - 1);

                    filter = Filter(tokens[1], opFromString(tokens[2]), stod(tokens[3]));

                } else {
                    query_strings.push_back(tmp);
                }
            else if (tmp[0] == '}') {
                cout << "Finished Reading" << endl;
                start = clock();
                tie(variables, res_table) = test.query_graph_pattern(query_strings);
                end = clock();

                cout << "Static Query time: " << (double) (end - start) / CLOCKS_PER_SEC * 1000 << " ms" << endl
                     << endl;

                data_variables_indexes = variables.get_data_variables_indexes(list_data_variables);

                // Query creation
                for (auto var: variables.get_var_vector(STR_DATA)) {
                    variable_indexes.push_back(variables.get_var_map()[var].second);
                }

                for (auto &res_line: res_table) {
                    for (auto j = 0; j < data_variables_indexes.size(); ++j) {
                        res_line.change_mode(data_variables_indexes[j],
                                             list_data_mode[j]); // Fonctionnement probablement cassé quand plusieurs requetes transforment la même DS
                    }                                                  // Solution: Créer une nouvelle DS quand une transformation est demandée, renvoyer et modifier le nouvel index
                }

                query_lock.lock();
                running_queries.emplace_back(variable_indexes, variables.get_var_vector(STR_DATA), signal, filter, res_table, spread);
                query_lock.unlock();
                spread += variable_indexes.size();
                query_strings.clear();
            }
        }
        usleep(1000);
    }
}

        int main(int argc, char *argv[]) {


/*
    bit_vector b = {1,1,0,1,0,0,1};

    bit_vector a = bit_vector(10000, 0);
    for(auto i=0; i < a.size(); i += 1000){
        a[i] = 1;
    }

    rrr_vector<63> rrra(a);

    rrr_vector<63> rrrb;

    if(true){
        rrrb = rrr_vector<63>(a);
    }


    rrr_vector<>::rank_1_type rrrb_rank(&rrrb);


    cout << rrrb_rank(2001) << endl;

*/

            //store_to_file(rrra, "/Users/xu/Documents/Doctorat/git_workplace/triple_store/rrr_test.dt");

            /*
            wt_huff<> wt;
            construct_im(wt, "test_data/wavelet_trees.hpp", 1);
            store_to_file(wt,"wt_index-file.sdsl");
            cout << endl;
            cout << "number of lines  : " << wt.rank(wt.size(), '\n') << endl;
            cout << "first '=' in line: " << wt.rank(wt.select(1, '='),'\n')+1 << endl;
            */
            /*if (argc < 2) {
                return 1;
            }*/


            /*
            ofstream out;
            out.open("temp.tmp", std::ios::binary);
            unsigned long long p = 0b110011111000001111000;
            out.write((char*)&p, sizeof(unsigned long long));
            out.close();

            ifstream in;
            in.open("temp.tmp", std::ios::binary);

            bit_vector bv;
            cout << "Before load: " << bv.size() << endl;
            bv.load(in);
            cout << "After load: " << bv.size() << endl;
            size_t zeros = rank_support_v<0>(&bv)(bv.size());
            bit_vector::select_0_type b_sel(&bv);*/
            /*
            for (size_t i=1; i <= zeros; ++i)
                cout << b_sel(i) << " ";
            */

/*
    wt_hutu_int<rrr_vector<63>> wt;
    string input = "6   1000 1 4 7 3   18 6 3 ";
    construct_im(wt, input, 'd');
    store_to_file(wt,"wt_index-file.sdsl");
    std::ofstream out("wt_index-file.sdsl.html");
    write_structure<HTML_FORMAT>(wt,out);
    cout << "size: " << wt.size() << "\n";
    cout << wt[0] << "\n";
    for (size_t i=0; i < wt.size(); ++i)
        cout << wt[i] << " ";
    cout << endl;
    cout << endl;
    //cout << "first '=' in line: " << wt.rank(wt.select(1, '='),'\n')+1 << endl;

*/
/*
    vector<long> p = {1,1,1,1,1,1,2,2,2,2,3,3,3,3,3,3,4};
    vector<long> s = {1,1,3,3,3,4,2,2,4,4,1,3,3,4,4,4,3};
    vector<long> o = {1,2,1,2,3,1,1,2,1,2,1,1,2,1,2,3,1};

    TripleStore pso(p,s,o);

    vector<long> results = pso.look_up_ele2(5,8);
    for(long i:results){
        cout << i << " ";
    }
    cout << '\n' << "\n" ;

    vector<long> results2 = pso.look_up_ele0(3,8);
    for(long i:results2){
        cout << i << " ";
    }
    cout << '\n' << "\n" ;

    vector<long> results3 = pso.look_up_ele1(8,8);
    for(long i:results3){
        cout << i << " ";
    }
    cout << '\n' << "\n" ;

    vector<tuple<long,long>> results4 = pso.look_up_ele12(8);
    cout << "ele1" << " " << "ele2" << "\n";
    for(auto i:results4){
        cout << std::get<0>(i) << " " << std::get<1>(i) << "\n";
    }
    cout << '\n' << "\n" ;

    vector<tuple<long,long>> results5 = pso.look_up_ele01(8);
    cout << "ele0" << " " << "ele1" << "\n";
    for(auto i:results5){
        cout << std::get<0>(i) << " " << std::get<1>(i) << "\n";
    }
    cout << '\n' << "\n" ;

    vector<tuple<long,long>> results6 = pso.look_up_ele02(8);
    cout << "ele0" << " " << "ele2" << "\n";
    for(auto i:results6){
        cout << std::get<0>(i) << " " << std::get<1>(i) << "\n";
    }
    cout << '\n' << "\n" ;

    vector<tuple<long,long,long>> results7 = pso.look_up_ele012();
    cout << "ele0" << " " << "ele1" << " " << "ele2" << "\n";
    for(auto i:results7){
        cout << std::get<0>(i) << " " << std::get<1>(i) << " " << std::get<2>(i) << "\n";
    }
    cout << '\n' << "\n" ;
*/

/*

        if (argc < 4) {
            cout << "Usage: triple_store data_path query_path storage_path tbox_path reason show_result" << endl;
            //exit(1);
        }


        char *data_path = "../test_data/sensor_test.nt";
        char *query_path = "../test_query/sensor_test.sparql";
        char *storage_path = "../store/sensor_test/";
        char *tbox_path = "../simu_tbox/";

        bool reason = false;
        bool show_result = true;
        bool active_plan = true;
        bool mini_batch = false;



*/

            char *data_path = argv[1];
            char *query_path = argv[2];
            char *storage_path = argv[3];
            char *tbox_path = argv[4];
            char *str_reason = argv[5];
            char *str_show_result = argv[6];


            bool reason = (str_reason[0] == 't' || str_reason[0] == 'T');
            bool show_result = (str_show_result[0] == 't' || str_show_result[0] == 'T');
            bool active_plan = true;
            if (argc > 7) {
                active_plan = (argv[7][0] == 't' || argv[7][0] == 'T');
            }

            bool mini_batch = false;
            if (argc > 8) {
                mini_batch = (argv[8][0] == 't' || argv[8][0] == 'T');
            }

            bool server = false;
            if (argc > 9) {
                server = (argv[9][1] == 's');
            }

            string mode = (server) ? "Server" : "Client";

            cout << "Running as " + mode << endl;
            run_tcp_receiver(server, &data_update_lock);

            clock_t start, end;
            ifstream abox_file;
            //Users/xu/Documents/Doctorat/git_workplace/triple_store/test_data/data.nt
            //Users/xu/Documents/Doctorat/git_workplace/triple_store/test_ABOX.nt


            abox_file.open(data_path);

            if (!abox_file) {
                cerr << "open file error!" << endl;
                exit(1);
            }
            start = clock();

            string str_tbox(tbox_path);

            RDFStorage test(str_tbox, abox_file, PSO);
            string topic_file = "test_topic_map/topic_map.txt";
            test.init_topic_cols_map(topic_file);
            end = clock();


            string inp;


            /*
            do{
                cout << "Continue to query? y/n" << endl;
                cin >> inp;
            }while(inp != "y");
             */



            test.set_reason(reason);
            test.set_show_results(false);
            test.store_to_disk(storage_path);
            test.set_active_plan(active_plan);


            cout << "Database constructed! " << (double) (end - start) / CLOCKS_PER_SEC << " s" << endl << endl;

            abox_file.close();

            //test.extend("./test_data/lubmEX.ttl",PSO);

            //////////////

            ifstream query_file;
            query_file.open(query_path);
            vector<string> query_strings;

            while(!query_file.eof()){
                string tmp;
                getline(query_file, tmp);
                cout << tmp << endl;
                if(tmp[0] == '\t')
                    query_strings.push_back(tmp);
                else if(tmp[0] == '}'){
                    JoinVariables variables;
                    list<JoinLine> res_table;
                    start = clock();
                    tie(variables, res_table) = test.query_graph_pattern(query_strings);
                    end = clock();
                    cout << "Query time: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << " ms" << endl << endl;
                    cout << endl << endl;
                    query_strings.clear();
                }
            }

            //////////////

            //vector<double> data{103324, 18.015, 3.4569};
            //test.insert_numeric_data(data);

            bool data_window_thread_stop = false;
            bool tumbling_out_flag = false;
            long window_size = 7000;
            WindowMode window_mode = SLIDING;
            long long sliding_step = 1000;
            //test.all_change_data_mode(MAX);

            //vector<long> data_indexes = test.get_indexes_with_data_names(columns);


            //Lance data window control thread
            thread data_window_control_thread = thread(dataWindowControl, ref(test), ref(data_update_lock),
                                                       ref(window_size), ref(data_window_thread_stop), ref(window_mode),
                                                       ref(tumbling_out_flag));


            data_function mini_batch_mode = NONE;

            thread query_thread = thread(query_reader, ref(mini_batch_mode), ref(mini_batch),
                                         ref(window_size), ref(sliding_step), ref(window_mode), ref(test));

            vector<vector<double>> mini_batch_data_window;

            long long step_timer = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
            //test.store_to_disk("./persistence");
            while (true) {
                for (const tuple<string, vector<vector<string>>> &tuple: getCSV()) {
                    vector<vector<string>> table;
                    string sensor;
                    std::tie(sensor, table) = tuple;
                    for (auto i: table) {
                        if (!mini_batch) {
                            vector<double> data_line;

                            for (auto j: i) {
                                data_line.push_back(std::stod(j));
                            }

                            test.insert_numeric_data_with_topic(sensor, data_line, data_update_lock);

                            if (show_result) {
                                if (window_mode == TUMBLING) {
                                    //cout << "flag: " << tumbling_out_flag << endl;
                                    if (tumbling_out_flag) {
                                        //sendResults(test, variables, res_table, filter);
                                        query_lock.lock();
                                        displayAllQueryResults(running_queries);
                                        query_lock.unlock();
                                        tumbling_out_flag = false;
                                    }
                                } else {
                                    //cout << "step: " << sliding_step << " " << std::chrono::system_clock::now().time_since_epoch() /std::chrono::milliseconds(1) << " " << step_timer << endl;
                                    long long time_now = std::chrono::system_clock::now().time_since_epoch() /
                                                         std::chrono::milliseconds(1);
                                    if (time_now - step_timer >= sliding_step) {
                                        //sendResults(test, variables, res_table, filter);
                                        query_lock.lock();
                                        displayAllQueryResults(running_queries);
                                        query_lock.unlock();
                                        step_timer = time_now;
                                    }
                                }

                            }
                        } else {

                            if (i[0][1] != 'e') {
                                vector<double> data_line;
                                for (auto j: i) {
                                    //cout << j << " ";
                                    data_line.push_back(std::stod(j));
                                }
                                mini_batch_data_window.push_back(data_line);
                            } else {
                                vector<double> mini_batch_result = calculate_mini_batch(mini_batch_data_window,
                                                                                        mini_batch_mode);
                                test.insert_numeric_data_with_topic(sensor, mini_batch_result, data_update_lock);
                                if (show_result) {
                                    //sendResults(test, variables, res_table, filter);
                                    query_lock.lock();
                                    displayAllQueryResults(running_queries);
                                    query_lock.unlock();
                                }
                                long long time = std::chrono::system_clock::now().time_since_epoch() /
                                                 std::chrono::milliseconds(1);
                                mini_batch_data_window.clear();
                            }
                        }
                    }
                }
                usleep(100000);
            }

            //std::cout << std::endl;
            //vector<double> data2{103344, 20.000, 2.4569};
            //test.all_change_data_mode(AVG);
            //test.insert_numeric_data(data2);
            //test.drop_numeric_data();
            cout << endl << endl;

            data_window_thread_stop = true;
            data_window_control_thread.join();
            stop_tcp_receiver();







/*
    query_strings.push_back("?v0 <http://db.uwaterloo.ca/~galuc/wsdbm/subscribes> <http://db.uwaterloo.ca/~galuc/wsdbm/Website174> .");
    query_strings.push_back("?v0 <http://db.uwaterloo.ca/~galuc/wsdbm/likes> ?v2 .");
    query_strings.push_back("?v2 <http://schema.org/caption> ?v3 .");

    //query_strings.push_back("?v4 <http://db.uwaterloo.ca/~galuc/wsdbm/makesPurchase> ?v5 .");
    //query_strings.push_back("?v5 <http://db.uwaterloo.ca/~galuc/wsdbm/purchaseDate> ?v6 .");
    //query_strings.push_back("?v5 <http://db.uwaterloo.ca/~galuc/wsdbm/purchaseFor> ?v0 .");



    vector<string> variables;
    vector<vector<ID_TYPE>> res_table;
    tie(variables, res_table) = test.query_graph_pattern(query_strings);

    for(auto i: variables){
        cout << i << " ";
    }
    cout << endl;
    for(auto i: res_table){
        for(auto j: i){
            cout << test.look_up_id_instance(j) << " ";
        }
        cout << endl;
    }
*/

            /*
            string str;
            cout << "Input triple pattern: S P O: " << endl;
            while(getline(cin, str, '\n')){
                if(str == "exit")
                    return 0;
                stringstream input(str);
                string subject, predicate, object;
                input >> subject >> predicate >> object;
                vector<vector<ID_TYPE>> results;
                vector<string> variables;
                tie(variables, results) = test.query_triple_pattern(subject, predicate, object);
                cout << variables.size() << " " << results.size() << endl;
                cout << "Input triple pattern: S P O:" << endl;
            }
            */

            /*
            vector<vector<ID_TYPE >> table_a = {{1,2,3},{1,5,6},{7,2,9},{10,11,6}};
            vector<vector<ID_TYPE >> table_b = {{1,1},{7,7},{10,2}};
            vector<string> var_a = {"a","b","c"};
            vector<string> var_b = {"a","d"};
            JoinResults res(table_a, var_a);
            JoinResults res2(table_b, var_b);
            res.join(res2);
            res.print_ele();
            */
            return 0;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop