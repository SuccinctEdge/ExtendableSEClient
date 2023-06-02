#include "sdsl/int_vector.hpp"
#include "sdsl/rrr_vector_extendable.hpp"
#include "sdsl/rrr_vector.hpp"
//#include "sdsl/rrr_vector_extendable.hpp"
#include "sdsl/util.hpp"
#include "gtest/gtest.h"
#include <vector>
#include <string>

int main(int argc, char** argv){
    //{1,0,1,0,1    ,0,0,0,0,1 ,0,0,1,1,0 ,1,1,1,0,1    ,0,0,1,0,0  ,0,0,1,0,1 ,0,1,1,0,1    ,0,0,0,0,0  ,0,0,1,0,1}
    //sdsl::int_vector<1> init = {1,0,1,0,1    ,0,0,0,0,1 ,0,0,1,1,0 ,1,1,1,0,1    ,0,0,1,0,0  ,0,0,1,0,1 ,0,1,1,0,1    ,0,0,0,0,0  ,0,0,1,0,1};
    //sdsl::int_vector<1> ex = {0,0,0,0,1};
    //vec->extend(ex);

    sdsl::int_vector<1> init = {1,0,1,0,1 ,0,0,0,0,0 ,0,0,0,0,0    ,0,0,0,0,0 ,0,0,0,0,0 ,0,0,0,0,0     ,0,0,0,0,0 ,0,0,0,0,0 ,0,0,1,0,1};
    auto *vec = new rrr_vector_extendable<5, int_vector<>, 3>(init);
    sdsl::int_vector<1> bv;
    auto *vec2 = new rrr_vector_extendable<5, int_vector<>, 3>(bv);
    auto *vec_rrr = new rrr_vector<5,int_vector<>,3>(init);

    std::cout << "taille:"  << vec->size() << "\n";
    vec2->extend(init);
/*
    rank_support_rrr<1, 5,int_vector<>, 3> vec_rank_rrr;
    vec_rank_rrr.set_vector(vec_rrr);
    select_support_rrr<1, 5,int_vector<>, 3> vec_select_rrr;
    vec_select_rrr.set_vector(vec_rrr);
*/
    rank_support_rrr_ex<1, 5,int_vector<>, 3> vec_rank;
    vec_rank.set_vector(vec);
    select_support_rrr_ex<1, 5,int_vector<>, 3> vec_select;
    vec_select.set_vector(vec);

    //rank_support_rrr_ex<1, 8,int_vector<>, 3> vec_rank2;
    //vec_rank2.set_vector(vec);
    //select_support_rrr_ex<1, 8,int_vector<>, 3> vec_select2;
    //vec_select2.set_vector(vec2);
    long start;
    long end;

    for(int i = 0;i < 6; i++){
        cout <<i << " : " << vec_select.select(i) << endl;
    }

/*
    start = clock();
    for(int i = 0;i<180000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_rank.m_v->bv_rank.rank(i);

    }
    end = clock();
    cout << "ranks: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;


    start = clock();
    for(int i = 0;i<180000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_rank_rrr.rank(i);
    }
    end = clock();
    cout << "ranks rrr: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    for(int i = 0;i<90000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_select_rrr.select(i);
    }

    for(int i = 0;i<90000;i++){
        //cout << vec_rank.rank(i) << endl;
        //vec_select.select(i);
        vec_rank.m_v->bv_select.select(i);
    }
    cout << "10000" << endl;

    start = clock();
    for(int i = 0;i<10000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_select_rrr.select(i);
    }
    end = clock();
    cout << "select rrr: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<10000;i++){
        //cout << vec_rank.rank(i) << endl;
        //vec_select.select(i);
        vec_select.m_v->bv_select.select(i);
    }
    end = clock();
    cout << "select: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<10000;i++){
        //cout << i << endl;
        vec_select2.select(i);
    }
    end = clock();
    cout << "select extended: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    cout << "20000" << endl;

    start = clock();
    for(int i = 0;i<20000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_select_rrr.select(i);
    }
    end = clock();
    cout << "select rrr: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<20000;i++){
        //cout << vec_rank.rank(i) << endl;
        //vec_select.select(i);
        vec_select.m_v->bv_select.select(i);
    }
    end = clock();
    cout << "select: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<20000;i++){
        //cout << i << endl;
        vec_select2.select(i);
    }
    end = clock();
    cout << "select extended: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    cout << "40000" << endl;

    start = clock();
    for(int i = 0;i<80000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_select_rrr.select(i);
    }
    end = clock();
    cout << "select rrr: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<40000;i++){
        //cout << vec_rank.rank(i) << endl;
        //vec_select.select(i);
        vec_select.m_v->bv_select.select(i);
    }
    end = clock();
    cout << "select: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<40000;i++){
        //cout << i << endl;
        vec_select2.select(i);
    }
    end = clock();
    cout << "select extended: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    cout << "80000" << endl;

    start = clock();
    for(int i = 0;i<80000;i++){
        //cout << vec_rank.rank(i) << endl;
        vec_select_rrr.select(i);
    }
    end = clock();
    cout << "select rrr: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<80000;i++){
        //cout << vec_rank.rank(i) << endl;
        //vec_select.select(i);
        vec_select.m_v->bv_select.select(i);
    }
    end = clock();
    cout << "select: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<80000;i++){
        //cout << i << endl;
        vec_select2.select(i);
    }
    end = clock();
    cout << "select extended: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
*/
    /////////////////////////////////////////////////////////////////////////////////

/*
    start = clock();
    for(int i = 0;i<10000;i++){
        vec_rank.m_v->bv_rank.rank(i);

    }
    end = clock();
    cout << "rank 10000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<10000;i++){
        vec_rank_rrr.rank(i);

    }
    end = clock();
    cout << "rank rrr 10000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<10000;i++){
        vec_rank2.rank(i);

    }
    end = clock();
    cout << "rank extended 10000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<20000;i++){
        vec_rank.m_v->bv_rank.rank(i);

    }
    end = clock();
    cout << "rank 20000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<20000;i++){
        vec_rank_rrr.rank(i);

    }
    end = clock();
    cout << "rank rrr 20000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<20000;i++){
        vec_rank2.rank(i);

    }
    end = clock();
    cout << "rank extended 20000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<40000;i++){
        vec_rank.m_v->bv_rank.rank(i);

    }
    end = clock();
    cout << "rank 40000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<40000;i++){
        vec_rank_rrr.rank(i);

    }
    end = clock();
    cout << "rank rrr 40000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<40000;i++){
        vec_rank2.rank(i);

    }
    end = clock();
    cout << "rank extended 40000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<80000;i++){
        vec_rank.m_v->bv_rank.rank(i);

    }
    end = clock();
    cout << "rank 80000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<80000;i++){
        vec_rank_rrr.rank(i);

    }
    end = clock();
    cout << "rank rrr 80000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;
    start = clock();
    for(int i = 0;i<80000;i++){
        vec_rank2.rank(i);

    }
    end = clock();
    cout << "rank extended 80000: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;

    start = clock();
    for(int i = 0;i<100;i++){
        vec_rank2.rank(i);

    }
    end = clock();
    cout << "rank extended 100: " << (double)(end-start)/CLOCKS_PER_SEC*1000 << "ms" << endl;



*/
    return 0;
}
