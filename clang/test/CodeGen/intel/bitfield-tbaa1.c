// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -fintel-compatibility-enable=IntelTBAABF -O1 -disable-llvm-optzns %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -O1 -disable-llvm-optzns %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -fintel-compatibility-disable=IntelTBAABF -O1 -disable-llvm-optzns %s -emit-llvm -o - | FileCheck %s -check-prefix=CHECK-DISABLED
struct in_t {
  int x,y,w;
    unsigned char  xage : 2;
    unsigned char  yage1 : 2;
    unsigned int  zage2 : 12;
    unsigned char  other;
    unsigned int  zage3 : 12;
};

/*
 * No AA info for the bitfield. Suggest adding a field in the struct
 * path info of the size of the load/store.
*/

int fum(struct in_t *sp) {
    sp->xage = 2;
// CHECK-LABEL: define i32 @fum
// CHECK:  %xage = getelementptr inbounds %struct.in_t, %struct.in_t* %0, i32 0, i32 3
// CHECK:  %bf.load = load i16, i16* %xage, align 4, !tbaa [[TAG_6:!.*]]
// CHECK-DISABLED-NOT: %bf.load = load i16, i16* %xage, align 4, !tbaa{{.*}}
// CHECK:  %bf.clear = and i16 %bf.load, -4
// CHECK:  %bf.set = or i16 %bf.clear, 2
// CHECK:  store i16 %bf.set, i16* %xage, align 4, !tbaa [[TAG_6]]
// CHECK-DISABLED-NOT:  store i16 %bf.set, i16* %xage, align 4, !tbaa{{.*}}
    sp->zage2 = 2;
// CHECK:  %zage2 = getelementptr inbounds %struct.in_t, %struct.in_t* %1, i32 0, i32 3
// CHECK:  %bf.load1 = load i16, i16* %zage2, align 4, !tbaa [[TAG_6]]
// CHECK-DISABLED-NOT:  %bf.load1 = load i16, i16* %zage2, align 4, !tbaa{{.*}}
// CHECK:  %bf.clear2 = and i16 %bf.load1, 15
// CHECK:  %bf.set3 = or i16 %bf.clear2, 32
// CHECK:  store i16 %bf.set3, i16* %zage2, align 4, !tbaa [[TAG_6]]
// CHECK-DISABLED-NOT:  store i16 %bf.set3, i16* %zage2, align 4, !tbaa{{.*}}
    sp->zage3 = 3;
// CHECK:  %zage3 = getelementptr inbounds %struct.in_t, %struct.in_t* %2, i32 0, i32 5
// CHECK:  %bf.load4 = load i16, i16* %zage3, align 4, !tbaa [[TAG_10:!.*]]
// CHECK-DISABLED-NOT:  %bf.load4 = load i16, i16* %zage3, align 4, !tbaa{{.*}}
// CHECK:  %bf.clear5 = and i16 %bf.load4, -4096
// CHECK:  %bf.set6 = or i16 %bf.clear5, 3
// CHECK:  store i16 %bf.set6, i16* %zage3, align 4, !tbaa [[TAG_10]]
// CHECK-DISABLED-NOT:  store i16 %bf.set6, i16* %zage3, align 4, !tbaa{{.*}}
    return sp->yage1 + sp->zage2 + sp->zage3;
}
int fum1(void) {
// CHECK-LABEL: define i32 @fum1
  typedef struct S {
    unsigned bf : 7;
    int a;
    unsigned bf2 : 2;
  } S_type;
  S_type s;
  return s.bf = 42;
// CHECK:  %bf.load = load i8, i8* %1, align 4, !tbaa [[TAG_11:!.*]]
// CHECK-DISABLED-NOT:  %bf.load = load i8, i8* %1, align 4, !tbaa{{.*}}
// CHECK:  %bf.clear = and i8 %bf.load, -128
// CHECK:  %bf.set = or i8 %bf.clear, 42
// CHECK:  store i8 %bf.set, i8* %1, align 4, !tbaa [[TAG_11]]
// CHECK-DISABLED-NOT:  store i8 %bf.set, i8* %1, align 4, !tbaa{{.*}}
}
// CHECK: [[TYPE_char:!.*]] = !{!"omnipotent char",{{.*}}}
// CHECK: = !{!"Simple C/C++ TBAA"}
// junk: [[TAG_6]] = !{[[TYPE_7:!.*]], ![[TYPE_9:!.*]], i64 12}
// CHECK: [[TAG_6]] = !{[[TYPE_7:![0-9]+]], [[TYPE_9:![0-9]+]], i64 12}
// CHECK: [[TYPE_7]] = !{!"struct@in_t",{{.*}}}
// junk: [[TYPE_7]] = !{!"struct@in_t".*}
// CHECK: [[TYPE_9]] = !{!"short", {{.*}}, i64 0}
// CHECK: [[TAG_10]] = !{[[TYPE_7]], [[TYPE_9]], i64 16}
// CHECK: [[TAG_11]] = !{[[TYPE_12:![0-9]+]], [[TYPE_char]], i64 0}
// CHECK: [[TYPE_12]] = !{!"struct@S",{{.*}}}
