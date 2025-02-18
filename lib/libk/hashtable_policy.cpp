#include "bits/hashtable_policy.hpp"
#include "algorithm" // lower_bound
namespace std
{
    namespace __impl
    {
        const unsigned long __prime_list[] = // 256 + 1 or 256 + 48 + 1
        {
            2UL, 3UL, 5UL, 7UL, 11UL, 13UL, 17UL, 19UL, 23UL, 29UL, 31UL,
            37UL, 41UL, 43UL, 47UL, 53UL, 59UL, 61UL, 67UL, 71UL, 73UL, 79UL,
            83UL, 89UL, 97UL, 103UL, 109UL, 113UL, 127UL, 137UL, 139UL, 149UL,
            157UL, 167UL, 179UL, 193UL, 199UL, 211UL, 227UL, 241UL, 257UL,
            277UL, 293UL, 313UL, 337UL, 359UL, 383UL, 409UL, 439UL, 467UL,
            503UL, 541UL, 577UL, 619UL, 661UL, 709UL, 761UL, 823UL, 887UL,
            953UL, 1031UL, 1109UL, 1193UL, 1289UL, 1381UL, 1493UL, 1613UL,
            1741UL, 1879UL, 2029UL, 2179UL, 2357UL, 2549UL, 2753UL, 2971UL,
            3209UL, 3469UL, 3739UL, 4027UL, 4349UL, 4703UL, 5087UL, 5503UL,
            5953UL, 6427UL, 6949UL, 7517UL, 8123UL, 8783UL, 9497UL, 10273UL,
            11113UL, 12011UL, 12983UL, 14033UL, 15173UL, 16411UL, 17749UL,
            19183UL, 20753UL, 22447UL, 24281UL, 26267UL, 28411UL, 30727UL,
            33223UL, 35933UL, 38873UL, 42043UL, 45481UL, 49201UL, 53201UL,
            57557UL, 62233UL, 67307UL, 72817UL, 78779UL, 85229UL, 92203UL,
            99733UL, 107897UL, 116731UL, 126271UL, 136607UL, 147793UL,
            159871UL, 172933UL, 187091UL, 202409UL, 218971UL, 236897UL,
            256279UL, 277261UL, 299951UL, 324503UL, 351061UL, 379787UL,
            410857UL, 444487UL, 480881UL, 520241UL, 562841UL, 608903UL,
            658753UL, 712697UL, 771049UL, 834181UL, 902483UL, 976369UL,
            1056323UL, 1142821UL, 1236397UL, 1337629UL, 1447153UL, 1565659UL,
            1693859UL, 1832561UL, 1982627UL, 2144977UL, 2320627UL, 2510653UL,
            2716249UL, 2938679UL, 3179303UL, 3439651UL, 3721303UL, 4026031UL,
            4355707UL, 4712381UL, 5098259UL, 5515729UL, 5967347UL, 6456007UL,
            6984629UL, 7556579UL, 8175383UL, 8844859UL, 9569143UL, 10352717UL,
            11200489UL, 12117689UL, 13109983UL, 14183539UL, 15345007UL,
            16601593UL, 17961079UL, 19431899UL, 21023161UL, 22744717UL,
            24607243UL, 26622317UL, 28802401UL, 31160981UL, 33712729UL,
            36473443UL, 39460231UL, 42691603UL, 46187573UL, 49969847UL,
            54061849UL, 58488943UL, 63278561UL, 68460391UL, 74066549UL,
            80131819UL, 86693767UL, 93793069UL, 101473717UL, 109783337UL,
            118773397UL, 128499677UL, 139022417UL, 150406843UL, 162723577UL,
            176048909UL, 190465427UL, 206062531UL, 222936881UL, 241193053UL,
            260944219UL, 282312799UL, 305431229UL, 330442829UL, 357502601UL,
            386778277UL, 418451333UL, 452718089UL, 489790921UL, 529899637UL,
            573292817UL, 620239453UL, 671030513UL, 725980837UL, 785430967UL,
            849749479UL, 919334987UL, 994618837UL, 1076067617UL, 1164186217UL,
            1259520799UL, 1362662261UL, 1474249943UL, 1594975441UL, 1725587117UL,
            1866894511UL, 2019773507UL, 2185171673UL, 2364114217UL, 2557710269UL,
            2767159799UL, 2993761039UL, 3238918481UL, 3504151727UL, 3791104843UL,
            4101556399UL, 4294967291UL,
            6442450933UL, 8589934583UL, 12884901857UL, 17179869143UL,
            25769803693UL, 34359738337UL, 51539607367UL, 68719476731UL,
            103079215087UL, 137438953447UL, 206158430123UL, 274877906899UL,
            412316860387UL, 549755813881UL, 824633720731UL, 1099511627689UL,
            1649267441579UL, 2199023255531UL, 3298534883309UL, 4398046511093UL,
            6597069766607UL, 8796093022151UL, 13194139533241UL, 17592186044399UL,
            26388279066581UL, 35184372088777UL, 52776558133177UL, 70368744177643UL,
            105553116266399UL, 140737488355213UL, 211106232532861UL, 281474976710597UL,
            562949953421231UL, 1125899906842597UL, 2251799813685119UL,
            4503599627370449UL, 9007199254740881UL, 18014398509481951UL,
            36028797018963913UL, 72057594037927931UL, 144115188075855859UL,
            288230376151711717UL, 576460752303423433UL,
            1152921504606846883UL, 2305843009213693951UL,
            4611686018427387847UL, 9223372036854775783UL,
            18446744073709551557UL, 18446744073709551557UL
        };
        static inline size_t __clp2(size_t n) noexcept
        {
            if(n < 2) return n;
            const unsigned __lz = sizeof(size_t) > sizeof(long) ? __builtin_clzll(n - 1ULL) : __builtin_clzl(n - 1UL);
            return (size_t(1) << ((sizeof(size_t) * __CHAR_BIT__ - 1) - __lz)) << 1;
        }
        size_t __prime_number_policy::__buckets_for_elements(size_t n) const { return static_cast<size_t>(__builtin_ceil(n / static_cast<double>(__my_max_load))); }
        size_t __prime_number_policy::__next_bucket(size_t n) const
        {
            if(!n) return 1;
            static const unsigned char __fast_bkt[] = { 2, 2, 2, 3, 5, 5, 7, 7, 11, 11, 11, 11, 13, 13 };
            if(n < sizeof(__fast_bkt))
            {
                __next_resize = __builtin_floor(__fast_bkt[n] * static_cast<double>(__my_max_load));
                return __fast_bkt[n];
            }
            constexpr auto n_primes = sizeof(__prime_list) / sizeof(unsigned long) - 1;
            constexpr auto last_prime = __prime_list + n_primes - 1;
            const unsigned long* __next = lower_bound(__prime_list + 6, last_prime, n);
            if(__next == last_prime) { __next_resize = size_t(-1); }
            else { __next_resize = __builtin_floor(*__next * static_cast<double>(__my_max_load)); }
            return *__next;
        }
        pair<bool, size_t> __prime_number_policy::__need_rehash(size_t n_buckets, size_t n_elems, size_t n_ins) const
        {
            if(n_elems + n_ins > __next_resize)
            {
                double __min_bkts = max<size_t>(n_elems + n_ins, __next_resize ? 0UL : 11UL) / static_cast<double>(__my_max_load);
                if(__min_bkts >= n_buckets) return make_pair(true, __next_bucket(max<size_t>(__builtin_floor(__min_bkts) + 1, n_buckets * __growth_factor)));
                __next_resize = __builtin_floor(n_buckets * static_cast<double>(__my_max_load));
                return make_pair(false, 0UL);
            }
            else return make_pair(false, 0UL);
        }
        size_t __pow2_policy::__buckets_for_elements(size_t n) const { return static_cast<size_t>(__builtin_ceil(n / static_cast<double>(__my_max_load))); }
        size_t __pow2_policy::__next_bucket(size_t n) const
        {
            if(!n) return 1;
            constexpr auto __max_wid = std::min(sizeof(size_t), 8UL);
            constexpr auto __max_bkt = size_t(1) << (__max_wid * __CHAR_BIT__ - 1);
            size_t res = __clp2(n);
            if(!res) res = __max_bkt;
            else if (res == 1) res = 2;
            if(res == __max_bkt) { __next_resize = size_t(-1); }
            else __next_resize = __builtin_floor(res * static_cast<double>(__my_max_load));
            return res;
        }
        pair<bool, size_t> std::__impl::__pow2_policy::__need_rehash(size_t n_buckets, size_t n_elems, size_t n_ins) const
        {
            if(n_elems + n_ins > __next_resize)
            {
                double __min_bkts = max<size_t>(n_elems + n_ins, __next_resize ? 0UL : 11UL) / static_cast<double>(__my_max_load);
                if(__min_bkts >= n_buckets) return make_pair(true, __next_bucket(max<size_t>(__builtin_floor(__min_bkts) + 1, n_buckets * __growth_factor)));
                __next_resize = __builtin_floor(n_buckets * static_cast<double>(__my_max_load));
                return make_pair(false, 0UL);
            }
            else return make_pair(false, 0UL);
        }
    }
}