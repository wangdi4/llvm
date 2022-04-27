// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
// CQ#366312
// RUN: %clang_cc1 -fintel-compatibility %s -emit-llvm -opaque-pointers -o - | FileCheck %s

typedef struct foo {
  short field1;
  long field2;
  char *field3;
} foo_t;

void check() {

  short *p1;
  int *p2;
  foo_t *p3;
  char *p4;
  double *p5;
  int i;
  __int64 i64;

  // CHECK: %{{.+}} = load i32, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = load i8, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i8 %{{.+}} to i32
  // CHECK-NEXT: %{{.+}} = mul nsw i32 %{{.+}}, %{{.+}}
  // CHECK-NEXT: %{{.+}} = trunc i32 %{{.+}} to i8
  // CHECK-NEXT: store i8 %{{.+}}, ptr %{{.+}}
  (char)p1 *= i;

  // CHECK: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = ptrtoint ptr %{{.+}} to i32
  // CHECK-NEXT: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i64
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds double, ptr %{{.+}}, i64 %{{.+}}
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  (double *)p1 += (int)p2;

  // CHECK: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i16, ptr %{{.+}}, i32 1
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  *((short *)p3)++;

  // CHECK: store i32 5, ptr %{{.+}}
  (unsigned int)i64 = 5;

  // CHECK: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i32, ptr %{{.+}}, i32 1
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  ((int *)p3)++;

  // CHECK: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds double, ptr %{{.+}}, i32 1
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  // CHECK-NEXT: store i32 5, ptr %{{.+}}
  (int)*p5++ = 5;

}

