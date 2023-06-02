/* sdsl - succinct data structures library
    Copyright (C) 2013 Simon Gog

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*! \file wt_pc.hpp
    \brief wt_pc.hpp contains a class for the wavelet tree of byte sequences.
           The wavelet tree shape is parametrized by a prefix code.
    \author Simon Gog, Timo Beller
*/
#ifndef INCLUDED_SDSL_WT_EX
#define INCLUDED_SDSL_WT_EX

#include "rrr_vector_extendable.hpp"
#include "bit_vectors.hpp"
#include "rank_support.hpp"
#include "select_support.hpp"
#include "wt_helper.hpp"
#include "iterators.hpp"
#include "wt_pc.hpp"
#include "wt_blcd.hpp"
#include <vector>
#include <utility>
#include <tuple>

//! Namespace for the succinct data structure library.

namespace sdsl
{

    template<int t_bs=63, class t_rac=int_vector<>, int t_sbs=5>
    class extendable_node{

        typedef typename rrr_vector_extendable<t_bs, t_rac, t_sbs>::rank_1_type rank_1_type;
        typedef typename rrr_vector_extendable<t_bs, t_rac, t_sbs>::select_1_type select_1_type;
        typedef typename rrr_vector_extendable<t_bs, t_rac, t_sbs>::select_0_type select_0_type;

        // pointer to the parent
        // pointer to the left child
        // pointer to the right child
        rank_1_type   rank_sup;
        select_1_type select1_sup;
        select_0_type select0_sup;


    public:
        explicit extendable_node(bit_vector &bv, extendable_node * creator){
            vec = new rrr_vector_extendable<t_bs,int_vector<>,t_sbs>(bv);
            rank_sup.set_vector(vec);
            select1_sup.set_vector(vec);
            select0_sup.set_vector(vec);
            parent = creator;
        }

        explicit extendable_node(bit_vector &bv){
            vec = new rrr_vector_extendable<t_bs,int_vector<>,t_sbs>(bv);
            rank_sup.set_vector(vec);
            select1_sup.set_vector(vec);
            select0_sup.set_vector(vec);
        }

        int rank(int i){
            return rank_sup.rank(i);
        }

        int select(int i, int element){
            if(element == 0)
                return select0_sup.select(i);
            else
                return select1_sup.select(i);
        }

        void set_left(extendable_node<t_bs,t_rac,t_sbs> *pNode) {
            left = pNode;
        }
        void set_right(extendable_node<t_bs,t_rac,t_sbs> *pNode) {
            right = pNode;
        }

        rrr_vector_extendable<t_bs,t_rac,t_sbs> * vec;
        extendable_node<t_bs,t_rac,t_sbs> * right= nullptr;
        extendable_node<t_bs,t_rac,t_sbs> * left = nullptr;
        extendable_node<t_bs,t_rac,t_sbs> * parent = nullptr;
    };


    template<int t_bs=63, class t_rac=int_vector<>, int t_sbs=5>
    class building_node{
    private:
        int size=0;
    public:
        bit_vector * bv;
        building_node * right= nullptr;
        building_node * left = nullptr;
        building_node * parent;

        building_node(uint64_t size, building_node * creator){
            bv = new bit_vector(size);
            parent = creator;
        }

        void add(bool i, int times){
            //cout << "adding " << i << " " << times << endl;
            for(int k=0; k < times; k++){
                //bv->set_int(size++,i);
                bv->operator[](size++) = i;
                //cout << k << endl;
            }
        }

        extendable_node<t_bs,t_rac,t_sbs> * construct(extendable_node<t_bs,t_rac,t_sbs> * creator){
            bv->resize(size);
            auto * node = new extendable_node<t_bs,t_rac,t_sbs>(*bv);
            if(left != nullptr)
                node->left = left->construct(node);
            if(right != nullptr)
                node->right = right->construct(node);
            node->parent = creator;

            if(left != nullptr)
                delete(left);
            if(right != nullptr)
                delete(right);
            delete(bv);
            return node;
        }
    };


//! A prefix code-shaped wavelet.
/*!
 * \tparam t_shape       Shape of the tree ().
 * \tparam t_bitvector   Underlying bitvector structure.
 * \tparam t_rank        Rank support for pattern `1` on the bitvector.
 * \tparam t_select      Select support for pattern `1` on the bitvector.
 * \tparam t_select_zero Select support for pattern `0` on the bitvector.
 * \tparam t_tree_strat  Tree strategy determines alphabet and the tree
 *                       class used to navigate the WT.
 *
 *  @ingroup wt
 */
    template<class t_shape = balanced_shape,
            int t_bs=63, class t_rac=int_vector<>, int t_k=5,
            class t_bitvector   = rrr_vector_extendable<t_bs,t_rac,t_k>,
            class t_rank        = typename t_bitvector::rank_1_type,
            class t_select      = typename t_bitvector::select_1_type,
            class t_select_zero = typename t_bitvector::select_0_type,
            class t_tree_strat  = int_tree<>
    >
    class wt_extendable
    {
    public:
        typedef typename
        t_tree_strat::template type<wt_extendable>            tree_strat_type;
        typedef int_vector<>::size_type               size_type;
        typedef typename
        tree_strat_type::value_type                   value_type;
        typedef typename t_bitvector::difference_type difference_type;
        typedef t_bitvector                           bit_vector_type;
        typedef t_rank                                rank_1_type;
        typedef t_select                              select_1_type;
        typedef t_select_zero                         select_0_type;
        typedef wt_tag                                index_category;
        typedef typename
        tree_strat_type::alphabet_category            alphabet_category;
        typedef typename
        t_shape::template type<wt_extendable>                 shape_type;
        enum { lex_ordered=shape_type::lex_ordered };
        using node_type = typename tree_strat_type::node_type;
        int code_size = 1;

    private:

#ifdef WT_PC_CACHE
        mutable value_type m_last_access_answer;
        mutable size_type  m_last_access_i;
        mutable size_type  m_last_access_rl;
#endif

        size_type        m_size  = 0;    // original text size
        bit_vector_type *m_bv = nullptr;           // bit vector to store the wavelet tree
        tree_strat_type  m_tree;

        map<int,uint64_t> dict;
        map<uint64_t,int> reverse_dict;
        extendable_node<t_bs,t_rac,t_k> * root_node;

        extendable_node<t_bs,t_rac,t_k> * recursive_build(int_vector_buffer<tree_strat_type::int_width>& content,int k, set<int> * set_to_build, extendable_node<t_bs,t_rac,t_k> * parent){
            bit_vector bv(content.size());
            int i = 0;
            for(auto elem : content){
                if(set_to_build->find(elem) != set_to_build->end()){
                    bv[i++] = decode(elem,k);
                }
            }
            bv.resize(i);

            auto * node  = new extendable_node<t_bs,t_rac,t_k>(bv,parent);
            //node->vec->display();
            if(k > 0){
                set<int> left;
                set<int> right;
                for(auto elem:*set_to_build){
                    if(decode(elem,k))
                        right.emplace(elem);
                    else
                        left.emplace(elem);
                }
                if(!left.empty())
                    node->set_left(recursive_build(content,k-1,&left,node));
                if(!right.empty())
                    node->set_right(recursive_build(content,k-1,&right,node));
            }
            return node;
        }

    public:

        //const size_type&       sigma = m_sigma;
        //const bit_vector_type bv  = m_bv;

        // Default constructor
        wt_extendable() {};

        //! Construct the wavelet tree from a file_buffer
        /*!
         * \param input_buf    File buffer of the input.
         * \param size         The length of the prefix.
         * \par Time complexity
         *      \f$ \Order{n\log|\Sigma|}\f$, where \f$n=size\f$
         */
        wt_extendable(int_vector_buffer<tree_strat_type::int_width>& input_buf,size_type size):m_size(size){
            if (0 == m_size)
                return;

            double start = clock();
            //Build the dictionnary for elements found in input_buf
            uint64_t code = 0;
            set<int> inital_set;
            for(auto i:input_buf) {
                // i not in dictionnary
                if(dict.find(i) == dict.end()){
                    //cout << "char " << i << " encoded by " << code << endl;
                    //cout << "in " << bits::hi(code)+1 << " bits " << endl;
                    code_size = bits::hi(code);
                    //cout << "SIZE: " << code_size << ", starter : " << code << endl;
                    dict[i] = code;
                    reverse_dict[code] = i;
                    code += 1;
                    inital_set.emplace(i);
                }
            }
            //double end = clock();
            //cout << "dict time: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << " ms" << endl << endl;
            // Builds the tree recursively from the root

            /*
            // Recursive build
            auto start = clock();
            root_node = recursive_build(input_buf,code_size, &inital_set, nullptr);
            cout << "  Wavelet tree build time : " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
            cout << " Contains " << root_node->vec->size() << " elements" << endl;
            */
            //double cumul_dec = 0;
            //start = clock();
            // Linear build
            int last = input_buf[0];
            int i = 0;
            int times = 0;
            building_node<t_bs,t_rac,t_k> * base = new building_node<t_bs,t_rac,t_k>(input_buf.size(),nullptr);
            building_node<t_bs,t_rac,t_k> * n = base;
            while(i < input_buf.size()){
                //cout << i << " " << code_size << endl;
                if(input_buf[i] == last)
                    times += 1;
                else{
                    for(int k = code_size; k >= 0; k--){

                        double start_dec = clock();
                        bool val = decode(last,k);
                        double end_dec = clock();
                        //cumul_dec += end_dec - start_dec;
                        n->add(val,times);
                        //cout << val << endl;
                        if(k>0) {
                            if (val) {
                                if(n->right == nullptr)
                                    n->right = new building_node<t_bs,t_rac,t_k>(input_buf.size(),n);
                                n = n->right;
                            }
                            else {
                                if(n->left == nullptr)
                                    n->left = new building_node<t_bs,t_rac,t_k>(input_buf.size(),n);
                                n = n->left;
                            }
                        }
                    }
                    n = base;
                    last = input_buf[i];
                    times = 1;
                }
                i+=1;
            }
            //cout << "decoding time: " << (double)(cumul_dec)/CLOCKS_PER_SEC*1000 << " ms" << endl << endl;

            // Writes last bits
            for(int k = code_size; k >= 0; k--){
                bool val = decode(last,k);
                n->add(val,times);
                if(k>0) {
                    if (val) {
                        if(n->right == nullptr)
                            n->right = new building_node<t_bs,t_rac,t_k>(input_buf.size(),n);
                        n = n->right;
                    }
                    else {
                        if(n->left == nullptr)
                            n->left = new building_node<t_bs,t_rac,t_k>(input_buf.size(),n);
                        n = n->left;
                    }
                }
            }
            //end = clock();
            //cout << "linear run time: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << " ms" << endl << endl;


            //start = clock();
            root_node = base->construct(nullptr);
            delete(base);
            //end = clock();
            //cout << "construct time: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << " ms" << endl << endl;
        }

        void extend(int_vector<>& input_buf){
            int code = dict.size();
            for(auto i : input_buf){
                if(dict.find(i) == dict.end()){
                    dict[i] = code;
                    reverse_dict[code] = i;
                    if(bits::hi(code) == code_size) {
                        int k = code_size;
                        auto n = root();
                        //cout << "smallgly" << i << endl;
                        while(k>0){
                            //cout << k << endl;
                            if(decode(i,k)){
                                n->vec->extend({1});
                                if(n->right == nullptr){
                                    extendable_node<t_bs,t_rac,t_k> * node;
                                    bit_vector bv(0);
                                    n->set_right(new extendable_node<t_bs,t_rac,t_k>(bv,n));
                                }
                                n = n->right;
                            }else{
                                n->vec->extend({0});
                                if(n->left == nullptr){
                                    extendable_node<t_bs,t_rac,t_k> * node;
                                    bit_vector bv(0);
                                    n->set_left(new extendable_node<t_bs,t_rac,t_k>(bv,n));
                                }
                                n = n->left;
                            }
                            k-=1;
                        }
                        if(n != nullptr){
                            if(decode(i,k)){
                                n->vec->extend({1});
                            }else{
                                n->vec->extend({0});
                            }
                        }
                    }
                    else{
                        //cout << "bigly" << i << endl;
                        code_size = bits::hi(code);
                        bit_vector bv(size());
                        //previous bits are all 0
                        for(int i = 0; i < bv.size();i++){
                            bv[i] = 0;
                        }
                        auto * new_root = new extendable_node<t_bs,t_rac,t_k>(bv,nullptr);
                        root()->parent = new_root;
                        new_root->left = root();
                        root_node = new_root;
                        // We now create nodes until we reach the end of the code
                        int k = code_size;
                        auto n = root();
                        bv.resize(1);
                        while(k>0){
                            if(decode(i,k)){
                                //In this case the only existing vector is the first one
                                n->vec->extend({1});
                                n->set_right(new extendable_node<t_bs,t_rac,t_k>(bv,n));
                                n = n->right;
                            }else{
                                n->set_left(new extendable_node<t_bs,t_rac,t_k>(bv,n));
                                n = n->left;
                            }
                            k-=1;
                        }
                    }
                    code += 1;
                }
                else{
                    int k = code_size;
                    auto n = root();
                    while(k>=0){
                        if(decode(i,k)){
                            n->vec->extend({1});
                            n = n->right;
                        }else{
                            n->vec->extend({0});
                            n = n->left;
                        }
                        k-=1;
                    }
                }
            }
        }

        //Returns the k-th bit from the right encoding the element in the dictionnary
        bool decode(int element, int k){
            //assert(dict.find(element) != dict.end());
            //cout << "decoding " << element << " " << k << endl;
            return (dict[element] & ( 1 << k )) >> k;
        }


        //! Returns the size of the original vector.
        size_type size()const { return root()->vec->size(); }

        //! Returns whether the wavelet tree contains no data.
        bool empty()const { return m_size == 0; }

        size_type serialize(std::ostream& out, structure_tree_node* v=nullptr, std::string name="")const {
            // TODO Not implemented
            return 0;
        }

        //! range_search_2d searches points in the index interval [lb..rb] and value interval [vlb..vrb].
        /*! \param lb     Left bound of index interval (inclusive)
         *  \param rb     Right bound of index interval (inclusive)
         *  \param vlb    Left bound of value interval (inclusive)
         *  \param vrb    Right bound of value interval (inclusive)
         *  \param report Should the matching points be returned?
         *  \return Pair (#of found points, vector of points), the vector is empty when
         *          report = false.
         */
        std::pair<size_type, std::vector<std::pair<value_type, size_type>>>
        range_search_2d(size_type lb, size_type rb, value_type vlb, value_type vrb, bool report=true)  {
            auto lower = rank(lb,vlb);
            auto upper = rank(rb+1,vlb);
            vector<pair<value_type,size_type>> vec;
            if(lower == upper)
                return make_pair(0,vec);
            else{
                for(int i = lower; i < upper; i++){
                    vec.push_back(make_pair(select(i+1,vlb),vlb));
                }
                return make_pair(lower-upper,vec);
            }
            //TODO Make sure this does what it's suppposed to do
        }

        //! Recovers the i-th symbol of the original vector.
        /*!
         * \param i Index in the original vector.
         * \return The i-th symbol of the original vector.
         * \par Time complexity
         *      \f$ \Order{H_0} \f$ on average, where \f$ H_0 \f$ is the
         *      zero order entropy of the sequence
         *
         * \par Precondition
         *      \f$ i < size() \f$
         */
        int operator[](size_type i){
            assert(i < size());
            int k = code_size;
            uint64_t code = 0;
            extendable_node<t_bs,t_rac,t_k> * n = root();
            //cout << "val : " << n->vec->at(i) << ", " << i << ", " << n->rank(i) <<endl;
            while(k >= 0){
                rrr_vector_extendable<t_bs,t_rac,t_k> vec = * n->vec;
                if(vec.at(i)) {
                    code += pow(2, k);
                    i = n->rank(i);
                    n = n->right;
                }
                else{
                    //cout << "k : " << k << ", " << code << endl;
                    i = i - n->rank(i);
                    n = n->left;
                }
                k-=1;
            }
            //cout << code << ", " << reverse_dict[code] << endl;
            return reverse_dict[code];
        };

        //! Calculates how many symbols c are in the prefix [0..i-1].
        /*!
         * \param i Exclusive right bound of the range.
         * \param element Symbol element.
         * \return Number of occurrences of symbol c in the prefix [0..i-1].
         * \par Time complexity
         *      \f$ \Order{H_0} \f$ on average, where \f$ H_0 \f$ is the
         *      zero order entropy of the sequence
         *
         * \par Precondition
         *      \f$ i \leq size() \f$
         */
        size_type rank(size_type i, int element){
            assert(i <= size());
            /*for(auto ele : dict){
                cout << ele.first <<" "<<ele.second << endl;
            }*/
            if(dict.find(element) == dict.end())
                return size();
            int k = code_size;
            auto n = root();
            while(k>=0){
                if(decode(element,k)){
                    i = n->rank(i);
                    n = n->right;
                }else{
                    i = i - n->rank(i);
                    n = n->left;
                }
                k-=1;
            }
            return i;
        };

        //! Calculates how many times symbol wt[i] occurs in the prefix [0..i-1].
        /*!
         * \param i The index of the symbol.
         * \return  Pair (rank(wt[i],i),wt[i])
         * \par Time complexity
         *      \f$ \Order{H_0} \f$
         *
         * \par Precondition
         *      \f$ i < size() \f$
         */
        std::pair<size_type, value_type>
        inverse_select(size_type i)const{
            //not implemented
            cout << "Should not be used" << endl;
            return nullptr;
            /*assert(i < size());
            node_type v = m_tree.root();
            while (!m_tree.is_leaf(v)) {   // while not a leaf
                if (m_bv[m_tree.bv_pos(v) + i]) {   //  goto right child
                    i = (m_bv_rank(m_tree.bv_pos(v) + i)
                         - m_tree.bv_pos_rank(v));
                    v = m_tree.child(v, 1);
                } else { // goto left child
                    i -= (m_bv_rank(m_tree.bv_pos(v) + i)
                          - m_tree.bv_pos_rank(v));
                    v = m_tree.child(v,0);
                }
            }
            // if v is a leaf bv_pos_rank returns symbol itself
            return std::make_pair(i, (value_type)m_tree.bv_pos_rank(v));
             */
        }

        //! Calculates the ith occurrence of the symbol c in the supported vector.
        /*!
         * \param i The ith occurrence.
         * \param element The symbol element.
         * \par Time complexity
         *      \f$ \Order{H_0} \f$ on average, where \f$ H_0 \f$ is the zero order
         *       entropy of the sequence
         *
         * \par Precondition
         *      \f$ 1 \leq i \leq rank(size(), c) \f$
         */
        size_type select(size_type i, value_type element){
            if(i == 0)
                return 0;
            if(dict.find(element) == dict.end()) {
                cout << "unknown element " << element << endl;
                return size();
            }
            int k = code_size;
            auto n = root();
            while(k>0){
                if(decode(element,k)){
                    n = n->right;
                }else{
                    n = n->left;
                }
                k-=1;
            }

            // Prevents from selecting more the number of occurences
            if(decode(element,0)) {
                if (i > n->vec->sum_rank) {
                    return -1;
                }
            }else{
                if(i > n->vec->size()-n->vec->sum_rank){
                    return -1;
                }
            }

            while(k<code_size){
                i = n->select(i,decode(element,k)) + 1;
                n = n->parent;
                k+=1;
            }
            return n->select(i,decode(element,k));
        };

        //! Swap operator
        void swap(wt_extendable& wt)
        {
            if (this != &wt) {

                map<int,uint64_t> a = this->dict;
                map<uint64_t,int> b = this->reverse_dict;
                sdsl::extendable_node<t_bs,t_rac,t_k> * c = this->root_node;
                int tmp = this->code_size;

                this->dict = wt.dict;
                this->reverse_dict = wt.reverse_dict;
                this->root_node = wt.root_node;
                this->code_size = wt.code_size;

                wt.dict = a;
                wt.reverse_dict = b;
                wt.root_node = c;
                wt.code_size = tmp;

            }
        }

        //! Indicates if node v is empty
        bool empty(const node_type& v) const
        {
            return size(v)==0;
        }

        //! Return the size of node v
        auto size(const node_type& v) const -> decltype(m_tree.size(v))
        {
            if (is_leaf(v)) {
                if (v == root())
                    return size();
                else {
                    auto parent = m_tree.parent(v);
                    auto rs = expand(parent, {0, size(parent)-1});
                    if (m_tree.child(parent, 0) == v)
                        return std::get<1>(std::get<0>(rs))-std::get<0>((std::get<0>(rs)))+1;
                    else
                        return std::get<1>(std::get<1>(rs))-std::get<0>((std::get<1>(rs)))+1;
                }
            } else {
                return m_tree.size(v);
            }
        }

        //! Returns the root node
        extendable_node<t_bs,t_rac,t_k> * root() const
        {
            return root_node;
        }

    private:

        //! Iterator to the begin of the bitvector of inner node v
        auto begin(const node_type& v) const -> decltype(m_bv->begin() + m_tree.bv_pos(v))
        {
            return m_bv->begin() + m_tree.bv_pos(v);
        }

        //! Iterator to the begin of the bitvector of inner node v
        auto end(const node_type& v) const -> decltype(m_bv->begin() + m_tree.bv_pos(v) + m_tree.size(v))
        {
            return m_bv->begin() + m_tree.bv_pos(v) + m_tree.size(v);
        }
    };

}

#endif
