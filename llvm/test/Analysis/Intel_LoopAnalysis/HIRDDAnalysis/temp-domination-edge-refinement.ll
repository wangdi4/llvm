; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; Verify that the flow edge (5:14 %2 --> %2 FLOW) is not formed as %2 on <5> is killed by %2 on <13>.

; CHECK-NOT: 5:14 %2 --> %2 FLOW

; CHECK: 13:14 %2 --> %2 FLOW (=)

; CHECK-NOT: 5:14 %2 --> %2 FLOW

; HIR-
; <2>                %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR(&((%ptr.addr)[0])1),  QUAL.OMP.REDUCTION.ADD(&((%s)[0])) ]
; <3>                %ptr.addr.promoted = (%ptr.addr)[0];
; <4>                %.pre = (%s)[0];
; <5>                %2 = %.pre;
; <27>
; <27>               + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <simd>
; <10>               |   %3 = (%ptr.addr.promoted)[i1];
; <13>               |   %2 = (sext.i8.i32(%inc18) * %3)  +  %2;
; <14>               |   (%s)[0] = %2;
; <15>               |   %incdec.ptr = &((%ptr.addr.promoted)[i1 + 1]);
; <27>               + END LOOP
; <27>
; <24>               (%ptr.addr)[0] = &((%incdec.ptr)[0]);
; <25>               @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3fooPiii(i32* %ptr, i32 %step, i32 %n, i8 %inc18) {
entry:
  %ptr.addr = alloca i32*, align 8
  %s = alloca i32, align 4
  store i32* %ptr, i32** %ptr.addr, align 8
  %0 = bitcast i32* %s to i8*
  store i32 0, i32* %s, align 4
  %conv = trunc i32 %step to i8
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.114, label %omp.precond.end

DIR.OMP.SIMD.114:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32** %ptr.addr, i32 1), "QUAL.OMP.REDUCTION.ADD"(i32* %s) ]
  %ptr.addr.promoted = load i32*, i32** %ptr.addr, align 8
  %.pre = load i32, i32* %s, align 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.114
  %2 = phi i32 [ %add8, %omp.inner.for.body ], [ %.pre, %DIR.OMP.SIMD.114 ]
  %incdec.ptr17 = phi i32* [ %incdec.ptr, %omp.inner.for.body ], [ %ptr.addr.promoted, %DIR.OMP.SIMD.114 ]
  %.omp.iv.0 = phi i32 [ %add9, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.114 ]
  %3 = load i32, i32* %incdec.ptr17, align 4
  %conv6 = sext i8 %inc18 to i32
  %mul7 = mul nsw i32 %3, %conv6
  %add8 = add nsw i32 %mul7, %2
  store i32 %add8, i32* %s, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr17, i64 1
  %add9 = add nuw nsw i32 %.omp.iv.0, 1
  %exitcond = icmp eq i32 %add9, %n
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  %incdec.ptr.lcssa = phi i32* [ %incdec.ptr, %omp.inner.for.body ]
  store i32* %incdec.ptr.lcssa, i32** %ptr.addr, align 8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  %.pre19 = load i32, i32* %s, align 4
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %4 = phi i32 [ %.pre19, %omp.loop.exit ], [ 0, %entry ]
  ret i32 %4
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)


