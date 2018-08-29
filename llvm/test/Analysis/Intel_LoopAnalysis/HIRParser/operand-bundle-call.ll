; RUN: opt < %s -hir-ssa-deconstruction -analyze -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -simplifycfg -mem2reg -force-hir-cg -S 2>&1 | FileCheck %s --check-prefix=CHECK-CG
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-cg,simplify-cfg,mem2reg" -force-hir-cg -S 2>&1 | FileCheck %s --check-prefix=CHECK-CG

; Verify that hir framework is able to parse calls with operand bundles and correctly generate code for them.

; CHECK: %0 = @llvm.directive.region.entry(); [ DIR.OMP.PARALLEL.LOOP(),  QUAL.OMP.PRIVATE(&((%i)[0])),  QUAL.OMP.SHARED(&((%ip.addr)[0])) ]
; CHECK: %2 = (%ip.addr)[0];

; CHECK: + DO i1 = 0, zext.i32.i64(trunc.i64.i32(%n)) + -1, 1   <DO_LOOP>
; CHECK: |   (%2)[i1] = i1;
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%0); [ DIR.OMP.END.PARALLEL.LOOP() ]

; Checks that we are able to recover the calls with the same parameters after code generation and cleanup.

; CHECK-CG: [[TOKEN:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32** %ip.addr) ]
; CHECK-CG: call void @llvm.directive.region.exit(token [[TOKEN]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

source_filename = "csa_offload.cpp"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"
target device_triples = "csa"

; Function Attrs: norecurse nounwind
define weak void @__omp_offloading_8098d6ff_765331__Z5loop1Pii_l20(i64 %n, i32* %ip) {
entry:
  %ip.addr = alloca i32*, align 8
  %i = alloca i32, align 4
  %n.addr.sroa.0.0.extract.trunc = trunc i64 %n to i32
  store i32* %ip, i32** %ip.addr, align 8
  %cmp = icmp sgt i32 %n.addr.sroa.0.0.extract.trunc, 0
  br i1 %cmp, label %DIR.OMP.PARALLEL.LOOP.112, label %omp.precond.end

DIR.OMP.PARALLEL.LOOP.112:                        ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32** %ip.addr) ]
  %1 = bitcast i32* %i to i8*
  %2 = load i32*, i32** %ip.addr, align 8
  %wide.trip.count = and i64 %n, 4294967295
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.112
  %indvars.iv = phi i64 [ 0, %DIR.OMP.PARALLEL.LOOP.112 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %3 = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds i32, i32* %2, i64 %indvars.iv
  store i32 %3, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)



