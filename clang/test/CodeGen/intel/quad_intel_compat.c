// The special IL0 backend version of the test is in the il0-backend subfolder.
// CQ#366961
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu --extended_float_types -emit-llvm %s -opaque-pointers -o - | FileCheck %s

// CHECK: %struct.anon = type { i32, fp128 }
struct {
  int t;
  _Quad q;
} my_quad;

// CHECK: global fp128 0xL00000000000000000000000000000000
_Quad q1, q2, result;

// CHECK: global ptr null
_Quad *pointer;

void check() {
  // CHECK: store ptr @q1, ptr @pointer, align 8
  pointer = &q1;

  // CHECK: %{{.+}} = load ptr, ptr @pointer, align 8
  // CHECK: %{{.+}} = load fp128, ptr %{{.+}}, align 16
  // CHECK: store fp128 %{{.+}}, ptr @result, align 16
  result = *pointer;

  // CHECK: %{{.+}} = load fp128, ptr @q1, align 16
  // CHECK: %{{.+}} = load fp128, ptr @q2, align 16
  // CHECK: %{{.+}} = fmul fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, ptr @result, align 16
  result = q1 * q2;

  // CHECK: %{{.+}} = load fp128, ptr @q1, align 16
  // CHECK: %{{.+}} = load fp128, ptr @q2, align 16
  // CHECK: %{{.+}} = fdiv fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, ptr @result, align 16
  result = q1 / q2;

  // CHECK: %{{.+}} = load fp128, ptr @q1, align 16
  // CHECK: %{{.+}} = load fp128, ptr @q2, align 16
  // CHECK: %{{.+}} = fadd fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, ptr @result, align 16
  result = q1 + q2;

  // CHECK: %{{.+}} = load fp128, ptr @q1, align 16
  // CHECK: %{{.+}} = load fp128, ptr @q2, align 16
  // CHECK: %{{.+}} = fsub fp128 %{{.+}}, %{{.+}}
  // CHECK: store fp128 %{{.+}}, ptr @result, align 16
  result = q1 - q2;

  // CHECK: %{{.+}} = load fp128, ptr @result, align 16
  // CHECK: %{{.+}} = fptosi fp128 %{{.+}} to i128
  // CHECK: store i128 %{{.+}}, ptr %{{.+}}, align 16
  __int128 v = result;

  // CHECK: %{{.+}} = load i128, ptr %{{.+}}, align 16
  // CHECK: %{{.+}} = sitofp i128 %{{.+}} to fp128
  // CHECK: store fp128 %{{.+}}, ptr @result, align 16
  result = v;
}

int check_sizeof_Quad() {
  // CHECK: ret i32 16
  return sizeof(_Quad);
}

_Quad foo();
__float128 foo();  // No error, _Quad and __float128 are the same type.

#if __is_identifier(__float128)
  void has_no_float128(void);
#else
  void has_float128(void);
#endif

int check_sizeof_float128() {
  // CHECK: call void @has_float128()
  has_float128();
  // CHECK: ret i32 16
  return sizeof(__float128);
}
