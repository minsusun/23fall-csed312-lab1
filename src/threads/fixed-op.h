#include <stdint.h>
#define _F (1<<14)

int int_fp (int n) { return n * _F; }
int fp_int (int x) { return x / _F; }
int fp_int_round (int x) { return (x >= 0) ? ((x + _F / 2) / _F) : ((x +- _F / 2) / _F); }
int fp_add (int x, int y) { return x + y; }
int fp_sub (int x, int y) { return x - y; }
int fp_mul (int x, int y) { return ((int64_t) x) * y / _F; }
int fp_div (int x, int y) { return ((int64_t) x) * _F / y; }