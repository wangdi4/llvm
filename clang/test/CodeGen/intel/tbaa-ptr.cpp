// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility \
// RUN:   -O1 -disable-llvm-passes -emit-llvm -o - %s \
// RUN:   | FileCheck %s

// Check that a separate type is not created for 'const' versions on the type.
// See CMPLRLLVM-11944.

struct VIt {
  typedef const int* _Iter;
  _Iter _M_current;
  const _Iter &base() const noexcept { return _M_current; }
};

bool operator==(const VIt &A, const VIt &B) {
  return A.base() == B.base();
}

struct Vec {
  bool empty();
  VIt _M_start;
  VIt _M_end;
};

//CHECK-LABEL: empty
bool Vec::empty()
{
  //CHECK: %call{{.*}}base
  //CHECK-NEXT: load{{.*}}!tbaa [[TB:![0-9]*]]
  //CHECK: %call{{.*}}base
  //CHECK-NEXT: load{{.*}}!tbaa [[TB]]
  return _M_start.base() == _M_end.base();
}

//CHECK: [[OC:![0-9]*]] = !{!"omnipotent char",
//CHECK: [[TB]] = !{[[TB2:![0-9]*]], [[TB2]], i64 0}
//CHECK-NEXT: [[TB2]] = !{!"pointer@_ZTSPi", [[OC]], i64 0}
