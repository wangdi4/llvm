// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -O2 -w -no-struct-path-tbaa -disable-llvm-optzns %s -emit-llvm -o - | FileCheck %s --implicit-check-not=fakeload
template <class _Hp>
struct Foo {
  Foo(); ~Foo();
  _Hp __value__;
  _Hp get() const { return __value__; }
};

void foo() {
  Foo<void (&)()> ff;
  ff.get();
}

