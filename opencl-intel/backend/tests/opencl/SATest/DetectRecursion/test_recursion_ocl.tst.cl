void bar();

__attribute__((noinline))
void foo() {
  bar();
}

__attribute__((noinline))
void bar() {
  foo();
}

kernel void test() {
  foo();
}
