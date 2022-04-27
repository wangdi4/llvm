// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -O1 \
// RUN:-no-struct-path-tbaa -disable-llvm-optzns %s -opaque-pointers -emit-llvm -o - | \
// RUN:FileCheck %s
// RUN: %clang_cc1 -triple x86_64-apple-darwin -fintel-compatibility -O1 \
// RUN:                     -disable-llvm-optzns %s -opaque-pointers -emit-llvm -o - | \
// RUN:FileCheck %s -check-prefix=PATH
void foo(_Complex double *dest) {
  long l1;
  // CHECK: for.cond:{{.*}}
  // CHECK: {{.*}} load i64, ptr %l1, {{.*}} [[TAG_l1_tbaa:!.*]]

  for (l1 = 0; l1 < 100; l1++) {
    dest[l1] = __builtin_complex((double)(l1), (double)(l1 + 1));
    // PATH: %arrayidx = getelementptr inbounds
    // PATH: %arrayidx.realp = getelementptr inbounds { double, double }, ptr %arrayidx, i32 0, i32 0, !intel-tbaa [[TAG_realp:!.*]]
    // PATH: store double {{.*}} ptr %arrayidx.realp, align 8, !tbaa [[TAG_realp]]
    // PATH: %arrayidx.imagp = getelementptr inbounds { double, double }, ptr %arrayidx, i32 0, i32 1, !intel-tbaa [[TAG_imagp:!.*]]
    // PATH: store double %conv1, ptr %arrayidx.imagp, align 8, !tbaa [[TAG_imagp]]
  }
}
// PATH: [[TYPE_char:!.*]] = !{!"omnipotent char", [[TAG_cxx_tbaa:!.*]], {{.*}}
// PATH: [[TAG_realp]] = !{[[TAG_complex:!.*]], [[TAG_double:!.*]], i64 0}
// PATH: [[TAG_complex]] = !{!"_Complex@_ZTSd", [[TAG_double]], i64 0, [[TAG_double]], i64 8}
// PATH: [[TAG_double]] = !{!"double", !4, i64 0}
// PATH: [[TAG_imagp]] = !{[[TAG_complex]], [[TAG_double]], i64 8}

// CHECK: [[TYPE_char:!.*]] = !{!"omnipotent char", [[TAG_cxx_tbaa:!.*]],{{.*}}
// CHECK: [[TAG_l1_tbaa]] = !{[[TAG_long:!.*]], [[TAG_long]], i64 0}
// CHECK: [[TAG_long]] = !{!"long", [[TYPE_char]], i64 0}
