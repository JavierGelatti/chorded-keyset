#include "bit_manipulation.h"

void setBit(int &n, int k) {
  n |= 1 << k;
}

void clearBit(int &n, int k) {
  n &= ~(1 << k);
}

bool testBit(int n, int k) {
  return (n & (1 << k)) != 0;
}
