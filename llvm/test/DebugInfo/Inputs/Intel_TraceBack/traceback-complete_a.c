extern int x;
int subr2(int x);

int subr1(int x);

int main() {
  return subr1(x);
}

int subr1(int x) {
  if (!x)
    return 0;
  return 1 + subr2(x);
}
