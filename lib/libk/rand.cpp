#include "stdlib.h"
#include "kernel/libk_decls.h"
/* Pseudo-random generator based on Minimal Standard by
	 Lewis, Goodman, and Miller in 1969.
	 I[j+1] = a*I[j] (mod m)
	 where a = 16807
				 m = 2147483647
	 Using Schrage's algorithm, a*I[j] (mod m) can be rewritten as:
		 a*(I[j] mod q) - r*{I[j]/q}      if >= 0
		 a*(I[j] mod q) - r*{I[j]/q} + m  otherwise
	 where: {} denotes integer division 
					q = {m/a} = 127773 
					r = m (mod a) = 2836
	 note that the seed value of 0 cannot be used in the calculation as
	 it results in 0 itself.
*/
extern "C"
{
		extern unsigned long __rdseed(unsigned long __unsupported_default);
		static unsigned long __local_seed = 0;
		void srand(unsigned int seed)  { __local_seed = __rdseed(0x12345678FFFFFFFF) & seed; }
		int rand()
		{
				// Use hardware RNG if possible for initial seed
				if(!__local_seed) __local_seed = __rdseed(0x123456787);
				long k        = __local_seed / 127773;
				long result   = 16807 * (__local_seed - 127773 * k) - 2836 * k;
				result        &= 0x7FFFFFFF;
				__local_seed  = result;
				return static_cast<int>(result);
		}
}