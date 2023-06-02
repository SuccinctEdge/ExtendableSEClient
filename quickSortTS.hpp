//
// Created by weiqin xu on 28/08/2019.
//

#ifndef TRIPLE_STORE_QUICKSORTTS_HPP
#define TRIPLE_STORE_QUICKSORTTS_HPP

#include "RankBaseElement.hpp"
#include <vector>

template <typename T>
unsigned long partition(std::vector<T> &v, unsigned long begin, unsigned long end)
{
    unsigned long m = begin + (end - begin)/2;
    T pivot = v[m];
    v[m] = v[begin];
    v[begin] = pivot;
    unsigned long left = begin + 1;
    unsigned long right = end;
    while(true){
        while(left < right && v[right] >= pivot)
            right--;
        while(left < right && v[left] < pivot)
            left++;
        if(left == right)
            break;
        T tmp = v[left];
        v[left] = v[right];
        v[right] = tmp;
    }
    if(v[left] >= pivot)
        return begin;
    v[begin] = v[left];
    v[left] = pivot;
    return left;
}


template <typename T>
void quickSortTS(std::vector<T> &v, unsigned long begin, unsigned long end)
{
    if(begin >= end)
        return;
    unsigned long boundary = partition(v, begin, end);
    if(boundary > begin){
        quickSortTS<T>(v, begin, boundary - 1);
    }
    if(boundary < end){
        quickSortTS<T>(v, boundary + 1, end);
    }

}


#endif //TRIPLE_STORE_QUICKSORTTS_HPP
