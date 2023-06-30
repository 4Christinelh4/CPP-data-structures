#ifndef _DEQUE_H_
#define _DEQUE_H_

#include <iostream>
#include <exception>
#include <iterator>
#include <vector>
#include <stdio.h>

template <typename _Tp, size_t chunk_size, size_t chunk_number, typename _Alloc = std::allocator<_Tp>>
class Deque {

    private:
        typedef _Tp             value_type;
        typedef _Tp&            reference;
        typedef _Tp*            pointer;

        using _chunk = std::vector<value_type>;

        typename _Alloc::template rebind<_chunk>::other chunk_alloc;
        std::vector<_chunk*> __map;

        /*
            buff_size is the length of the chunk
            */
        struct __deque_iterator {
            using _Self = __deque_iterator;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = _Tp;
            using pointer = _Tp*;
            using reference = _Tp;

            // pointer to the chunk
            pointer cur;       
            pointer first;     // the begin of the chunk
            pointer last;      // the end of the chunk

            //because the pointer may skip to other chunk
            _chunk **node;    // pointer to the map

            void set_node(_chunk ** __x) {
                node = __x; // *node:  addr of the vector
                first = & ((*node)->at(0)) ;
                last = first + chunk_size;

                // printf("node = %p, first = %p, last = %p\n", node, first, last);
            }

            void set_value(const value_type& val ) 
            {
                *cur = val;
            }

            reference &operator*() const _GLIBCXX_NOEXCEPT
            { 
                return *cur;
            }

            _Self& operator++() _GLIBCXX_NOEXCEPT
            {
                ++cur;
                if (cur == last) {      //if it reach the end of the chunk
                    // std::cout << "cur == last\n";
                    set_node(node + 1);//skip to the next chunk
                    cur = first;
                }

                return *this;
            }

            _Self operator++(int) _GLIBCXX_NOEXCEPT
            {
                _Self __tmp = *this;
                ++*this;//invoke prefix ++
                return __tmp;
            }

            _Self& operator--(){
                if (cur == first){      // if it pointer to the begin of the chunk
                    set_node(node - 1);//skip to the prev chunk
                    cur = last;
                }

                --cur;
                return *this;
            }

            _Self operator--(int)
            {
                _Self tmp = *this;
                --*this;
                return tmp;
            }

            // skip steps, for random access ([] or at)
            _Self& operator+=(difference_type n) { 
                difference_type offset = n + (cur - first); // n = 2, cur - first = 5
                if (offset >=0 && offset < chunk_size){
                    // in the same chunk
                    cur += n;
                } else {//not in the same chunk
                    difference_type chunk_offset;
                    
                    if (offset >= chunk_size) {
                        
                        chunk_offset = offset / chunk_size;
                        
                        // skip to the new chunk
                        set_node(node + chunk_offset); // node: 0x50 (a vector's address )
                        // set new cur
                        cur = first + (offset - chunk_offset * chunk_size); // offset = 14, node_offset = 1                            
                    } else {
                        // offset < 0 for n<0
                        offset = first-cur-n;
                        
                        chunk_offset = 1 + (offset / chunk_size);
                        set_node(node - chunk_offset);

                        cur = first + (chunk_offset * chunk_size - offset );

                    }
                }

                return *this;
            }

            // skip n steps
            _Self & operator+(difference_type n)const{
                _Self tmp = *this;
                return tmp+= n; //reuse  operator +=
            }


            // skip steps, for random access ([] or at)
            _Self& operator-=(difference_type n) {
                (*this) += (-n);
                return *this;
            }

            // skip n steps
            _Self & operator-(difference_type n)const{
                _Self tmp = *this;
                return tmp-= n; //reuse  operator +=
            }

            // random access (iterator can skip n steps)
            // invoke operator + ,operator *
            reference &operator[](difference_type n)const{
                return *(*this + n);
            }

            friend bool
            operator==(const _Self& __x, const _Self& __y) _GLIBCXX_NOEXCEPT
            { return __x.cur == __y.cur; }

            friend bool
            operator!=(const _Self& __x, const _Self& __y) _GLIBCXX_NOEXCEPT
            { return __x.cur != __y.cur; }

        };

        _Alloc _M_alloc;
        size_t  sz;
    public:
        using iterator = __deque_iterator;

        //data members
        iterator start;
        iterator finish;

        size_t current_chunk_num;

    public:
        
        /*
            * init chunk_number vectors of chunk_size
            */
        Deque():sz(0) {
            __map.reserve(8 * chunk_number); // to reduce reallocation
            current_chunk_num = chunk_number;
            _create_map_and_nodes();
            std::cout << "init\n";
        }

        ~Deque() {
            std::cout << "release\n";
            for (size_t i = 0; i < __map.size() ; ++i) {
                delete __map[i];
            }
            std::cout << "finish release\n";
        }

        size_t size() const {
            return sz;
        }

        void push_back (const value_type& val) {
            // check if resize is needed
            // chunk num = 8 = chunk size
            __check_and_resize();
            finish.set_value(val);
            ++finish;
            ++sz;
        }

        void push_front (const value_type& val) {
            // check if resize is needed 
            // printf("start.cur = %p, start.first = %p, start.cur-1 = %p, start.node = %p, __map[0] = %p\n"
            // , start.cur, start.first, start.cur -1, start.node, &__map[0]);
            
            __check_and_resize();

            // if ( (start.node == &__map[0]) &&  (start.cur - 1 == start.first) ) {
            //     start.cur = start.first;
            //     *(start.cur) = val;
            //     // printf("set %p ", start.cur);
            //     // std::cout << "value " << val << std::endl;
            //     return;
            // }

            --start;
            start.set_value(val); // 48
            
            ++sz;
        }

        // remove the last element
        void pop_back() {
            if (sz > 0) {
                --finish;
                --sz;
            }
        }

        void pop_front() {
            if (sz > 0) {
                ++start;
                --sz;
            }
        }

        size_t capacity() {
            return chunk_size * current_chunk_num;
        }

        /*
            * move all values in front or after, depending on how close "position" is to
            */
        iterator erase (iterator position) {
            if (position == finish || sz == 0) {
                return finish;
            }

            --sz;
            
            if (position == start) {
                start ++;
                return position++;
            }

            size_t __dist_first = position.cur - start.cur;
            iterator cursor;
            
            if (2*__dist_first <= sz) {
                // closer with the first, start changed, finish doesn't change 

                for (cursor = position-1; cursor != start; --cursor) {
                    cursor.set_value(*(cursor-1));
                }

                start++;
                return position++;
            
            } else {
                // start doesn't change, finish changed
                for (cursor = position; cursor != finish-1; ++cursor) {
                    cursor.set_value(*(cursor+1));
                }

                finish--;

                // finish-2 finish-1 finish
                return position;
            }
        }

        /* 
            * the caller needs to make sure that n < sz, otherwise there might be seg fault
            * chunk = 8, n = 15 -> chunk 1
            * n = 16: chunk 2
            */
        value_type & operator[](size_t n) const {
            // find the chunk from the start
            return start[n];
        }

        iterator begin() {
            return start;
        }

        iterator end() {
            return finish;
        }

        reference front(){
            //invoke __deque_iterator operator*
            // return start's member *cur
            return *start;
        }

        reference back(){
            // cna't use *finish
            iterator tmp = finish;
            --tmp; 
            return *tmp; //return finish's  *cur
        }

        // resize to let the capacity = __n, __n is num of elements
        // void resize(size_t __n) {

        // }

    private:

        bool __check_boundary() {
            // start is the first block and start.cur is at the beginning
            
            if (start.cur == start.first && start.node == &__map[0]) {
                return true;
            }

            // finish is the last block and finish.cur is at the end
            if (finish.cur == finish.last - 1 && finish.node == & __map[current_chunk_num-1]) {
                return true;
            }

            return false;
        }

        // resize to current capacity * 2
        // only change chunk number, not change chunk_size
        void __check_and_resize() {
            if (__check_boundary()) {
                __do_resize();
            }
        }


        /*
            * put the vectors storing data at the center
            * 
            */
        void __do_resize() {
            
            _chunk * chunk_addr;
            size_t i = 0;

            // save the index of current start node
            size_t start_node_index = start.node - &__map[0] ;
            std::ptrdiff_t cur_start = start.cur - start.first;

            // and current end node
            size_t finish_node_index = finish.node - &__map[0];
            std::ptrdiff_t cur_last = finish.cur - finish.first;
            
            for (i = 0; i < current_chunk_num; ++i) {
                chunk_addr = new _chunk(chunk_size);
                __map.push_back(chunk_addr);                    
            }

            // calculate dist at left and right
            current_chunk_num = __map.size();
            size_t __empty = current_chunk_num - (1+finish.node - start.node);
            std::cout << "__empty " << __empty << std::endl;
            size_t _dist = __empty >> 1; // how many elements should move the data to its right
            
            if (start_node_index != 0) {
                // full at the end
                _dist -= start_node_index;
            }


            /*
                |x1|x2|x3|x4|y1|y2|y3|y4|
                -> |y1|y2|x1|x2|x3|x4|y3|y4|
                */

            /*

                |E0|x1|x2|x3|y1|y2|y3|y4|
                -> |y1|y2|E0|x1|x2|x3|y3|y4|
                */
            
            // std::cout << "finish node = " << finish_node_index << " start node = " \
            // << start_node_index << std::endl;

            size_t j = finish_node_index;
            
            for (; j - start_node_index >= 0; --j) {
                // std::cout << "j = " << j << std::endl;
                __swap(j+_dist, j);
                if (j == 0) {
                    break;
                }
            }
            
            // reset start, start.cur, finish, finish.cur
            // printf("start_index = %ld, finish_index = %ld\n", start_node_index, finish_node_index);

            start.set_node(&__map[_dist + start_node_index]);
            start.cur = start.first + cur_start;

            finish.set_node(&__map[_dist + finish_node_index]);
            finish.cur = finish.first + cur_last;
        }


        void __swap(size_t x1, size_t x2) {
            _chunk * tmp = __map[x1];
            __map[x1] = __map[x2];
            __map[x2] = tmp;
        }
        
        void _create_map_and_nodes(){
            // allocate map array
            // the map stores the first element's address of eacch chunk (vector)
            _chunk * chunk_addr;
            for (size_t i = 0; i < chunk_number; ++i) {
                chunk_addr = new _chunk(chunk_size);
                __map.push_back(chunk_addr);
            }

            // tmp_start，tmp_finish poniters to the center range of map
            _chunk ** tmp_start  = & __map[chunk_number / 2]; 
            _chunk ** tmp_finish = tmp_start;

            // set start and end iterator
            start.set_node(tmp_start);
            start.cur = start.first;

            finish.set_node(tmp_finish);
            finish.cur = finish.first;
        }

};

#endif