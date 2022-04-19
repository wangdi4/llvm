// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -O1 \
// RUN: -no-struct-path-tbaa -disable-llvm-optzns %s -opaque-pointers -emit-llvm -o - | \
// RUN: FileCheck %s
// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -O1 \
// RUN:                     -disable-llvm-optzns %s -opaque-pointers -emit-llvm -o - | \
// RUN: FileCheck %s -check-prefix=PATH
namespace std {
template <typename> struct complex {
  float real() const { return __real__ _M_value; }
  float imag() const { return __imag__ _M_value; }
  template <typename a> complex &operator+=(const complex<a> &__z) {
    __real__ _M_value += __z.real();
    __imag__ _M_value += __z.imag();
    return *this;
  }
  _Complex float _M_value;
};
template <typename a>
inline complex<a> operator+(const complex<a> &__x, const complex<a> &__y) {
  complex<a> b = __x;
  b += __y;
  return b;
}
} // namespace std
using namespace std;
void foo(complex<float> *dest, complex<float> *src1, complex<float> *src2) {
  long l1;
#pragma ivdep
  for (l1 = 0; l1 < 100; l1++)
    dest[l1] = src1[l1] + src2[l1];
  // PATH: define {{.*}}void @_Z3fooPSt7complexIfES1_S1_{{.*}}
  // PATH: for.body:{{.*}}
  // PATH: call void @llvm.memcpy{{.*}}, !tbaa.struct [[TBAA_struct:!.*]]
  // CHECK: call void @llvm.memcpy{{.*}}, !tbaa.struct [[TBAA_struct:!.*]]

  // PATH: define {{.*}}@_ZNSt7complexIfEpLIfEERS0_RKS_IT_E{{.*}}
  // PATH: {{.*}} = load float, {{.*}} !tbaa [[TAG_float_field:!.*]]
  // PATH: {{.*}} = call noundef float @_ZNKSt7complexIfE4imagEv{{.*}}
  // PATH: {{.*}} = getelementptr inbounds %"struct.std::complex", ptr {{.*}} i32 0, !intel-tbaa [[TAG_scomplex:!.*]]
  // CHECK: {{.*}} = getelementptr inbounds %"struct.std::complex", ptr {{.*}} i32 0
}
// PATH: [[TYPE_char:!.*]] = !{!"omnipotent char", [[TAG_cxx_tbaa:!.*]], {{.*}}
// PATH: [[TBAA_struct]] = !{i64 0, i64 4, [[TAG_float:!.*]], i64 4, i64 4, [[TAG_float]]}
// PATH: [[TAG_float]] = !{!"float", !{{.*}}, i64 0}
// PATH: [[TAG_scomplex]] = !{[[TAG_s:!.*]], [[TAG_c:!.*]], i64 0}
// PATH: [[TAG_s]] = !{!"struct@_ZTSSt7complexIfE", [[TAG_c]], i64 0}
// PATH: [[TAG_c]] = !{!"_Complex@_ZTSf", [[TAG_float]], i64 0, [[TAG_float]], i64 4}
// PATH: [[TAG_float_field]] = !{[[TAG_float]], [[TAG_float]], i64 0}

// CHECK: [[TYPE_char:!.*]] = !{!"omnipotent char", [[TAG_cxx_tbaa:!.*]], {{.*}}
// CHECK: [[TBAA_struct]] = !{i64 0, i64 8, [[TAG_9:!.*]]}
// CHECK: [[TAG_9]] = !{[[TYPE_char]], [[TYPE_char]], i64 0}
// CHECK-NOT: !{!"_Complex@_ZTSf", {{.*}}
// CHECK: [[TAG_float:!.*]] = !{!"float", !{{.*}}, i64 0}
