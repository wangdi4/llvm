int i;

void subr2() {
  ++i;
}

void subr1() {
  subr2();
}

int main() {
  subr1();
  return 0;
}
