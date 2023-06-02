#ifndef SDSL_RRR_VECTOR_EX_H
#define SDSL_RRR_VECTOR_EX_H

#include "rrr_vector_modified.hpp"
#include <iostream>


using namespace sdsl;
using namespace std;

// forward declaration needed for friend declaration
template<uint8_t t_b=1, uint16_t t_bs=63, class t_rac=int_vector<>, uint16_t t_k=32>
class rank_support_rrr_ex;

template<uint8_t t_b=1, uint16_t t_bs=63, class t_rac=int_vector<>, uint16_t t_k=32>
class select_support_rrr_ex;

template<uint16_t t_bs=63, class t_rac=int_vector<>, uint16_t t_sbs=32>
class rrr_vector_extendable {
public:
    typedef bit_vector::size_type size_type;
    typedef rrr_helper<t_bs> rrr_helper_type;
    typedef bit_vector::difference_type              difference_type;

    typedef typename rrr_vector_mod<t_bs, t_rac, t_sbs>::rank_1_type base_rank_1_type;
    typedef typename rrr_vector_mod<t_bs, t_rac, t_sbs>::rank_0_type base_rank_0_type;
    typedef typename rrr_vector_mod<t_bs, t_rac, t_sbs>::select_1_type base_select_1_type;
    typedef typename rrr_vector_mod<t_bs, t_rac, t_sbs>::select_0_type base_select_0_type;

    typedef rank_support_rrr_ex<1, t_bs, t_rac, t_sbs>   rank_1_type;
    typedef rank_support_rrr_ex<0, t_bs, t_rac, t_sbs>   rank_0_type;
    typedef select_support_rrr_ex<1, t_bs, t_rac, t_sbs> select_1_type;
    typedef select_support_rrr_ex<0, t_bs, t_rac, t_sbs> select_0_type;

    friend class rank_support_rrr_ex<>;
    friend class select_support_rrr_ex<>;

public: // TODO fix this
    int_vector<> m_rank;
    rrr_vector_mod<t_bs, t_rac, t_sbs> * base_vec;
    map<int, int_vector<>*> superblock_map;
    map<int, size_type> rank_map;
    map<int, bit_vector*> base_map;
    size_type last_block_size = 0;
    int last_super_block_size = t_sbs;
    int superblock_count = 0;
    size_type sum_rank = 0;
    int base_rank;
    size_type o_size;
    size_type last_sb0 = 0;
    size_type last_sb1 = 0;
    bool selectable = true;

    base_rank_1_type bv_rank;
    base_rank_0_type bv_rank0;
    base_select_1_type bv_select;
    base_select_0_type bv_select0;

    rrr_vector_extendable<t_bs, t_rac, t_sbs>* select0_vec;
    rrr_vector_extendable<t_bs, t_rac, t_sbs>* select1_vec;
    map<int,int> select0_map;
    map<int,int> select1_map;

private:
    int_vector<1> copy_from(const int_vector<1> *source, int start , int count, const int_vector<1> *destination, int from){
        int_vector<1> s = *source;
        int_vector<1> d = *destination;
        for(int i = 0; i < count; i++) {
            d[from + i] = s[start + i];
            //cout << d[from + i];
        }
        //cout << endl;
        return d;
    }

public:
    explicit rrr_vector_extendable(const bit_vector &bv) {
        base_vec = new rrr_vector_mod<t_bs, t_rac, t_sbs>(bv);
        o_size = bv.size();
        bv_rank.set_vector(base_vec);
        bv_rank0.set_vector(base_vec);
        bv_select.set_vector(base_vec);
        bv_select0.set_vector(base_vec);
        select0_vec = new rrr_vector_extendable<t_bs, t_rac, t_sbs>(sdsl::int_vector<1>(),false);
        select1_vec = new rrr_vector_extendable<t_bs, t_rac, t_sbs>(sdsl::int_vector<1>(),false);
        sum_rank = bv_rank.rank(base_vec->size());
        base_rank = sum_rank;
        rank_map[0] = sum_rank;
    }
    explicit rrr_vector_extendable(const bit_vector &bv, bool selectability) {
        base_vec = new rrr_vector_mod<t_bs, t_rac, t_sbs>(bv);
        o_size = bv.size();
        bv_rank.set_vector(base_vec);
        bv_rank0.set_vector(base_vec);
        sum_rank = bv_rank.rank(base_vec->size());
        base_rank = sum_rank;
        rank_map[0] = sum_rank;
        selectable = selectability;
        if(selectability) {
            select0_vec = new rrr_vector_extendable<t_bs, t_rac, t_sbs>(sdsl::int_vector<1>(),false);
            select1_vec = new rrr_vector_extendable<t_bs, t_rac, t_sbs>(sdsl::int_vector<1>(),false);
            bv_select.set_vector(base_vec);
            bv_select0.set_vector(base_vec);
            bv_select0.set_vector(base_vec);
        }
    }

    void extend(const bit_vector &bv) {
        int_vector<> bt_array;
        int_vector<1> base_array;
        size_type pos = 0;
        size_type i = last_super_block_size, x;
        size_type base_pos = last_super_block_size * t_bs + last_block_size;
        int_vector<1> set_array;
        int_vector<1> unset_array;
        size_type j;
        size_type set_count = 0;
        size_type unset_count = 0;
        int sb_size = t_bs * t_sbs;
        int adjusted_sb_size = sb_size;
        int extended_size = o_size - base_vec->size();

        if(selectable){
            set_array.width(1);
            set_array.resize(bv.size());
            unset_array.width(1);
            unset_array.resize(bv.size());

            for (j = 0; j < bv.size(); j++) {
                if (bv[j]) {
                    if ((j + extended_size) / sb_size != last_sb1) {
                        //cout << (j + extended_size) << endl;
                        //cout << (j + extended_size) / sb_size << endl;
                        //cout << "last_sb1 " << last_sb1 << endl;
                        set_array[set_count] = true;
                        last_sb1 = (j + extended_size) / sb_size;
                        select1_map[select1_map.size()] = last_sb1;
                    } else {
                        if(last_sb1 == 0)
                            select1_map[0] = 0;
                        set_array[set_count] = false;
                    }
                    set_count += 1;
                } else {
                    if ((j + extended_size) / sb_size != last_sb0) {
                        unset_array[unset_count] = true;
                        last_sb0 = (j + extended_size) / sb_size;
                        select0_map[select0_map.size()] = last_sb0;
                    } else {
                        if(last_sb0 == 0)
                            select0_map[0] = 0;
                        unset_array[unset_count] = false;
                    }
                    unset_count += 1;
                }
            }

            set_array.resize(set_count);
            select1_vec->extend(set_array);


            unset_array.resize(unset_count);
            select0_vec->extend(unset_array);
        }

        // TODO If the last superblock is incomplete we complete it last_block_size != t_bs
        // Get old block through pointer
        // copy it into a t_bs sized bitvector and add remaining bits
        // evaluate the class and replace the old one and pointer
        while (last_super_block_size < t_sbs && pos < bv.size()) {

            bt_array = *superblock_map[superblock_count];
            base_array = *base_map[superblock_count];

            if (last_block_size != 0) {
                // if incomplete completed block is still incomplete if last_block_size +b v_size < t_bs
                if (last_block_size + bv.size() < t_bs) {
                    base_array = copy_from(&bv,pos,bv.size(),&base_array,base_pos);
                    //::set_bt(base_array, base_pos, bv[pos], bv.size());
                    bt_array[i++] = rrr_helper_type::get_bt(base_array, base_pos - last_block_size,bv.size() + last_block_size);
                    x = rrr_helper_type::get_bt(bv, pos, bv.size());
                    sum_rank += x;
                    last_block_size += bv.size() - pos;
                    pos = bv.size();
                } else {
                    base_array = copy_from(&bv,pos,t_bs - last_block_size,&base_array,base_pos);
                    //rrr_helper_type::set_bt(base_array, base_pos, bv[pos], t_bs - last_block_size);
                    bt_array[i++] = rrr_helper_type::get_bt(base_array, base_pos - last_block_size, t_bs);
                    x = rrr_helper_type::get_bt(bv, pos, t_bs - last_block_size);
                    sum_rank += x;
                    pos += t_bs - last_block_size;
                    base_pos += t_bs - last_block_size;
                    last_super_block_size += 1;
                    last_block_size = 0;
                }
            } else {
                if (bv.size() - pos < t_bs) {
                    base_array = copy_from(&bv,pos,bv.size()-pos,&base_array,base_pos);
                    //rrr_helper_type::set_bt(base_array, base_pos, bv[pos], bv.size());
                    bt_array[i++] = rrr_helper_type::get_bt(base_array, base_pos - last_block_size,bv.size() + last_block_size);
                    x = rrr_helper_type::get_bt(bv, pos, bv.size()-pos);
                    sum_rank += x;
                    last_block_size += bv.size() - pos;
                    pos = bv.size();
                } else {
                    bt_array[i++] = x = rrr_helper_type::get_bt(bv, pos, t_bs);

                    base_array = copy_from(&bv,pos,t_bs,&base_array,base_pos);
                    //rrr_helper_type::set_bt(base_array, base_pos, bv[pos], t_bs);
                    sum_rank += x;
                    pos += t_bs;
                    base_pos += t_bs;
                    last_super_block_size += 1;
                }
            }

            std::copy(bt_array.begin(), bt_array.end(),superblock_map[superblock_count]->begin());
            std::copy(base_array.begin(), base_array.end(),base_map[superblock_count]->begin());

            if(last_super_block_size == t_sbs) {
                superblock_count += 1;
                adjusted_sb_size += sb_size;
                rank_map[superblock_count] = sum_rank;
            }

            ///////DEBUG//////
            /*
            cout << bt_array[0] << endl;
            cout << bt_array[1] << endl;
            cout << bt_array[2] << endl;

            cout << "Base " << base_array.size() << endl;
            for(int k = 0; k < base_array.size(); k++)
                cout << base_array[k];
            cout << endl;
            cout << "BV " << bv.size() << endl;
            for(int k = 0; k < bv.size(); k++)
                cout << bv[k];
            cout << endl;
            cout << "Cumulated rank :" << sum_rank << endl;
            cout << "Superblock count :" << superblock_count << endl;
            */
            /////////////////
        }
        // If we still have bits to add, we start a new superblock
        while (pos != bv.size()) {
            last_super_block_size = 0;
            last_block_size = 0;

            superblock_map[superblock_count] = new int_vector<>;
            base_map[superblock_count] = new bit_vector(sb_size);
            superblock_map[superblock_count]->width(bits::hi(t_bs) + 1);
            superblock_map[superblock_count]->resize(t_sbs); // size for the a superblock's bloc ranks

            bt_array = *superblock_map[superblock_count];
            base_array = *base_map[superblock_count];

            i = 0;
            base_pos = 0;

            while (pos + t_bs <= adjusted_sb_size  && pos + t_bs <= bv.size()) { // handle all full blocks
                bt_array[i++] = x = rrr_helper_type::get_bt(bv, pos, t_bs);
                base_array = copy_from(&bv,pos,t_bs,&base_array,base_pos);
                //cout << "x = " << x << endl;
                //cout << "bt_array = " << bt_array[i-1] << endl;
                //cout << "sum_rank = " << sum_rank + x << endl;

                //rrr_helper_type::set_bt(base_array, base_pos, bv[pos], t_bs);
                sum_rank += x;
                pos += t_bs;
                base_pos += t_bs;
                last_super_block_size += 1;
                last_block_size = 0;
            }
            if (pos < bv.size() && pos + t_bs > bv.size() && pos < adjusted_sb_size) { // handle last not full block
                bt_array[i++] = x = rrr_helper_type::get_bt(bv, pos, bv.size() - pos);
                base_array = copy_from(&bv,pos,bv.size()-pos,&base_array,base_pos);
                //rrr_helper_type::set_bt(base_array, base_pos, bv[pos], t_bs);
                sum_rank += x;
                last_block_size = bv.size() - pos;
                pos = bv.size();
            }

            std::copy(bt_array.begin(), bt_array.end(),superblock_map[superblock_count]->begin());
            std::copy(base_array.begin(), base_array.end(),base_map[superblock_count]->begin());

            if(last_super_block_size == t_sbs) {
                superblock_count += 1;
                adjusted_sb_size += sb_size;
                rank_map[superblock_count] = sum_rank;
                last_super_block_size = 0;
            }

            ///////DEBUG//////
            /*
            cout << bt_array[0] << endl;
            cout << bt_array[1] << endl;
            cout << bt_array[2] << endl;

            cout << "Base " << base_array.size() << endl;
            for(int k = 0; k < base_array.size(); k++)
                cout << base_array[k];
            cout << endl;
            cout << "BV " << bv.size() << endl;
            for(int k = 0; k < bv.size(); k++)
                cout << bv[k];
            cout << endl;
            cout << "Cumulated rank :" << sum_rank << endl;
            cout << "Superblock count :" << superblock_count << endl;
            */
            /////////////////
        }

        o_size += bv.size();
    }

    bool at(rrr_vector_extendable::size_type i) {
        assert(i<o_size);
        if(i < base_vec->size())
            return base_vec->operator[](i);
        else{
            int sbid = (i/ t_bs)/t_sbs;
            return base_map[sbid]->data()[i%(t_sbs*t_bs)];
        }
    }

    sdsl::rrr_vector_mod<>::iterator begin() const{
        cout << "BEGIN shouldn't be used" << endl;
        return base_vec->begin();
    }

    sdsl::rrr_vector_mod<>::iterator end() const{
        cout << "END shouldn't be used" << endl;
        return base_vec->end();
    }

    //! Swap method
    void swap(rrr_vector_extendable<>& rrr)
    {
        if (this != &rrr) {
            //TODO Swap
        }
    }


    size_type size()const
    {
        return o_size;
    }

    void display(){


        cout << "////////////////////////////////////////" << endl;

        cout << "base vector :" << endl;
        for(auto i : *base_vec){
            cout << i;
        }
        cout << endl;

        cout << "extensions :" << endl;
        if(superblock_map.find(0) == superblock_map.end()){
            cout << "////////////////////////////////////////" << endl;
            return;
        }
        for(int i = 0; i <= superblock_count; i++) {

            cout << "SB  " << i << endl;


            auto bt_array = *superblock_map[i];
            auto base_array = *base_map[i];

            for(auto i : bt_array)
                cout << i;
            cout << endl;

            cout << "Starting rank = " << rank_map[i] << endl;
            cout << "Base " << base_array.size() << endl;
            for (int k = 0; k < base_array.size(); k++)
                cout << base_array[k];
            cout << endl;
        }
        cout << "////////////////////////////////////////" << endl;
    }

    map<int, int_vector<>*> get_sbm(){
        return superblock_map;
    }

    int get_sbm_at(int sb, int b){
        return superblock_map[sb]->data()[b];
    }

    map<int, size_type> get_rm(){
        return rank_map;
    }
    map<int, bit_vector*> get_bm(){
        return base_map;
    }

};

template<uint8_t t_b, uint16_t t_bs, class t_rac, uint16_t t_sbs>
class rank_support_rrr_ex
{
    static_assert(t_b == 1u or t_b == 0u , "rank_support_rrr: bit pattern must be `0` or `1`");
public:
    typedef rrr_vector_extendable<t_bs, t_rac, t_sbs> bit_vector_type;
    typedef typename bit_vector_type::size_type size_type;
    typedef typename bit_vector_type::rrr_helper_type rrr_helper_type;
    typedef typename rrr_helper_type::number_type number_type;
    enum { bit_pat = t_b };
    enum { bit_pat_len = (uint8_t)1 };

    friend class rrr_vector_extendable<>;
    rrr_vector_extendable<t_bs, t_rac, t_sbs> *m_v;

public:
    //! Standard constructor
    /*! \param v Pointer to the rrr_vector, which should be supported
     */
    explicit rank_support_rrr_ex( rrr_vector_extendable<t_bs, t_rac, t_sbs> *v=nullptr)
    {
        m_v = v;
    }

    void set_vector( rrr_vector_extendable<t_bs, t_rac, t_sbs> *v=nullptr)
    {
        m_v = v;
    }

    size_type rank(size_type r) const {
        return t_b ? rank1(r) : rank0(r);
    }

    size_type rank0(size_type r) const {
        return r-rank1(r);
    }

    //! Answers rank queries
    /*! \param r Argument for the length of the prefix v[0..i-1], with \f$0\leq i \leq size()\f$.
       \returns Number of 1-bits in the prefix [0..i-1] of the original bit_vector.
       \par Time complexity
            \f$ \Order{ sample\_rate of the rrr\_vector} \f$
    */
    size_type rank1(size_type r) const {
        if (m_v->base_vec != nullptr && r <= m_v->base_vec->size())
            return m_v->bv_rank.rank(r);
        else {
            r -= m_v->base_vec->size();
            int block_id = r / t_bs;
            int superblock_id = block_id / t_sbs;
            int rank = m_v->rank_map[superblock_id];
            //cout << "mapped rank" << rank << endl;

            int i = 0;
            for(auto j : *(m_v->superblock_map[superblock_id])){
                if(i < block_id%t_sbs){
                    rank+=j;
                    i+=1;
                }
            }

            //cout << "bloc ranks " << rank << endl;
            //cout << "block_id " << block_id << endl;
            //cout << "superblock_id " << superblock_id << endl;

            //rank += rrr_helper_type::get_bt(base_map[superblock_id], (t_bs * block_id) % (t_bs * t_sbs),(r % t_bs));
            int start = (block_id * t_bs) % (t_sbs * t_bs);

            i = 0;
            for(auto j : *(m_v->base_map[superblock_id])){
                if(i < start+(r % t_bs)){
                    if(i >= start)
                        if (j)
                            rank += 1;
                    i+=1;
                }
            }
            /*
            for(int i = 0; i < (r % t_bs); i++) {
                //cout << base_map[superblock_id][block_id*t_bs + i] << endl;
                if (m_v->base_map[superblock_id]->data()[start + i]){
                    rank += 1;
                }
            }*/

            return rank;
        }
    }

    //! Swap method
    void swap(rank_support_rrr_ex& rrr){
    }
};
template<uint8_t t_b, uint16_t t_bs, class t_rac, uint16_t t_k>
class select_support_rrr_ex
{
    static_assert(t_b == 1u or t_b == 0u , "select_support_rrr: bit pattern must be `0` or `1`");
public:
    typedef rrr_vector_extendable<t_bs, t_rac, t_k> bit_vector_type;
    typedef typename bit_vector_type::size_type size_type;
    typedef typename bit_vector_type::rrr_helper_type rrr_helper_type;
    typedef typename rrr_helper_type::number_type number_type;
    enum { bit_pat = t_b };
    enum { bit_pat_len = (uint8_t)1 };
    friend class rrr_vector_extendable<>;
    rrr_vector_extendable<t_bs, t_rac, t_k> * m_v;

private:

    size_type select1(size_type i)const {
        if(i == 0)
            return 0;
        if(i <= m_v->base_rank){
            return m_v->bv_select.select(i);
        }
        //cout << m_v->base_rank << endl;
        rank_support_rrr_ex<1,t_bs, t_rac, t_k> vec_rank;
        vec_rank.set_vector(m_v->select1_vec);

        size_type sb;

        if(i-m_v->base_rank == m_v->select1_vec->size())
            sb = m_v->select1_map[m_v->select1_map.size()-1];
        else {
            int r = vec_rank.rank(i - m_v->base_rank);
            sb = m_v->select1_map[r];
        }
        size_type rank = m_v->rank_map[sb];

        //cout << "rank " << rank << endl;

        // Calcul du premier index possible dans la bitmap
        size_type index = m_v->base_vec->size() + (sb * t_bs * t_k) ;


        //cout << "size " << m_v->base_map[sb]->size() << endl;
        auto v = *m_v->base_map[sb];
        for(auto j : v){
            //cout << j;
            if (j){
                rank += 1;
                if(rank == i){
                    return index;
                }
            }
            index += 1;
        }
        //cout << endl;

        return -1;
    }


    size_type select0(size_type i)const{
        if(i == 0)
            return 0;
        if(i <= m_v->base_vec->size()-m_v->base_rank){
            return m_v->bv_select0.select(i);
        }

        rank_support_rrr_ex<1,t_bs, t_rac, t_k> vec_rank;
        vec_rank.set_vector(m_v->select0_vec);

        size_type sb;

        if((m_v->base_vec->size()-m_v->base_rank) == m_v->select0_vec->size())
            sb = m_v->select0_map[m_v->select0_map.size()-1];
        else {
            int r = vec_rank.rank(i - (m_v->base_vec->size()-m_v->base_rank));
            sb = m_v->select0_map[r];
        }
        //Inversed to count zeroes
        size_type rank = ((sb * t_bs * t_k) + m_v->base_vec->size()) - m_v->rank_map[sb];

        // Calcul du premier index possible dans la bitmap
        size_type index = m_v->base_vec->size() + (sb * t_bs * t_k) ;

        //cout << "size " << m_v->base_map[sb]->size() << endl;
        auto v = *m_v->base_map[sb];
        for(auto j : v){
            //cout << j;
            if (!j){
                rank += 1;
                if(rank == i){
                    return index;
                }
            }
            index += 1;
        }
        //cout << endl;

        return -1;
    }



public:
    explicit select_support_rrr_ex(rrr_vector_extendable<t_bs, t_rac, t_k>* v=nullptr)
    {
        m_v = v;
    }

    //! Answers select queries
    size_type select(size_type i)const
    {
        return  t_b ? select1(i) : select0(i);
    }

    const size_type operator()(size_type i)const
    {
        return select(i);
    }

    const size_type size()const
    {
        return m_v->size();
    }

    //! Swap method
    void swap(select_support_rrr_ex& rrr){
    }

    void set_vector( rrr_vector_extendable<t_bs, t_rac, t_k> * v=nullptr)
    {
        m_v = v;
    }
};

#endif //SDSL_RRR_VECTOR_EXTENDABLE_H