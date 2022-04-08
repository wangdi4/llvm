// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s
template <typename a> struct b { using c = a; };
template <typename a, template <typename> class> using e = b<a>;
template <typename a, template <typename> class d>
using f = typename e<a, d>::c;
struct g {
  template <typename h> using i = typename h::j;
};
struct k : g {
  using j = f<int *, i>;
};
struct l {
  k ::j m;
};
class n {
  l *o;
  int p;
};
class q : n {
} r;

// CHECK: !intel.dtrans.types = !{![[Q:[0-9]+]], ![[NBASE:[0-9]+]], ![[L:[0-9]+]], ![[N:[0-9]+]]}

// ![[Q]] = !{!"S", %class.q zeroinitializer, i32 2, ![[NBASE_REF:[0-9]+]], ![[CHAR_ARRAY:[0-9]+]]}
// ![[NBASE_REF]] = !{%class.n.base zeroinitializer, i32 0}
// ![[CHAR_ARRAY]] = !{!"A", i32 4, ![[CHAR:[0-9]+]]}
// ![[CHAR]] = !{i8 0, i32 0}
// ![[NBASE]] = !{!"S", %class.n.base zeroinitializer, i32 2, ![[L_PTR:[0-9]+]], ![[INT:[0-9]+]]}
// ![[L_PTR]] = !{%struct.l zeroinitializer, i32 1}
// ![[INT]] = !{i32 0, i32 0}
// ![[L]] = !{!"S", %struct.l zeroinitializer, i32 1, ![[INT_PTR:[0-9]+]]}
// ![[INT_PTR]] = !{i32 0, i32 1}
// ![[N]] = !{"S", %class._ZTS1n.n zeroinitializer, i32 3, ![[L_PTR]], ![[INT]], ![[CHAR_ARRAY]]}
