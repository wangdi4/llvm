; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

;
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
; CRITICAL: call void @.omp_combiner.(i16* %{{.*}}, i16* %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(i16* %{{.*}}, i16* %{{.*}})
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
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR"([100 x i16]* @_ZL1y_ec586bddf7946ec5149ed0e0672a3784, i8* null, i8* null, void (i16*, i16*)* @.omp_combiner., i8* null),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i),
    "QUAL.OMP.SHARED"([100 x i16]* @_ZL1x_ec586bddf7946ec5149ed0e0672a3784) ]

  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load i32, i32* %i, align 4
  %div = sdiv i32 %5, 4
  %add1 = add nsw i32 25, %div
  %idxprom = sext i32 %add1 to i64
  %arrayidx = getelementptr inbounds [100 x i16], [100 x i16]* @_ZL1y_ec586bddf7946ec5149ed0e0672a3784, i64 0, i64 %idxprom
  %6 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds [100 x i16], [100 x i16]* @_ZL1x_ec586bddf7946ec5149ed0e0672a3784, i64 0, i64 %idxprom2
  call void @_Z6my_addRsRKs(i16* dereferenceable(2) %arrayidx, i16* dereferenceable(2) %arrayidx3)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %7, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @_Z6my_addRsRKs(i16* dereferenceable(2) %lhs, i16* dereferenceable(2) %rhs)
declare void @.omp_combiner.(i16* noalias %0, i16* noalias %1)
