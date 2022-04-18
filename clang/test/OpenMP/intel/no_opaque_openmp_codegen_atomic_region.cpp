// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -fintel-openmp-region-atomic \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: foo
void foo() {
  long int n1, n2;
  n1 = 0;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.UPDATE"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic
  ++n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.UPDATE"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic update
  ++n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.READ"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic read
  n2 = n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.WRITE"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: store i64 1, i64*
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic write
  n1 = 1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.CAPTURE"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic capture
  n2 = ++n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.UPDATE.SEQ_CST"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic seq_cst
  ++n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.UPDATE.SEQ_CST"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic seq_cst update
  ++n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.READ.SEQ_CST"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic read, seq_cst
  n2 = n1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.WRITE.SEQ_CST"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: store i64 1, i64*
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic write seq_cst
  n1 = 1;
// CHECK: region.entry() [ "DIR.OMP.ATOMIC"()
// CHECK-SAME: "QUAL.OMP.CAPTURE.SEQ_CST"
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK: region.exit{{.*}}"DIR.OMP.END.ATOMIC"()
#pragma omp atomic seq_cst, capture
  n2 = ++n1;
}
