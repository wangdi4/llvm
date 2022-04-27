// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
// CQ#366312
// RUN: %clang_cc1 -fintel-compatibility %s -emit-llvm -opaque-pointers -o - | FileCheck %s

typedef struct foo {
  short field1;
  long field2;
  char *field3;
} foo_t;

struct Item {
  char *field1;
  short field2;
};

void check() {

  short *p1;
  int *p2;
  foo_t *p3;
  char *p4;
  double *p5;
  int i;
  __int64 i64;

  // CHECK: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i16, ptr %{{.+}}, i32 1
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  *((short *)p3)++;

  // CHECK-NEXT: store i32 5, ptr %{{.+}}
  (unsigned int)i64 = 5;

  // CHECK-NEXT: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i32, ptr %{{.+}}, i32 1
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  ((int *)p3)++;

  // CHECK: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds double, ptr %{{.+}}, i32 1
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  // CHECK-NEXT: store i32 5, ptr %{{.+}}
  (int)*p5++ = 5;

  const Item *p6;
  int i1, i2, i3;
  extern const char *src[];
  extern char *dst[];

  // CHECK: %{{.+}} = load i32, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i64
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds [0 x ptr], ptr @{{.+}}, i64 0, i64 %{{.+}}
  // CHECK-NEXT: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds %{{.+}}, ptr %{{.+}}, i32 0, i32 0
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  const_cast<char *>(p6->field1) = dst[i3];

  // CHECK: %{{.+}} = load i32, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = load i32, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i64
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds [0 x ptr], ptr @{{.+}}, i64 0, i64 %{{.+}}
  // CHECK-NEXT: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i6
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i8, ptr %{{.+}}, i64 %{{.+}}
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  reinterpret_cast<const char *>(src[i3]) += i1;

  // CHECK: %{{.+}} = load i32, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = load i32, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i6
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds [0 x ptr], ptr @{{.+}}, i64 0, i64 %{{.+}}
  // CHECK-NEXT: %{{.+}} = load ptr, ptr %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i64
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i8, ptr %{{.+}}, i64 %{{.+}}
  // CHECK-NEXT: store ptr %{{.+}}, ptr %{{.+}}
  reinterpret_cast<char *>(dst[i3]) += i2;

}

