// CQ#366961
// RUN: %clang_cc1 -fintel-compatibility --extended_float_types -emit-llvm %s -o - | FileCheck %s

// CHECK: %struct.anon = type { i32, fp128 }
struct {
  int t;
  _Quad q;
} my_quad;

// CHECK: common global fp128 0xL00000000000000000000000000000000
_Quad q1, q2, result;

// CHECK: common global fp128* null
_Quad *pointer;

void check() {
  // CHECK: store fp128* @q1, fp128** @pointer, align 8
  pointer = &q1;

  // CHECK: %{{.+}} = load fp128*, fp128** @pointer, align 8
  // CHECK: %{{.+}} = load fp128, fp128* %{{.+}}, align 16
  // CHECK: store fp128 %{{.+}}, fp128* @result, align 16
  result = *pointer;

  // CHECK: %{{.+}} = load fp128, fp128* @q1, align 16
  // CHECK: %{{.+}} = load fp128, fp128* @q2, align 16
  // CHECK: %{{.+}} = fmul fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, fp128* @result, align 16
  result = q1 * q2;

  // CHECK: %{{.+}} = load fp128, fp128* @q1, align 16
  // CHECK: %{{.+}} = load fp128, fp128* @q2, align 16
  // CHECK: %{{.+}} = fdiv fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, fp128* @result, align 16
  result = q1 / q2;

  // CHECK: %{{.+}} = load fp128, fp128* @q1, align 16
  // CHECK: %{{.+}} = load fp128, fp128* @q2, align 16
  // CHECK: %{{.+}} = fadd fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, fp128* @result, align 16
  result = q1 + q2;

  // CHECK: %{{.+}} = load fp128, fp128* @q1, align 16
  // CHECK: %{{.+}} = load fp128, fp128* @q2, align 16
  // CHECK: %{{.+}} = fsub fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, fp128* @result, align 16
  result = q1 - q2;

  // CHECK: %{{.+}} = load fp128, fp128* @result, align 16
  // CHECK: %{{.+}} = fptosi fp128 %{{.+}} to i128
  // CHECK: store i128 %{{.+}}, i128* %{{.+}}, align 16
  __int128 v = result;

  // CHECK: %{{.+}} = load i128, i128* %{{.+}}, align 16
  // CHECK: %{{.+}} = sitofp i128 %{{.+}} to fp128
  // CHECK: store fp128 %{{.+}}, fp128* @result, align 16
  result = v;
}

int check_sizeof_Quad() {
  // CHECK: ret i32 16
  return sizeof(_Quad);
}
