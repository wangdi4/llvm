#ifdef A_C
int func();
int main() {
  return func();
}
#endif
#ifdef B_C
int func() {
  return 0;
}
#endif
