
//
// Created by weiqin xu on 20/01/2020.
//

#include "DataTypeStore.hpp"
#include <time.h>

using arrow::DoubleBuilder;
using arrow::ListBuilder;

DataTypeStore::DataTypeStore(){};

DataTypeStore::DataTypeStore(std::vector<ID_TYPE> &ele0_vector, std::vector<ID_TYPE> &ele1_vector,
                             std::vector<string> &ele2_vector){
    load(ele0_vector, ele1_vector, ele2_vector);

    for(auto i: *vec_ele2){
        vec_ele2_data.push_back(DataSeries<double>());
    }

}

void DataTypeStore::load(std::vector<ID_TYPE> &ele0_vector, std::vector<ID_TYPE> &ele1_vector,
                         std::vector<string> &ele2_vector) {
    ID_TYPE last_ele0 = -1, last_ele1 = -1;
    string ele0_str, ele1_str;

    vector<int> ele1_bit_map_vec, ele2_bit_map_vec;


    // Construct the string of ele0, ele1, ele2 and bit_maps
    auto start = clock();
    for(unsigned long long i = 0; i < ele0_vector.size(); ++i){


        if(ele1_vector[i] != last_ele1){
            last_ele1 = ele1_vector[i];
            ele1_str += to_string(ele1_vector[i]);
            ele1_str += " ";

            ele2_bit_map_vec.push_back(1);

            if(ele0_vector[i] != last_ele0){
                last_ele0 = ele0_vector[i];
                ele0_str += to_string(ele0_vector[i]);
                ele0_str += " ";

                ele1_bit_map_vec.push_back(1);
            }
            else{
                ele1_bit_map_vec.push_back(0);
            }
        }
        else{
            ele2_bit_map_vec.push_back(0);
        }
    }
    cout << endl << "DataTripleStore: " << endl;
    cout << "  |    -Construct the string of ele0, ele1, ele2 and bit_maps: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;

    start = clock();
    construct_im(wt_ele0, ele0_str, 'd');
    cout << "  |   | Call construct_im to create ele0 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();

    //construct_im(wt_ele1_bit_map, ele1_bit_map_str, 'd');
    bit_vector ele1_bit_vector(ele1_bit_map_vec.size(), 0);
    for(auto i = 0; i < ele1_bit_map_vec.size(); ++i){
        ele1_bit_vector[i] = ele1_bit_map_vec[i];
    }
    wt_ele1_bit_map = new BitMap(ele1_bit_vector);

    cout << "  |   | Call construct_im to create ele01 bit map wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    construct_im(wt_ele1, ele1_str, 'd');
    cout << "  |   | Call construct_im to create ele1 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    //construct_im(wt_ele2_bit_map, ele2_bit_map_str, 'd');

    bit_vector ele2_bit_vector(ele2_bit_map_vec.size(), 0);
    for(auto i = 0; i < ele2_bit_map_vec.size(); ++i){
        ele2_bit_vector[i] = ele2_bit_map_vec[i];
    }
    wt_ele2_bit_map = new BitMap(ele2_bit_vector);

    cout << "  |   | Call construct_im to create ele12 bit map wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    vec_ele2 = new vector<string>;
    (*vec_ele2).swap(ele2_vector);
    cout << "  |   | create ele2 vector: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;



}

// For triple pattern: ele0 ?ele1 ele2
std::vector<JoinLine> DataTypeStore::look_up_ele1(ID_TYPE ele0, const string& ele2){
    std::vector<JoinLine> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0 + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0 + 1 < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0 + 2, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();


    for(auto index1 = index1_begin; index1 < index1_end; ++index1){
        unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
        unsigned long long index2_end = [&]{
            unsigned long long index_tmp;
            if(index1_end < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(),1)){
                index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
            }else{
                index_tmp = wt_ele2_bit_map->size();
            }
            return index_tmp;
        }();

        if(range_search_string_data(index2_begin, index2_end, ele2) >= 0){
            JoinLine tmp_result;
            tmp_result.add(wt_ele1[index1], INS);
            results.push_back(tmp_result);
        }
    }

    return results;
}

std::vector<JoinLine> DataTypeStore::look_up_ele1_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end,
                                                              const string &ele2) {
    std::vector<JoinLine> results;
    unsigned long long index0_begin;
    bool trig = false;
    if(wt_ele0.rank(wt_ele0.size(), ele0_begin) == 0){
        for(auto i = 0; i < (wt_ele0.size() - 1); ++i){
            if(wt_ele0[i] > ele0_begin && wt_ele0[i + 1] <= ele0_begin){
                index0_begin = i + 1;
                trig = true;
                break;
            }
        }
        if(!trig){
            return results;
        }
    }else{
        index0_begin = wt_ele0.select(1, ele0_begin);
    }
    unsigned long long index0_end = index0_begin + 1;
    while(index0_end < wt_ele0.size() && wt_ele0[index0_end] < ele0_end){
        ++index0_end;
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0_begin + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0_end < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0_end + 1, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();


    for(auto index1 = index1_begin; index1 < index1_end; ++index1){
        unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
        unsigned long long index2_end = [&]{
            unsigned long long index_tmp;
            if(index1_end < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(),1)){
                index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
            }else{
                index_tmp = wt_ele2_bit_map->size();
            }
            return index_tmp;
        }();

        if(range_search_string_data(index2_begin, index2_end, ele2) >= 0){
            JoinLine tmp_result;
            tmp_result.add(wt_ele1[index1], INS);
            results.push_back(tmp_result);
        }
    }

    return results;
}

////

std::vector<JoinLine> DataTypeStore::look_up_ele2(ID_TYPE ele0, ID_TYPE ele1) {
    std::vector<JoinLine> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }


    unsigned long long index1_begin = wt_ele1_bit_map->select(index0 + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0 + 1 < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0 + 2, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();

    auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
    if(temp.first == 0){
        return results;
    }

    unsigned long long index1 = temp.second[0].first;

    unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
    unsigned long long index2_end = [&]{
        unsigned long long index_tmp;
        if(index1 + 1 < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(),1)){
            index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
        }else{
            index_tmp = wt_ele2_bit_map->size();
        }
        return index_tmp;
    }();

    for(unsigned long long i = index2_begin; i < index2_end; ++i){

        JoinLine tmp_result;
        //cout << "out before add: " << *(vec_ele2_data[i].get_output()) << endl;
        tmp_result.add(vec_ele2_data[i].get_output(), STR_DATA);
        tmp_result.add(vec_ele2_data[i].get_data_series());

        results.push_back(tmp_result);
    }

    return results;
}

std::vector<JoinLine> DataTypeStore::look_up_ele2_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end, ID_TYPE ele1) {
    std::vector<JoinLine> results;

    unsigned long long index0_begin;
    bool trig = false;
    if(wt_ele0.rank(wt_ele0.size(), ele0_begin) == 0){
        for(auto i = 0; i < (wt_ele0.size() - 1); ++i){
            if(wt_ele0[i] > ele0_begin && wt_ele0[i + 1] <= ele0_begin){
                index0_begin = i + 1;
                trig = true;
                break;
            }
        }
        if(!trig){
            return results;
        }
    }else{
        index0_begin = wt_ele0.select(1, ele0_begin);
    }
    unsigned long long index0_end = index0_begin + 1;
    while(index0_end < wt_ele0.size() && wt_ele0[index0_end] < ele0_end){
        ++index0_end;
    }


    unsigned long long index1_begin = wt_ele1_bit_map->select(index0_begin + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0_end < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0_end + 1, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();

    auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
    if(temp.first == 0){
        return results;
    }

    unsigned long long index1 = temp.second[0].first;

    unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
    unsigned long long index2_end = [&]{
        unsigned long long index_tmp;
        if(index1 + 1 < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(),1)){
            index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
        }else{
            index_tmp = wt_ele2_bit_map->size();
        }
        return index_tmp;
    }();

    for(unsigned long long i = index2_begin; i < index2_end; ++i){

        JoinLine tmp_result;
        tmp_result.add(vec_ele2_data[i].get_output(), STR_DATA);
        tmp_result.add(vec_ele2_data[i].get_data_series());

        results.push_back(tmp_result);
    }

    return results;
}

////

std::vector<JoinLine> DataTypeStore::look_up_ele12(ID_TYPE ele0) {
    std::vector<JoinLine> results;
    unsigned long long index0;

    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0 + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0 + 1 < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0 + 2, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();

    for(auto index1 = index1_begin; index1 < index1_end; ++index1){
        unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
        unsigned long long index2_end = [&]{
            unsigned long long index_tmp;
            if(index1_end < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(),1)){
                index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
            }else{
                index_tmp = wt_ele2_bit_map->size();
            }
            return index_tmp;
        }();

        for(auto index2 = index2_begin; index2 < index2_end; ++index2){
            JoinLine tmp_result;
            tmp_result.add(wt_ele1[index1], INS);
            tmp_result.add(vec_ele2_data[index2].get_output(), STR_DATA);
            tmp_result.add(vec_ele2_data[index2].get_data_series());
            results.push_back(tmp_result);
        }
    }

    return results;
}

std::vector<JoinLine> DataTypeStore::look_up_ele12_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end) {
    std::vector<JoinLine> results;

    unsigned long long index0_begin;
    bool trig = false;
    if(wt_ele0.rank(wt_ele0.size(), ele0_begin) == 0){
        for(auto i = 0; i < (wt_ele0.size() - 1); ++i){
            if(wt_ele0[i] > ele0_begin && wt_ele0[i + 1] <= ele0_begin){
                index0_begin = i + 1;
                trig = true;
                break;
            }
        }
        if(!trig){
            return results;
        }
    }else{
        index0_begin = wt_ele0.select(1, ele0_begin);
    }
    unsigned long long index0_end = index0_begin + 1;
    while(index0_end < wt_ele0.size() && wt_ele0[index0_end] < ele0_end){
        ++index0_end;
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0_begin + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0_end < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0_end + 1, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();


    for(auto index1 = index1_begin; index1 < index1_end; ++index1){
        unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
        unsigned long long index2_end = [&]{
            unsigned long long index_tmp;
            if(index1_end < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(),1)){
                index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
            }else{
                index_tmp = wt_ele2_bit_map->size();
            }
            return index_tmp;
        }();

        for(auto index2 = index2_begin; index2 < index2_end; ++index2){
            JoinLine tmp_result;
            tmp_result.add(wt_ele1[index1], INS);
            tmp_result.add(vec_ele2_data[index2].get_output(), STR_DATA);
            tmp_result.add(vec_ele2_data[index2].get_data_series());
            results.push_back(tmp_result);
        }
    }

    return results;
}

////

std::vector<JoinLine> DataTypeStore::verify_triple(ID_TYPE ele0, ID_TYPE ele1, const string& ele2) {
    std::vector<JoinLine> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0 + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0 + 1 < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0 + 2, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();

    auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
    if(temp.first == 0){
        return results;
    }

    unsigned long long index1 = temp.second[0].first;

    unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
    unsigned long long index2_end = [&] {
        unsigned long long index_tmp;
        if (index1 + 1 < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(), 1)) {
            index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
        } else {
            index_tmp = wt_ele2_bit_map->size();
        }
        return index_tmp;
    }();

    if(range_search_string_data(index2_begin, index2_end, ele2) >= 0){
        JoinLine line;
        line.add(ele0, PRO);
        results.push_back(line);
    }

    return results;
}

std::vector<JoinLine> DataTypeStore::verify_triple_with_reason(ID_TYPE ele0_begin, ID_TYPE ele0_end, ID_TYPE ele1,
                                                               const string &ele2) {
    std::vector<JoinLine> results;

    unsigned long long index0_begin;
    bool trig = false;
    if(wt_ele0.rank(wt_ele0.size(), ele0_begin) == 0){
        for(auto i = 0; i < (wt_ele0.size() - 1); ++i){
            if(wt_ele0[i] > ele0_begin && wt_ele0[i + 1] <= ele0_begin){
                index0_begin = i + 1;
                trig = true;
                break;
            }
        }
        if(!trig){
            return results;
        }
    }else{
        index0_begin = wt_ele0.select(1, ele0_begin);
    }
    unsigned long long index0_end = index0_begin + 1;
    while(index0_end < wt_ele0.size() && wt_ele0[index0_end] < ele0_end){
        ++index0_end;
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0_begin + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0_end < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0_end + 1, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();

    auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
    if(temp.first == 0){
        return results;
    }

    unsigned long long index1 = temp.second[0].first;

    unsigned long long index2_begin = wt_ele2_bit_map->select(index1 + 1, 1);
    unsigned long long index2_end = [&] {
        unsigned long long index_tmp;
        if (index1 + 1 < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(), 1)) {
            index_tmp = wt_ele2_bit_map->select(index1 + 2, 1);
        } else {
            index_tmp = wt_ele2_bit_map->size();
        }
        return index_tmp;
    }();

    if(range_search_string_data(index2_begin, index2_end, ele2) >= 0){
        JoinLine line;
        line.add(ele1, INS);
        results.push_back(line);
    }

    return results;
}

unsigned long long DataTypeStore::look_up_ele0_num(ID_TYPE ele0) {
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return 0;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = wt_ele1_bit_map->select(index0 + 1, 1);
    unsigned long long index1_end = [&]{
        unsigned long long index_tmp;
        if(index0 + 1 < wt_ele1_bit_map->rank(wt_ele1_bit_map->size(),1)){
            index_tmp = wt_ele1_bit_map->select(index0 + 2, 1);
        }else{
            index_tmp = wt_ele1_bit_map->size();
        }
        return index_tmp;
    }();

    unsigned long long index2_begin = wt_ele2_bit_map->select(index1_begin + 1, 1);
    unsigned long long index2_end = [&] {
        unsigned long long index_tmp;
        if (index1_end + 1 < wt_ele2_bit_map->rank(wt_ele2_bit_map->size(), 1)) {
            index_tmp = wt_ele2_bit_map->select(index1_end + 2, 1);
        } else {
            index_tmp = wt_ele2_bit_map->size();
        }
        return index_tmp;
    }();

    return (index2_end - index2_begin);
}

void DataTypeStore::store_to_disk(const string &path) {
    string directory_path;

    if(path[path.length() - 1] == '/'){
        directory_path = path;
    }else{
        directory_path = path + "/";
    }

    string dts_ele0_path = directory_path + "DTS_ele0.st";
    string dts_ele01_bitmap_path = directory_path + "DTS_ele01_bitmap.st";
    string dts_ele1_path = directory_path + "DTS_ele1.st";
    string dts_ele12_bitmap_path = directory_path + "DTS_ele12_bitmap.st";

    sdsl::store_to_file(wt_ele0, dts_ele0_path);
    //sdsl::store_to_file(wt_ele1_bit_map, ele01_bitmap_path);
    wt_ele1_bit_map->store_to_disk(dts_ele01_bitmap_path);
    sdsl::store_to_file(wt_ele1, dts_ele1_path);
    //sdsl::store_to_file(wt_ele1_bit_map, ele12_bitmap_path);
    wt_ele2_bit_map->store_to_disk(dts_ele12_bitmap_path);
    ofstream data_ele2(directory_path + "DTS_ele2.st");
    for(auto data: *vec_ele2){
        data_ele2 << data << endl;
    }
    data_ele2.close();
}

void DataTypeStore::insert_data(vector<long>& data_indexes, vector<double>& data){
    //cout << "Vec_ele2_data size: " << vec_ele2_data.size() << endl;
    for(int i = 0; i < data_indexes.size(); ++i){
        //vec_ele2_data[i].print_mode();
        vec_ele2_data[data_indexes[i]].insert_element(data[i]);
    }
}

void DataTypeStore::insert_data(vector<long>& data_indexes, vector<string>& data){
    //cout << "Vec_ele2_data size: " << vec_ele2_data.size() << endl;
    for(int i = 0; i < vec_ele2_data.size(); ++i){
        //vec_ele2_data[i].print_mode();
        vec_ele2_data[data_indexes[i]].insert_element(stod(data[i]));
    }
}

void DataTypeStore::drop_data() {
    for(auto i = vec_ele2_data.begin(); i != vec_ele2_data.end(); ++i){
        i->remove_element();
    }
}

void DataTypeStore::series_to_arrow_table(const vector<DataSeries<double>> series, std::shared_ptr<arrow::Table>* table) {
    
  arrow::MemoryPool* pool = arrow::default_memory_pool();
  ListBuilder components_builder(pool, std::make_shared<DoubleBuilder>(pool));
  DoubleBuilder& cost_components_builder = *(static_cast<DoubleBuilder*>(components_builder.value_builder()));
  
  // Iterating over every serie
  // should be replaced by an index list
  for (auto serie : series) {
    list<double> *lst = serie.get_data();
    vector<double> vec(lst->size());
    copy(lst->begin(),lst->end(),vec.begin());
    
    components_builder.Append();
    cost_components_builder.AppendValues(vec.data(), vec.size());
  }

  std::shared_ptr<arrow::Array> cost_components_array;
  components_builder.Finish(&cost_components_array);

  std::vector<std::shared_ptr<arrow::Field>> schema_vector = {};
  for (auto data : *vec_ele2) {
    schema_vector.push_back(arrow::field(data, arrow::list(arrow::float64())));
  }

  auto schema = std::make_shared<arrow::Schema>(schema_vector);
  *table = arrow::Table::Make(schema, {cost_components_array});
}

void DataTypeStore::persist_all(){
    std::shared_ptr<arrow::Table> table;
    series_to_arrow_table(vec_ele2_data, &table);
   
    std::shared_ptr<arrow::io::FileOutputStream> outfile;
    PARQUET_ASSIGN_OR_THROW(
      outfile,
      arrow::io::FileOutputStream::Open("test.parquet"));

    PARQUET_THROW_NOT_OK(
      parquet::arrow::WriteTable(*table, arrow::default_memory_pool(), outfile, 3));
}

void DataTypeStore::drop_data_with_index(long index) {
    vec_ele2_data[index].remove_element();
}

void DataTypeStore::drop_all_data() {
    // Parcourir les index, associer les index aux uri
    // créer colonnes parquet
    // save to file
    
    for(auto i = vec_ele2_data.begin(); i != vec_ele2_data.end(); ++i){
        i->remove_all_element();
    }
}

void DataTypeStore::drop_all_data_with_index(long index) {
    vec_ele2_data[index].remove_all_element();
}

void DataTypeStore::change_mode(int index, data_function function) {
    vec_ele2_data[index].change_function(function);
}

void DataTypeStore::all_change_mode(data_function function) {
    for(auto i = vec_ele2_data.begin() + 1; i != vec_ele2_data.end(); ++i){
        i->change_function(function);
        //i->print_mode();
    }
}

long long DataTypeStore::get_head_tail_difference_with_index(long index) {
    return vec_ele2_data[index].get_head_tail_difference();
}

void DataTypeStore::extend(std::vector<ID_TYPE> &ele0_vector, std::vector<ID_TYPE> &ele1_vector,std::vector<string> &ele2_vector) {
    ID_TYPE last_ele0 = -1, last_ele1 = -1;
    int_vector<> ele0_vec(ele0_vector.size()), ele1_vec(ele1_vector.size()), ele2_vec(ele2_vector.size());

    vector<int> ele1_bit_map_vec, ele2_bit_map_vec;


    // Construct the string of ele0, ele1, ele2 and bit_maps
    auto start = clock();
    for(unsigned long long i = 0; i < ele0_vector.size(); ++i){
        ele0_vec[i] = ele0_vector[i];
    }

    for(unsigned long long i = 0; i < ele1_vector.size(); ++i){
        ele1_vec[i] = ele1_vector[i];
    }

    cout << "  |    -Construct the string of ele0, ele1, ele2 and bit_maps: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;

    start = clock();
    wt_ele0.extend(ele0_vec);
    cout << "  |   | Call construct_im to create ele0 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();

    //construct_im(wt_ele1_bit_map, ele1_bit_map_str, 'd');
    bit_vector ele1_bit_vector(ele1_bit_map_vec.size(), 0);
    for(auto i = 0; i < ele1_bit_map_vec.size(); ++i){
        ele1_bit_vector[i] = ele1_bit_map_vec[i];
    }
    wt_ele1_bit_map->extend(ele1_bit_vector);

    cout << "  |   | Call construct_im to create ele01 bit map wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    wt_ele1.extend(ele1_vec);
    cout << "  |   | Call construct_im to create ele1 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    //construct_im(wt_ele2_bit_map, ele2_bit_map_str, 'd');

    bit_vector ele2_bit_vector(ele2_bit_map_vec.size(), 0);
    for(auto i = 0; i < ele2_bit_map_vec.size(); ++i){
        ele2_bit_vector[i] = ele2_bit_map_vec[i];
    }
    wt_ele2_bit_map->extend(ele2_bit_vector);

    cout << "  |   | Call construct_im to create ele12 bit map wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    for(auto el:ele2_vector)
        vec_ele2->push_back(el);
    cout << "  |   | create ele2 vector: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
}
