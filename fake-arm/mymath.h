#ifndef _mymath_h
#define _mymath_h

#define sqr(x) ((x)*(x))
#define saturate(x, M, m) ((x > M) ? M : (x < m) ? m : x)
#define sign(x) ((x > 0) ? 1 : (x < 0) ? -1 : 0)

#endif
