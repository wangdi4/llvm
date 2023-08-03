; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; typedef short TYPE;
;
; static TYPE x[100];
; static TYPE y[100];
; static const int N = 100;
;
; void my_add(TYPE& lhs, TYPE const &rhs) {
;     lhs += rhs;
; }
;
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in))
;
; // Reduction on:         Array
; // Reduction type:       UDR (no initializer)
; // Array layout:         1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_b() {
; #pragma omp parallel for reduction(my_reduction_add:y)
;     for(int i = 0; i < N; i++) {
;         my_add(y[25 + i/4], x[i]);
;     }
; }
;
; int main(int arc, char** argv) {
;     int i;
;
;     for (i = 0; i < N; i++) {
;         x[i] = 1;
;         y[i] = i;
;     }
;
;     cq415166_1d_b();
;
;     return 0;
; }

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: store {{.*}} 0, {{.*}}
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}

; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL1y_ec586bddf7946ec5149ed0e0672a3784 = internal global [100 x i16] zeroinitializer, align 16
@_ZL1x_ec586bddf7946ec5149ed0e0672a3784 = internal global [100 x i16] zeroinitializer, align 16

define dso_local void @_Z13cq415166_1d_bv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr @_ZL1y_ec586bddf7946ec5149ed0e0672a3784, i16 0, i32 100, ptr null, ptr null, ptr @.omp_combiner., ptr null),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x_ec586bddf7946ec5149ed0e0672a3784, i16 0, i32 100) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load i32, ptr %i, align 4
  %div = sdiv i32 %5, 4
  %add1 = add nsw i32 25, %div
  %idxprom = sext i32 %add1 to i64
  %arrayidx = getelementptr inbounds [100 x i16], ptr @_ZL1y_ec586bddf7946ec5149ed0e0672a3784, i64 0, i64 %idxprom
  %6 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds [100 x i16], ptr @_ZL1x_ec586bddf7946ec5149ed0e0672a3784, i64 0, i64 %idxprom2
  call void @_Z6my_addRsRKs(ptr dereferenceable(2) %arrayidx, ptr dereferenceable(2) %arrayidx3)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %7, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @_Z6my_addRsRKs(ptr dereferenceable(2) %lhs, ptr dereferenceable(2) %rhs)
declare void @.omp_combiner.(ptr noalias %0, ptr noalias %1)
