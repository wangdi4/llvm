int x = 2;
int subr1(int x);

int subr2(int x) {
  return subr1(x - 1);
}
