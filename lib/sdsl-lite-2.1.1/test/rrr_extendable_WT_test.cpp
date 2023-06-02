#include "sdsl/int_vector.hpp"
#include "sdsl/wt_extendable.hpp"
#include "sdsl/construct.hpp"
#include "sdsl/util.hpp"
#include "gtest/gtest.h"
#include <vector>
#include <string>

int main(int argc, char** argv){
    //auto *vec = new rrr_vector_extendable<5, int_vector<>, 3>(init);
    wt_extendable<> wt;
    construct_im(wt, "0 1 0 1 1 0 1 0 0 2 0 2 0 1 0 0 1 1 0 2 0 2 1 0 2 1 1 0 2 2 2 0 2 1 0 2", 'd');

    for(int i = 1; i <= wt.size();i++ ) {
        cout << i << " : " << wt.select(i,2) << endl;
    }
    /*
    int_vector<> ex({2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33});
    wt.extend(ex);
    cout << "INSERT EN COURS" << endl;


    for(int i = 1; i <= wt.size();i++ ) {
        cout << i << " : " << wt.select(i,2) << endl;
    }
    */
    return 0;
}
