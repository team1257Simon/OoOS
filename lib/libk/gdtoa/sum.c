#include "gdtoa.h"
Bigint* __sum_D2A(Bigint* a, Bigint* b)
{
  Bigint* c;
  ULong carry, *xc, *xa, *xb, *xe, y;
  ULong z;
  if (a->wds < b->wds) {
    c = b;
    b = a;
    a = c;
  }
  c = __Balloc_D2A(a->k);
  if (c == NULL)
    return (NULL);
  c->wds = a->wds;
  carry = 0;
  xa = a->x;
  xb = b->x;
  xc = c->x;
  xe = xc + b->wds;
  do {
    y = (*xa & 0xffff) + (*xb & 0xffff) + carry;
    carry = (y & 0x10000) >> 16;
    z = (*xa++ >> 16) + (*xb++ >> 16) + carry;
    carry = (z & 0x10000) >> 16;
    (((unsigned short*)xc)[1] = (unsigned short)z, ((unsigned short*)xc)[0] = (unsigned short)y, xc++);
  } while (xc < xe);
  xe += a->wds - b->wds;
  while (xc < xe) {
    y = (*xa & 0xffff) + carry;
    carry = (y & 0x10000) >> 16;
    z = (*xa++ >> 16) + carry;
    carry = (z & 0x10000) >> 16;
    (((unsigned short*)xc)[1] = (unsigned short)z, ((unsigned short*)xc)[0] = (unsigned short)y, xc++);
  }
  if (carry) {
    if (c->wds == c->maxwds) {
      b = __Balloc_D2A(c->k + 1);
      if (b == NULL)
        return (NULL);
      memcpy(&b->sign, &c->sign, c->wds * sizeof(ULong) + 2 * sizeof(int));
      __Bfree_D2A(c);
      c = b;
    }
    c->x[c->wds++] = 1;
  }
  return c;
}