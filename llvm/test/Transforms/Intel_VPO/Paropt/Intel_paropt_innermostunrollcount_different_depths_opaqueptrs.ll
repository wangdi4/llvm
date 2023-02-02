; RUN: opt -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-apply-config,vpo-paropt-prepare)' -vpo-paropt-innermost-loop-unroll-count=50 -switch-to-offload -S %s | FileCheck %s
; Check that we apply the unroll count to multiple innermost loops at different levels
; Original code:
;int main()
;{
; #pragma omp target parallel for
; for (unsigned j = 0; j < 10; j++) {
;    for (unsigned i = 0; i < 5; ++i) {
;        for(unsigned p = 0; p < 100; p++) {}
;    }
;    for (unsigned k = 0; k < 500; k++) {}
;  }
;  return 0;
;}
;
; CHECK: store i32 {{.*}}, ptr addrspace(4) %[[P:p.ascast[0-9]+]], align 4
; CHECK: br label {{.*}}, !llvm.loop ![[PLoopMD:[0-9]+]]
; CHECK: store i32 {{.*}}, ptr addrspace(4) %[[P:k.ascast[0-9]+]], align 4
; CHECK: br label {{.*}}, !llvm.loop ![[KLoopMD:[0-9]+]]
; CHECK: ![[PLoopMD]] = distinct !{![[PLoopMD]], {{.*}}, ![[UnrollMD:[0-9]+]]}
; CHECK: ![[UnrollMD]] = !{!"llvm.loop.unroll.count", i32 50}
; CHECK: ![[KLoopMD]] = distinct !{![[KLoopMD]], {{.*}}, ![[UnrollMD]]}

source_filename = "test.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %j = alloca i32, align 4
  %i = alloca i32, align 4
  %p = alloca i32, align 4
  %k = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %p.ascast = addrspacecast ptr %p to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %p.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %p.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %add = add i32 %4, 1
  %cmp = icmp ult i32 %3, %add
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul i32 %5, 1
  %add1 = add i32 0, %mul
  store i32 %add1, ptr addrspace(4) %j.ascast, align 4
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %omp.inner.for.body
  %6 = load i32, ptr addrspace(4) %i.ascast, align 4
  %cmp2 = icmp ult i32 %6, 5
  br i1 %cmp2, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, ptr addrspace(4) %p.ascast, align 4
  br label %for.cond3

for.cond3:                                        ; preds = %for.inc, %for.body
  %7 = load i32, ptr addrspace(4) %p.ascast, align 4
  %cmp4 = icmp ult i32 %7, 100
  br i1 %cmp4, label %for.body5, label %for.end

for.body5:                                        ; preds = %for.cond3
  br label %for.inc

for.inc:                                          ; preds = %for.body5
  %8 = load i32, ptr addrspace(4) %p.ascast, align 4
  %inc = add i32 %8, 1
  store i32 %inc, ptr addrspace(4) %p.ascast, align 4
  br label %for.cond3, !llvm.loop !8

for.end:                                          ; preds = %for.cond3
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %9 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc7 = add i32 %9, 1
  store i32 %inc7, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond, !llvm.loop !10

for.end8:                                         ; preds = %for.cond
  store i32 0, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond9

for.cond9:                                        ; preds = %for.inc12, %for.end8
  %10 = load i32, ptr addrspace(4) %k.ascast, align 4
  %cmp10 = icmp ult i32 %10, 500
  br i1 %cmp10, label %for.body11, label %for.end14

for.body11:                                       ; preds = %for.cond9
  br label %for.inc12

for.inc12:                                        ; preds = %for.body11
  %11 = load i32, ptr addrspace(4) %k.ascast, align 4
  %inc13 = add i32 %11, 1
  store i32 %inc13, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond9, !llvm.loop !11

for.end14:                                        ; preds = %for.cond9
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end14
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add15 = add nuw i32 %12, 1
  store i32 %add15, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66312, i32 55446765, !"_Z4main", i32 3, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = distinct !{!11, !9}
