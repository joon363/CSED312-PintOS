#define F (1 << 14)

int ITOF(int);
int FTOI(int);
int FTOI_ROUND(int);
int ADD(int, int);
int SUB(int, int);
int ADD_FI(int, int);
int SUB_FI(int, int);
int MUL_II(int, int);
int MUL(int, int);
int DIV_II(int, int);
int DIV(int, int);

// Convert n to fixed point:	                    n * f
int ITOF(int n) {
  return n * F;
}

// Convert x to integer (rounding toward zero):   x / f
int FTOI(int x) {
  return x / F;
}

// Convert x to integer (rounding to nearest):	  (x + f / 2) / f if x >= 0, (x - f / 2) / f if x <= 0.
int FTOI_ROUND(int x) {
  if (x >= 0) {
    return (x + F / 2) / F;
  } else {
    return (x - F / 2) / F;
  }
}

// Add x and y:	                                  x + y
int ADD(int x, int y) {
  return x + y;
}

// Subtract y from x:	                            x - y
int SUB(int x, int y) {
  return x - y;
}

// Add x and n:	                                  x + n * f
int ADD_FI(int x, int n) {
  return x + n * F;
}

// Subtract n from x:	                            x - n * f
int SUB_FI(int x, int n) {
  return x - n * F;
}

// Multiply x by y:	                              ((int64_t) x) * y / f
int MUL_II(int x, int y) {
  return ((int64_t) x) * y / F;
}

// Multiply x by n:	                              x * n
int MUL(int x, int n) {
  return x * n;
}

// Divide x by y:	                                ((int64_t) x) * f / y
int DIV_II(int x, int y) {
  return ((int64_t) x) * F / y;
}

// Divide x by n:	                                x / n
int DIV(int x, int n) {
  return x / n;
}