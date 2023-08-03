; RUN: opt -passes=vplan-vec -disable-output -vplan-print-after-vpentity-instrs %s 2>&1 | FileCheck %s

; Test that we properly connect live-outs when a pointer induction has
; two live-outs (from the exit instruction and the header phi).
; Verifies fix for CMPLRLLVM-47026, 47028.

@ext1 = external global ptr
@ext2 = external global ptr

define ptr @foo(ptr %f2.init, ptr %tp2.init, ptr %t.ptr) {

; CHECK: VPlan after insertion of VPEntities instructions:

; Preheader
; CHECK: BB1:
; CHECK-NEXT: ptr [[INIT1:%.*]] = induction-init{getelementptr} ptr %f2.init i64 8
; CHECK-NEXT: i64 [[STEP1:%.*]] = induction-init-step{getelementptr} i64 8
; CHECK-NEXT: ptr [[INIT2:%.*]] = induction-init{getelementptr} ptr %tp2.init i64 8
; CHECK-NEXT: i64 [[STEP2:%.*]] = induction-init-step{getelementptr} i64 8

; Loop body
; CHECK: BB2:
; CHECK-NEXT: ptr [[PHI1:%.*]] = phi [ ptr [[GEP1:%.*]], BB2 ], [ ptr [[INIT1]], BB1 ]
; CHECK-NEXT: ptr [[PHI2:%.*]] = phi [ ptr [[GEP2:%.*]], BB2 ], [ ptr [[INIT2]], BB1 ]
; CHECK-NEXT: ptr [[GEP1]] = getelementptr inbounds i8, ptr [[PHI1]] i64 [[STEP1]]
; CHECK: ptr [[GEP2]] = getelementptr inbounds i8, ptr [[PHI2]] i64 [[STEP2]]

; Post-exit
; CHECK: BB3:
; CHECK-NEXT: ptr [[P1:%.*]] = induction-final{getelementptr} ptr %f2.init i64 8
; CHECK-NEXT: ptr [[P2:%.*]] = induction-final{getelementptr} ptr %tp2.init i64 8
; CHECK-NEXT: ptr [[P3:%.*]] = private-final-uc ptr [[PHI2]]

; CHECK: External Uses:
; CHECK-NEXT: Id: 0     %tp2.lcssa = phi ptr [ %tp2, %do.body ] ptr [[P3]] -> ptr %tp2;
; CHECK-EMPTY:
; CHECK-NEXT: Id: 1     %incdec.ptr75.lcssa = phi ptr [ %incdec.ptr75, %do.body ] ptr [[P1]] -> ptr %incdec.ptr75;
; CHECK-EMPTY:
; CHECK-NEXT: Id: 2     %incdec.ptr76.lcssa = phi ptr [ %incdec.ptr76, %do.body ] ptr [[P2]] -> ptr %incdec.ptr76;

entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %do.body

do.body:
  %f2 = phi ptr [ %incdec.ptr75, %do.body ], [ %f2.init, %entry ]
  %tp2 = phi ptr [ %incdec.ptr76, %do.body ], [ %tp2.init, %entry ]
  %incdec.ptr75 = getelementptr inbounds ptr, ptr %f2, i64 1
  %tmp = load ptr, ptr %f2, align 8
  %incdec.ptr76 = getelementptr inbounds ptr, ptr %tp2, i64 1
  store ptr %tmp, ptr %tp2, align 8
  %cmp77 = icmp ult ptr %incdec.ptr75, %t.ptr
  br i1 %cmp77, label %do.body, label %do.end

do.end:
  %tp2.lcssa = phi ptr [ %tp2, %do.body ]
  %incdec.ptr75.lcssa = phi ptr [ %incdec.ptr75, %do.body ]
  %incdec.ptr76.lcssa = phi ptr [ %incdec.ptr76, %do.body ]
  store ptr %incdec.ptr75.lcssa, ptr @ext1, align 8
  store ptr %incdec.ptr76.lcssa, ptr @ext2, align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret ptr %tp2.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
