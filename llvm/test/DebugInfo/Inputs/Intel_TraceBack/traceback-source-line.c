extern int foo(void);
extern int bar(void);

int binop(int a, int b) {
  if (a)
    b -= foo();
  else
    b -= bar();
  return b;
}
