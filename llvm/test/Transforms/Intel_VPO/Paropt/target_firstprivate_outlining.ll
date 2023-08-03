; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -sroa -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,loop-simplify,sroa,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Verify that CodeExtractor does not shrinkwrap alloca for 'i',
; when lifetime markers are present.
; Original code:
; void foo() {
;   int i;
; #pragma omp target firstprivate(i)
;   for (i = 0; i < 200; ++i) {
;   }
; }

; Check that the artificial alloca created by Paropt for i is inserted in
; the function's entry block.
; CHECK: entry:
; CHECK-NOT: br label %{{.*}}
; CHECK: %promoted.clause.args = alloca i8, align 1
; CHECK: br label %{{.*}}

; CHECK: define internal void @__omp_offloading_804_52009c5_foo_l3(ptr{{ *%[^,]*}})

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; Function Attrs: nounwind uwtable
define dso_local void @foo() {
entry:
  %i = alloca i32, align 4
  %0 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  store i32 0, ptr %i, align 4, !tbaa !3
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr %i, align 4, !tbaa !3
  %cmp = icmp slt i32 %2, 200
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr %i, align 4, !tbaa !3
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %i, align 4, !tbaa !3
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  %4 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %4)
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)


!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2052, i32 85985733, !"foo", i32 3, i32 0, i32 0}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
