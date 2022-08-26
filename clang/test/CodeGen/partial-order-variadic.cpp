<<<<<<< HEAD
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-unknown -fclang-abi-compat=14 -DCLANG_ABI_COMPAT=14 %s -emit-llvm -disable-llvm-passes -o - | FileCheck %s --check-prefix=CHECK-14
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-unknown %s -emit-llvm -disable-llvm-passes -o - | FileCheck %s

#if defined(CLANG_ABI_COMPAT) && CLANG_ABI_COMPAT <= 14

// CHECK-14: %"struct.temp_func_order_example3::S" = type { i8 }

// CHECK-14: define dso_local void @_ZN24temp_func_order_example31hEi(i32 noundef %i)
// CHECK-14-NEXT:  entry:
// CHECK-14-NEXT:    %i.addr = alloca i32, align 4
// CHECK-14-NEXT:    %r = alloca ptr, align 8
// CHECK-14-NEXT:    %a = alloca %"struct.temp_func_order_example3::S", align 1
// CHECK-14-NEXT:    store i32 %i, ptr %i.addr, align 4
// CHECK-14-NEXT:    %call = call noundef nonnull align 4 dereferenceable(4) ptr @_ZN24temp_func_order_example31gIiJEEERiPT_DpT0_(ptr noundef %i.addr)
// CHECK-14-NEXT:    store ptr %call, ptr %r, align 8
// CHECK-14-NEXT:    ret void

namespace temp_func_order_example3 {
  template <typename T, typename... U> int &g(T *, U...);
  template <typename T> void g(T);

  template <typename T, typename... Ts> struct S;
  template <typename T> struct S<T> {};

  void h(int i) {
    int &r = g(&i);
    S<int> a;
  }
=======
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fclang-abi-compat=15 -DCLANG_ABI_COMPAT=15 %s -emit-llvm -disable-llvm-passes -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown %s -emit-llvm -disable-llvm-passes -o - | FileCheck %s --check-prefixes=CHECK,AFTER-15

// CHECK: %struct.S = type { i8 }
// CHECK: @_Z2ggiRi
// CHECK: @_Z1gIiJEERiPT_DpT0_
template <typename T, typename... U> int &g(T *, U...);
template <typename T> void g(T);
template <typename T, typename... Ts> struct S;
template <typename T> struct S<T> {};
void gg(int i, int &r) {
  r = g(&i);
  S<int> a;
>>>>>>> 088ba8efeb4921deb49534714ddefb14a91afe46
}

// CHECK: @_Z1hIJiEEvDpPT_
template<class ...T> void h(T*...) {}
template<class T>    void h(const T&) {}
template void h(int*);

#if !defined(CLANG_ABI_COMPAT)

// AFTER-15: @_Z1fIiJEEvPT_DpT0_
template<class T, class... U> void f(T*, U...){}
template<class T> void f(T){}
template void f(int*);

template<class T, class... U> struct A;
template<class T1, class T2, class... U> struct A<T1,T2*,U...> {};
template<class T1, class T2> struct A<T1,T2>;
template struct A<int, int*>;

#endif
