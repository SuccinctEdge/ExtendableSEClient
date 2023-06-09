/* sdsl - succinct data structures library
    Copyright (C) 2012-2013 Simon Gog
    Copyright (C) 2013 Timo Beller
*/

#include "sdsl/construct_lcp.hpp"
#include "sdsl/construct.hpp"
#include <stdexcept>
#include <algorithm>

namespace sdsl
{

void construct_lcp_semi_extern_PHI(cache_config& config)
{
    typedef int_vector<>::size_type size_type;
    int_vector_buffer<> sa_buf(cache_file_name(conf::KEY_SA, config));
    size_type n = sa_buf.size();
    if (1==n) {
        int_vector<> lcp(1, 0);
        store_to_cache(lcp, conf::KEY_LCP, config);
        return;
    }
    const uint8_t log_q = 6; // => q=64
    const uint32_t q = 1<<log_q;
    const uint64_t modq = bits::lo_set[log_q];

    // n-1 is the maximum entry in SA
    int_vector<64> plcp((n-1+q)>>log_q);

    for (size_type i=0, sai_1=0; i < n; ++i) {   // we can start at i=0. if SA[i]%q==0
        // we set PHI[(SA[i]=n-1)%q]=0, since T[0]!=T[n-1]
        size_type sai = sa_buf[i];
        if ((sai & modq) == 0) {
            if ((sai>>log_q) >= plcp.size()) {
//                    std::cerr<<"sai="<<sai<<" log_q="<<log_q<<" sai>>log_q="<<(sai>>log_q)<<" "<<sai_1<<std::endl;
//                    std::cerr<<"n="<<n<<" "<<" plcp.size()="<<plcp.size();
            }
            plcp[sai>>log_q] = sai_1;
        }
        sai_1 = sai;
    }

    int_vector<8> text;
    load_from_cache(text, conf::KEY_TEXT, config);

    for (size_type i=0,j,k,l=0; i < plcp.size(); ++i) {
        j =	i<<log_q;   // j=i*q
        k = plcp[i];
        while (text[j+l] == text[k+l])
            ++l;
        plcp[i] = l;
        if (l >= q) {
            l -= q;
        } else {
            l = 0;
        }
    }

    size_type buffer_size = 4000000; // buffer_size is a multiple of 8!
    sa_buf.buffersize(buffer_size);
    int_vector_buffer<> lcp_out_buf(cache_file_name(conf::KEY_LCP, config), std::ios::out, buffer_size, sa_buf.width());	// open buffer for plcp

    for (size_type i=0, sai_1=0,l=0, sai=0,iq=0; i < n; ++i) {
        /*size_type*/ sai = sa_buf[i];
//				std::cerr<<"i="<<i<<" sai="<<sai<<std::endl;
        if ((sai & modq) == 0) { // we have already worked the value out ;)
            lcp_out_buf[i] = l=plcp[sai>>log_q];
        } else {
            /*size_type*/ iq = sai & bits::lo_unset[log_q];
            l  = plcp[sai>>log_q];
            if (l >	(sai-iq))
                l -= (sai-iq);
            else
                l=0;
            while (text[ sai+l ] == text[ sai_1+l ])
                ++l;
            lcp_out_buf[i] = l;
        }
#ifdef CHECK_LCP
        size_type j=0;
        for (j=0; j<l; ++j) {
            if (text[sai+j] !=text[sai_1+j]) {
                std::cout<<"lcp["<<i<<"]="<<l<<" is two big! "<<j<<" is right!"<<" sai="<<sai<<std::endl;
                if ((sai&modq)!=0)
                    std::cout<<" plcp[sai>>log_q]="<<plcp[sai>>log_q]<<" sai-iq="<<sai-iq<<" sai="<<sai<<" sai-iq="<<sai-iq<<std::endl;
                break;
            }
        }
#endif
        sai_1 = sai;
    }
    lcp_out_buf.close();
    register_cache_file(conf::KEY_LCP, config);
    return;
}

void construct_lcp_go(cache_config& config)
{
    typedef int_vector<>::size_type size_type;
#ifdef STUDY_INFORMATIONS
    size_type racs  = 0; // random accesses to the text
    size_type matches = 0;
    size_type comps2 = 0; // comparisons the second phase
#endif
    int_vector<8> text;
    load_from_cache(text, conf::KEY_TEXT, config);
    int_vector_buffer<> sa_buf(cache_file_name(conf::KEY_SA, config));   // initialize buffer for suffix array
    const size_type n = sa_buf.size();
    const size_type m = 254; // LCP[i] == m+1 corresp. to LCP[i]>= m+1; LCP[i] <= m corresp. to LCP[i] was calculated

    if (1==n) {
        int_vector<> lcp(1, 0);
        store_to_cache(lcp, conf::KEY_LCP, config);
        return;
    }

    size_type cnt_c[257] = {0};   // counter for each character in the text
    size_type cnt_cc[257] = {0};  // prefix sum of the counter cnt_c
    size_type cnt_cc2[257] = {0};  //
    size_type omitted_c[257] = {0};  // counts the omitted occurrences for the second phase
    size_type prev_occ_in_bwt[256] = {0};  // position of the previous occurrence of each character c in the bwt
    for (size_type i=0; i<256; ++i) prev_occ_in_bwt[i] = (size_type)-1; // initialize the array with -1
    unsigned char alphabet[257] = {0};
    uint8_t sigma = 0;

    tLI m_list[2][256];
    size_type m_char_count[2] = {0};
    uint8_t m_chars[2][256] = {{0},{0}};

    size_type nn = 0; // n' for phase 2
    // phase 1: calculate lcp_sml;  memory consumption: 2n bytes (lcp_sml=n bytes, text=n bytes)
    {

        int_vector<8> lcp_sml(n, 0); // initialize array for small values of first phase; note lcp[0]=0
        size_type done_cnt=0;

        for (size_type i=0; i<n; ++i) { // initialize cnt_c
            ++cnt_c[text[i]+1];
        }
        for (int i=1; i<257; ++i) { // calculate sigma and initailize cnt_cc
            if (cnt_c[i] > 0) {
                alphabet[sigma++] = (unsigned char)(i-1);
            }
            cnt_cc[i] = cnt_c[i] + cnt_cc[i-1];
        }
        alphabet[sigma] = '\0';
        {
            int_vector_buffer<8> bwt_buf(cache_file_name(conf::KEY_BWT, config)); // initialize buffer of bwt
            size_type sai_1 = sa_buf[0];  // store value of sa[i-1]
            uint8_t bwti_1 = bwt_buf[0];       // store value of BWT[i-1]
            lcp_sml[ cnt_cc[bwti_1]++ ] = 0;   // lcp_sml[ LF[0] ] = 0
            prev_occ_in_bwt[bwti_1] = 0;  // init previous occurence of character BWT[0]
            ++omitted_c[alphabet[0]];	  //

            int_vector<64> rmq_stack(2*(m+10));   // initialize stack for m+10 elements representing (position, value)
            rmq_stack[0] = 0; rmq_stack[1] = 0;  // first element (-1, -1)
            rmq_stack[2] = 1; rmq_stack[3] = 0;  // second element (0, -1)
            size_type rmq_end=3;				 // index of the value of the topmost element

            const size_type m_mod2 = m%2;
            uint8_t cur_c = alphabet[1];
            size_type big_val = 0;
            for (size_type i=1, sai, cur_c_idx=1, cur_c_cnt=cnt_c[alphabet[1]+1]; i < n; ++i, --cur_c_cnt) {
                uint8_t bwti = bwt_buf[i];
                sai = sa_buf[i];
                size_type lf = cnt_cc[bwti];
                if (!cur_c_cnt) {// cur_c_cnt==0, if there is no more occurence of the current character
                    if (cur_c_cnt < sigma) {
                        cur_c_cnt = cnt_c[(cur_c=alphabet[++cur_c_idx])+1];
                    }
                }
                size_type l=0;
                if (i >= cnt_cc[cur_c]) { // if the current lcp entry is not already done   TODO: schleife von i bis cnt_cc[cur_c]
                    if (bwti == bwti_1 and lf < i) {  // BWT[i]==BWT[i-1]
                        l = lcp_sml[lf] ? lcp_sml[lf]-1 : 0; // l = LCP[LF[i]]-1; l < m+1
                        if (l == m) { // if LCP[LF[i]] == m+1; otherwise LCP[LF[i]] < m+1  the result is correct
                            l += (text[sai_1+m] == text[sai+m]);
#ifdef STUDY_INFORMATIONS
                            if ((sai_1^sai)>>6) // if i and phii are in the same cache line
                                ++racs;
#endif
                        }
                        lcp_sml[i] = l;
                        ++done_cnt;
                    } else { // BWT[i] != BWT[i-1] or LF[i] > i
                        if (lf < i)
                            l = lcp_sml[lf] ? lcp_sml[lf]-1 : 0;
#ifdef STUDY_INFORMATIONS
                        if ((sai_1^sai)>>6) // if i and phii are in the same cache line
                            ++racs;
#endif
                        while (text[sai_1+l] == text[sai+l] and l < m+1) {
                            ++l;
#ifdef STUDY_INFORMATIONS
                            ++matches;
#endif
                        }
                        lcp_sml[i] = l;
                    }
                } else { // if already done
                    l = lcp_sml[i];  // load LCP value
                }
                if (l > m) {
                    ++big_val;
                    if (i > 10000 and i < 10500 and big_val > 3000) { // if most of the values are big: switch to PHI algorithm
                        util::clear(text);
                        util::clear(lcp_sml);
                        construct_lcp_PHI<8>(config);
                        return;
                    }
                }
                // invariant: l <= m+1
                // begin update rmq_stack
                size_type x = l+1;
                size_type j = rmq_end;
                while (x <= rmq_stack[j]) j-=2;  // pop all elements with value >= l
                rmq_stack[++j] = i+1; // push position i
                rmq_stack[++j] = x;	  // push value	l
                rmq_end = j;          // update index of the value of the topmost element
                if (lf > i) {   // if LF[i] > i, we can calculate LCP[LF[i]] in constant time with rmq
                    ++done_cnt;
                    // rmq query for lcp-values in the interval I=[prev_occ_in_bwt[BWT[i]]+1..i]
                    // rmq is linear in the stack size; can also be implemented with binary search on the stack
                    size_type x_pos = prev_occ_in_bwt[bwti]+2;
                    j = rmq_end-3;
                    while (x_pos <= rmq_stack[j]) j-=2;   //  search smallest value in the interval I
                    lcp_sml[lf] = rmq_stack[j+3] - (rmq_stack[j+3]==m+2); // if lcp-value equals m+1, we subtract 1
                }
                if (l >= m) {
                    if (l == m)
                        push_front_m_index(nn, cur_c, m_list[m_mod2], m_chars[m_mod2], m_char_count[m_mod2]);
                    ++nn;
                } else
                    ++omitted_c[cur_c];

                prev_occ_in_bwt[bwti] = i;  	 // update previous position information for character BWT[i]
                ++cnt_cc[bwti];					 // update counter and therefore the LF information
                sai_1 = sai;					 // update SA[i-1]
                bwti_1 = bwti;					 // update BWT[i-1]
            }
        }
        util::clear(text);

        if (n > 1000 and nn > 5*(n/6)) {  // if we would occupy more space than the PHI algorithm => switch to PHI algorithm
            util::clear(lcp_sml);
            construct_lcp_PHI<8>(config);
            return;
        }
        store_to_cache(lcp_sml, "lcp_sml", config);
    }
#ifdef STUDY_INFORMATIONS
    std::cout<<"# n="<<n<<" nn="<<nn<<" nn/n="<<((double)nn)/n<<std::endl;
#endif

    // phase 2: calculate lcp_big
    {
//			std::cout<<"# begin calculating LF' values"<<std::endl;
        int_vector<> lcp_big(nn, 0, bits::hi(n-1)+1); // lcp_big first contains adapted LF values and finally the big LCP values
        {
            // initialize lcp_big with adapted LF values
            bit_vector todo(n,0);  // bit_vector todo indicates which values are >= m in lcp_sml
            {
                // initialize bit_vector todo
                int_vector_buffer<8> lcp_sml_buf(cache_file_name("lcp_sml", config)); // load lcp_sml
                for (size_type i=0; i < n; ++i) {
                    if (lcp_sml_buf[i] >= m) {
                        todo[i] = 1;
                    }
                }
            }

            cnt_cc2[0] = cnt_cc[0]= 0;
            for (size_type i=1, omitted_sum=0; i<257; ++i) { // initialize cnt_cc and cnt_cc2
                cnt_cc[i] = cnt_c[i] + cnt_cc[i-1];
                omitted_sum += omitted_c[i-1];
                cnt_cc2[i] = cnt_cc[i] - omitted_sum;
            }

            int_vector_buffer<8> bwt_buf(cache_file_name(conf::KEY_BWT, config)); // load BWT
            for (size_type i=0, i2=0; i < n; ++i) {
                uint8_t b = bwt_buf[i];  // store BWT[i]
                size_type lf_i = cnt_cc[b]; // LF[i]
                if (todo[i]) { // LCP[i] is a big value
                    if (todo[lf_i]) { // LCP[LF[i]] is a big entry
                        lcp_big[i2] = cnt_cc2[b]; // LF'[i]
                    }/*else{
							lcp_big[i2] = 0;
						}*/
                    ++i2;
                }
                if (todo[lf_i]) { // LCP[LF[i]] is a big entry
                    ++cnt_cc2[b];	// increment counter for adapted LF
                }
                ++cnt_cc[b]; // increment counter for LF
            }
        }

//			std::cout<<"# begin initializing bwt2, shift_bwt2, run2"<<std::endl;
        int_vector<8> bwt2(nn), shift_bwt2(nn); // BWT of big LCP values, and shifted BWT of big LCP values
        bit_vector run2(nn+1);					// indicates for each entry i, if i and i-1 are both big LCP values
        run2[nn] = 0;							// index nn is not a big LCP value
        {
            // initialize bwt2, shift_bwt2, adj2
            int_vector_buffer<8> lcp_sml_buf(cache_file_name("lcp_sml", config)); // load lcp_sml
            int_vector_buffer<8> bwt_buf(cache_file_name(conf::KEY_BWT, config)); // load BWT
            uint8_t b_1 = '\0'; // BWT[i-1]
            bool is_run = false;
            for (size_type i=0, i2=0; i < n; ++i) {
                uint8_t b = bwt_buf[i];
                if (lcp_sml_buf[i] >= m) {
                    bwt2[i2] 		= b;
                    shift_bwt2[i2]	= b_1;
                    run2[i2] 		= is_run;
                    is_run 			= true;
                    ++i2;
                } else {
                    is_run = false;
                }
                b_1 = b;
            }
        }

        bit_vector todo2(nn+1, 1); // init all values with 1, except
        todo2[nn] = 0; 			   // the last one! (handels case "i < nn")

//			std::cout<<"# begin calculating m-indices"<<std::endl;
        {
            // calculate m-indices, (m+1)-indices,... until we are done
            size_type m2 = m;
            size_type char_ex[256]; for (size_type i=0; i<256; ++i) char_ex[i] = nn;
            size_type char_occ=0;
            size_type m_mod2 = m2%2, mm1_mod2 = (m2+1)%2;
            while (m_char_count[m_mod2] > 0) { // while there are m-indices, calculate (m+1)-indices and write m-indices
                // For all values LCP[i] >= m2 it follows that todo2[i] == 1
                // list m_list[mm1_mod2][b] is sorted in decreasing order
                ++m2;
                mm1_mod2 = (m2+1)%2, m_mod2 = m2%2;
                m_char_count[m_mod2] = 0;

                std::sort(m_chars[mm1_mod2], m_chars[mm1_mod2]+m_char_count[mm1_mod2]); // TODO: ersetzen?

                for (size_type mc=0; mc<m_char_count[mm1_mod2]; ++mc) { // for every character
                    tLI& mm1_mc_list = m_list[mm1_mod2][m_chars[mm1_mod2][ m_char_count[mm1_mod2]-1-  mc ]];
//						size_type old_i = nn;
                    while (!mm1_mc_list.empty()) {
                        size_type i = mm1_mc_list.front();  // i in [0..n-1]
                        mm1_mc_list.pop_front();
                        // For all values LCP[i] >= m-1 it follows that todo2[i] == 1
                        for (size_type k=i; todo2[k]; --k) {
#ifdef STUDY_INFORMATIONS
                            ++comps2;
#endif
                            uint8_t b = shift_bwt2[k];
                            if (char_ex[b] != i) {
                                char_ex[b] = i;
                                ++char_occ;
                            }
                            if (!run2[k])
                                break;
                        }
                        for (size_type k=i; todo2[k] and char_occ; ++k) {
#ifdef STUDY_INFORMATIONS
                            ++comps2;
#endif
                            uint8_t b = bwt2[k];
                            if (char_ex[b] == i) {
                                size_type p = lcp_big[k];
                                push_back_m_index(p, b, m_list[m_mod2], m_chars[m_mod2], m_char_count[m_mod2]);
                                char_ex[b] = nn;
                                --char_occ;
                            }
                            if (!run2[k+1])
                                break;
                        }
                        lcp_big[ i ] = m2-1;
                        todo2[ i ] = 0;
//							old_i = i;
                    }
                }
            }

        }

        store_to_cache(lcp_big, "lcp_big", config);
    } // end phase 2

//		std::cout<<"# merge lcp_sml and lcp_big"<<std::endl;
    // phase 3: merge lcp_sml and lcp_big and save to disk
    {
        const size_type buffer_size = 1000000; // buffer_size has to be a multiple of 8!
        int_vector_buffer<> lcp_big_buf(cache_file_name("lcp_big", config)); 					// file buffer containing the big LCP values
        int_vector_buffer<8> lcp_sml_buf(cache_file_name("lcp_sml", config), std::ios::in, buffer_size);		// file buffer containing the small LCP values
        int_vector_buffer<> lcp_buf(cache_file_name(conf::KEY_LCP, config), std::ios::out, buffer_size, lcp_big_buf.width()); // buffer for the resulting LCP array
        for (size_type i=0, i2=0; i < n; ++i) {
            size_type l = lcp_sml_buf[i];
            if (l >= m) { // if l >= m it is stored in lcp_big
                l = lcp_big_buf[i2];
                ++i2;
            }
            lcp_buf[i] = l;
        }
        lcp_buf.close();
    }
    register_cache_file(conf::KEY_LCP, config);

#ifdef STUDY_INFORMATIONS
    std::cout<<"# racs: "<<racs<<std::endl;
    std::cout<<"# matches: "<<matches<<std::endl;
    std::cout<<"# comps2: "<<comps2<<std::endl;
#endif
    return;
}

void construct_lcp_goPHI(cache_config& config)
{
    typedef int_vector<>::size_type size_type;
    int_vector<8> text;
    load_from_cache(text, conf::KEY_TEXT, config);  // load text from file system
    int_vector_buffer<> sa_buf(cache_file_name(conf::KEY_SA, config));   // initialize buffer for suffix array
    const size_type n = sa_buf.size();
    const size_type m = 254; // LCP[i] == m+1 corresp. to LCP[i]>= m+1; LCP[i] <= m corresp. to LCP[i] was calculated

    if (1==n) {
        int_vector<> lcp(1, 0);
        store_to_cache(lcp, conf::KEY_LCP, config);
        return;
    }

    size_type cnt_c[257] = {0};   // counter for each character in the text
    size_type cnt_cc[257] = {0};  // prefix sum of the counter cnt_c
    size_type omitted_c[257] = {0};  // counts the omitted occurrences for the second phase
    size_type prev_occ_in_bwt[256] = {0};  // position of the previous occurrence of each character c in the bwt
    for (size_type i=0; i<256; ++i) prev_occ_in_bwt[i] = (size_type)-1; // initialize the array with -1
    unsigned char alphabet[257] = {0};
    uint8_t sigma = 0;

    size_type nn = 0; // n' for phase 2
    // phase 1: calculate lcp_sml;  memory consumption: 2n bytes (lcp_sml=n bytes, text=n bytes)
    {

        int_vector<8> lcp_sml(n, 0); // initialize array for small values of first phase; note lcp[0]=0
        size_type done_cnt=0;

        for (size_type i=0; i<n; ++i) { // initialize cnt_c
            ++cnt_c[text[i]+1];
        }
        for (int i=1; i<257; ++i) { // calculate sigma and initailize cnt_cc
            if (cnt_c[i] > 0) {
                alphabet[sigma++] = (unsigned char)(i-1);
            }
            cnt_cc[i] = cnt_c[i] + cnt_cc[i-1];
        }
        alphabet[sigma] = '\0';
        {
            int_vector_buffer<8> bwt_buf(cache_file_name(conf::KEY_BWT, config)); // initialize buffer of bwt
            size_type sai_1 = sa_buf[0];  // store value of sa[i-1]
            uint8_t bwti_1 = bwt_buf[0];       // store value of BWT[i-1]
            lcp_sml[ cnt_cc[bwti_1]++ ] = 0;   // lcp_sml[ LF[0] ] = 0
            prev_occ_in_bwt[bwti_1] = 0;  // init previous occurence of character BWT[0]
            ++omitted_c[alphabet[0]];	  //

            int_vector<64> rmq_stack(2*(m+10));   // initialize stack for m+10 elements representing (position, value)
            rmq_stack[0] = 0; rmq_stack[1] = 0;  // first element (-1, -1)
            rmq_stack[2] = 1; rmq_stack[3] = 0;  // second element (0, -1)
            size_type rmq_end=3;				 // index of the value of the topmost element

            uint8_t cur_c = alphabet[1];
            for (size_type i=1, sai, cur_c_idx=1, cur_c_cnt=cnt_c[alphabet[1]+1]; i < n; ++i, --cur_c_cnt) {
                uint8_t bwti = bwt_buf[i];
                sai = sa_buf[i];
                size_type lf = cnt_cc[bwti];
                if (!cur_c_cnt) {// cur_c_cnt==0, if there is no more occurence of the current character
                    if (cur_c_cnt < sigma) {
                        cur_c_cnt = cnt_c[(cur_c=alphabet[++cur_c_idx])+1];
                    }
                }
                size_type l=0;
                if (i >= cnt_cc[cur_c]) { // if the current lcp entry is not already done   TODO: schleife von i bis cnt_cc[cur_c]
                    if (bwti == bwti_1 and lf < i) {  // BWT[i]==BWT[i-1]
                        l = lcp_sml[lf] ? lcp_sml[lf]-1 : 0; // l = LCP[LF[i]]-1; l < m+1
                        if (l == m) { // if LCP[LF[i]] == m+1; otherwise LCP[LF[i]] < m+1  the result is correct
                            l += (text[sai_1+m] == text[sai+m]);
                        }
                        lcp_sml[i] = l;
                        ++done_cnt;
                    } else { // BWT[i] != BWT[i-1] or LF[i] > i
                        if (lf < i)
                            l = lcp_sml[lf] ? lcp_sml[lf]-1 : 0;
                        while (text[sai_1+l] == text[sai+l] and l < m+1) {
                            ++l;
                        }
                        lcp_sml[i] = l;
                    }
                } else { // if already done
                    l = lcp_sml[i];  // load LCP value
                }
                // invariant: l <= m+1
                // begin update rmq_stack
                size_type x = l+1;
                size_type j = rmq_end;
                while (x <= rmq_stack[j]) j-=2;  // pop all elements with value >= l
                rmq_stack[++j] = i+1; // push position i
                rmq_stack[++j] = x;	  // push value	l
                rmq_end = j;          // update index of the value of the topmost element
                if (lf > i) {   // if LF[i] > i, we can calculate LCP[LF[i]] in constant time with rmq
                    ++done_cnt;
                    // rmq query for lcp-values in the interval I=[prev_occ_in_bwt[BWT[i]]+1..i]
                    // rmq is linear in the stack size; can also be implemented with binary search on the stack
                    size_type x_pos = prev_occ_in_bwt[bwti]+2;
                    j = rmq_end-3;
                    while (x_pos <= rmq_stack[j]) j-=2;   //  search smallest value in the interval I
                    lcp_sml[lf] = rmq_stack[j+3] - (rmq_stack[j+3]==m+2); // if lcp-value equals m+1, we subtract 1
                }
                if (l > m) {
                    ++nn;
                } else
                    ++omitted_c[cur_c];

                prev_occ_in_bwt[bwti] = i;  	 // update previous position information for character BWT[i]
                ++cnt_cc[bwti];					 // update counter and therefore the LF information
                sai_1 = sai;					 // update SA[i-1]
                bwti_1 = bwti;					 // update BWT[i-1]
            }
        }
        store_to_cache(lcp_sml, "lcp_sml", config);
    }

    // phase 2: calculate lcp_big with PHI algorithm on remaining entries of LCP
    {
        int_vector<> lcp_big(0, 0, bits::hi(n-1)+1);//nn, 0, bits::hi(n-1)+1);
        {

            memory_monitor::event("lcp-init-phi-begin");
            size_type sa_n_1 = 0;  // value for SA[n-1]
            bit_vector todo(n,0);  // bit_vector todo indicates which values are > m in lcp_sml
            {
                // initialize bit_vector todo
                int_vector_buffer<8> lcp_sml_buf(cache_file_name("lcp_sml", config)); // load lcp_sml
                for (size_type i=0; i < n; ++i) {
                    if (lcp_sml_buf[i] > m) {
                        todo[sa_buf[i]] = 1;
                    }
                }
                sa_n_1 = sa_buf[n-1];
            }
            rank_support_v<> todo_rank(&todo); // initialize rank for todo

            const size_type bot = sa_n_1;
            int_vector<> phi(nn, bot, bits::hi(n-1)+1); // phi

            int_vector_buffer<8> bwt_buf(cache_file_name(conf::KEY_BWT, config)); // load BWT
            int_vector_buffer<8> lcp_sml_buf(cache_file_name("lcp_sml", config)); // load lcp_sml
            uint8_t b_1 = 0;
            for (size_type i=0,sai_1=0; i < n; ++i) { // initialize phi
                uint8_t b = bwt_buf[i];  // store BWT[i]
                size_type sai = sa_buf[i];
                if (lcp_sml_buf[i] > m and b != b_1) {  // if i is a big irreducable value
                    phi[todo_rank(sai)] = sai_1;
                } // otherwise phi is equal to bot
                b_1 = b;
                sai_1 = sai;
            }
            memory_monitor::event("lcp-init-phi-end");

            memory_monitor::event("lcp-calc-plcp-begin");
            for (size_type i=0, ii=0, l=m+1,p=0; i < n and ii<nn; ++i) { // execute compact Phi algorithm
                if (todo[i]) {
                    if (i > 0 and todo[i-1])
                        l = l-1;
                    else
                        l = m+1;
                    if ((p=phi[ii]) != bot) {
                        while (text[i+l] == text[p+l]) ++l;
                    }
                    phi[ii++] = l;
                }
            }
            memory_monitor::event("lcp-calc-plcp-end");
            util::clear(text);

            memory_monitor::event("lcp-calc-lcp-begin");
            lcp_big.resize(nn);
            for (size_type i = 0, ii = 0; i < n and ii<nn; ++i) {
                if (lcp_sml_buf[i] > m) {
                    lcp_big[ii++] = phi[todo_rank(sa_buf[i])];
                }
            }
            memory_monitor::event("lcp-calc-lcp-end");
        }
        store_to_cache(lcp_big, "lcp_big", config);
    } // end phase 2

//		std::cout<<"# merge lcp_sml and lcp_big"<<std::endl;
    // phase 3: merge lcp_sml and lcp_big and save to disk
    {
        const size_type buffer_size = 1000000; // buffer_size has to be a multiple of 8!
        int_vector_buffer<> lcp_big_buf(cache_file_name("lcp_big", config)); 					// file buffer containing the big LCP values
        int_vector_buffer<8> lcp_sml_buf(cache_file_name("lcp_sml", config), std::ios::in, buffer_size);		// file buffer containing the small LCP values
        int_vector_buffer<> lcp_buf(cache_file_name(conf::KEY_LCP, config), std::ios::out, buffer_size, lcp_big_buf.width()); // file buffer for the resulting LCP array

        for (size_type i=0, i2=0; i < n; ++i) {
            size_type l = lcp_sml_buf[i];
            if (l > m) { // if l > m it is stored in lcp_big
                l = lcp_big_buf[i2];
                ++i2;
            }
            lcp_buf[i] = l;
        }
        lcp_big_buf.close(true); // close buffer and remove file
        lcp_sml_buf.close(true); // close buffer and remove file
    }
    register_cache_file(conf::KEY_LCP, config);
    return;
}

//void check_lcp(std::string lcpI, std::string lcpII, std::string id)
//{
//    typedef int_vector<>::size_type size_type;
//    int_vector<> lcp1,lcp2;
//    load_from_file(lcp1, (lcpI+"_"+id));
//    load_from_file(lcp2, (lcpII+"_"+id));
//    if (lcp1 != lcp2) {
//        std::cout<<"lcp results of "<<  lcpI << "and " <<lcpII<<" differ"<<std::endl;
//        for (size_type i=0, cnt=0; i<lcp1.size() and cnt<10; ++i) {
//            if (lcp1[i] != lcp2[i]) {
//                std::cout<<"i="<<i<<" "<<lcpI<<"[i]="<<lcp1[i]<<" "<<lcpII<<"[i]="<<lcp2[i]<<std::endl;
//                ++cnt;
//            }
//        }
//    }
//}
//
} // end namespace sdsl
