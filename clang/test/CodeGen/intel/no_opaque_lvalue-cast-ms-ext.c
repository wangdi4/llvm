// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
// CQ#366312
// RUN: %clang_cc1 -fintel-compatibility %s -emit-llvm -no-opaque-pointers -o - | FileCheck %s

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

  // CHECK: %{{.+}} = load i32, i32* %{{.+}}
  // CHECK-NEXT: %{{.+}} = bitcast i16** %{{.+}} to i8*
  // CHECK-NEXT: %{{.+}} = load i8, i8* %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i8 %{{.+}} to i32
  // CHECK-NEXT: %{{.+}} = mul nsw i32 %{{.+}}, %{{.+}}
  // CHECK-NEXT: %{{.+}} = trunc i32 %{{.+}} to i8
  // CHECK-NEXT: store i8 %{{.+}}, i8* %{{.+}}
  (char)p1 *= i;

  // CHECK: %{{.+}} = load i32*, i32** %{{.+}}
  // CHECK-NEXT: %{{.+}} = ptrtoint i32* %{{.+}} to i32
  // CHECK-NEXT: %{{.+}} = bitcast i16** %{{.+}} to double**
  // CHECK-NEXT: %{{.+}} = load double*, double** %{{.+}}
  // CHECK-NEXT: %{{.+}} = sext i32 %{{.+}} to i64
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds double, double* %{{.+}}, i64 %{{.+}}
  // CHECK-NEXT: store double* %{{.+}}, double** %{{.+}}
  (double *)p1 += (int)p2;

  // CHECK: %{{.+}} = bitcast %{{.+}}** %{{.+}} to i16*
  // CHECK-NEXT: %{{.+}} = load i16*, i16** %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i16, i16* %{{.+}}, i32 1
  // CHECK-NEXT: store i16* %{{.+}}, i16** %{{.+}}
  *((short *)p3)++;

  // CHECK: %{{.+}} = bitcast i64* %{{.+}} to i32*
  // CHECK-NEXT: store i32 5, i32* %{{.+}}
  (unsigned int)i64 = 5;

  // CHECK: %{{.+}} = bitcast %{{.+}}** %{{.+}} to i32**
  // CHECK-NEXT: %{{.+}} = load i32*, i32** %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds i32, i32* %{{.+}}, i32 1
  // CHECK-NEXT: store i32* %{{.+}}, i32** %{{.+}}
  ((int *)p3)++;

  // CHECK: %{{.+}} = load double*, double** %{{.+}}
  // CHECK-NEXT: %{{.+}} = getelementptr inbounds double, double* %{{.+}}, i32 1
  // CHECK-NEXT: store double* %{{.+}}, double** %{{.+}}
  // CHECK-NEXT: %{{.+}} = bitcast double* %{{.+}} to i32*
  // CHECK-NEXT: store i32 5, i32* %{{.+}}
  (int)*p5++ = 5;

}

