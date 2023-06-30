#ifndef __QUICK_SORT_H__
#define __QUICK_SORT_H__

#include <vector>
#include <queue>
#include <stdio.h>

template <typename _Tp>
class QuickSort {

    public:
        using size_type = __int64_t;

        std::vector<_Tp> *v;

        QuickSort(){ }

        ~QuickSort() { }

        void swap(_Tp *i, _Tp *j) {
            _Tp tmp = *i;
            *i = *j;
            *j = tmp;
        }

        size_type  partition(size_type low, size_type high /* std::vector<_Tp> &v*/) {
            
            _Tp pivot = v->at(high);
            // printf("high = %ld\n", high);
            size_type  left_end = low; // index of final pivot
            for (size_type  i = low; i < high; ++i) {
                
                if (pivot > v->at(i)) {
                    //cprintf("swap i = %ld, left_end = %ld\n", i, left_end);
                    swap( &v->at(left_end),  &v->at(i));
                    ++left_end;
                }
                
            }
            // swap left_end and pivot
            // printf("swap left %ld and high %ld\n", left_end, high);
            swap(&v->at(left_end), & v->at(high));
            return left_end;
        }
        // v[pivot] > v[2], left_end = 0
        // v[5] = 3, v[2] = 1 
        // 1, 6, 5, 2, 4, 3 left_end = 1, 
        // 1, 2, 5, 6, 4, 3, left_end = 2
       
        // 1, 2, 3, 6, 4, 5, left_end = 2
        void quickSort(size_type start, size_type end /*, std::vector<_Tp> &v*/) {
            if (start >= end) {
                return;
            }

            size_type left_end = partition(start, end);
            // printf("left_end = %ld\n", left_end);

            // left_end: index of the last element at the left half
            quickSort(start, left_end-1);
            quickSort(left_end+1, end);
        }

        // all elements left to prev_last is smaller than prev_last
        void __prepare_quickSort (size_type prev_last /*, std::vector<_Tp> &v*/) {
            size_type sz = v->size();

            swap(&v->at(prev_last-1), &v->at(sz-1));
            partition(prev_last-1, sz-1);
        }
};

/*

1 3 2 4 5

min_heap 
max_heap

arr[0] arr[1]

1 -> max_heap
3 -> min_heap

2 > 1 2 < 3
1, 2 -> max_heap
3 -> min_heap

4 > 3

1, 2 -> max_heap
3, 4 -> min_heap

2+3 /2

5 > 3

1, 2
3, 4, 5

-> rebalance
1, 2, 3

4, 5

while (min_heap.size() > max_heap.size()) {
    int k = min_heap.top();
    min_heap.pop();
    max_heap.push(k);
}

 */


#endif