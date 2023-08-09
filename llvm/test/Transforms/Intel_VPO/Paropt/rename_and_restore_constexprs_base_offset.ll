; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S -o %t1.ll <%s
; RUN: opt -bugpoint-enable-legacy-pm -early-cse -instcombine -vpo-restore-operands -S %t1.ll -o %t2.ll
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %t2.ll | FileCheck %s -check-prefix=TFORM

; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare)" -S -o %t1.ll <%s
; RUN: opt -passes="function(early-cse,instcombine,vpo-restore-operands)" -S %t1.ll -o %t2.ll
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %t2.ll | FileCheck %s -check-prefix=TFORM

; The test IR was hand-generated to simulate uses of constexprs offsets into an
; array as clause operands. A source program equivalent of the IR
; (not OpenMP spec compliant) would look something like this:
;
; char global[20];
;
; void bar(int in1, int in2);
; void wibble() {
;   #pragma omp parallel private(((char*)&global)[0:4]) private(((int*) &global[4])[0:1])
;   bar(*((int*) &global[4]), *((int*)&global));
; }

; INTEL_CUSTOMIZATION
; A Fortran example that would result in similar IR is:
;
; integer x, y
; common /global/ x, y
;
; print*, loc(x), loc(y)   ! p1 p2
;
; !$omp parallel private(x) private(y) num_threads(1)
;   print*, loc(x), loc(y) ! p3 p4 (should be different from p1, p2)
; !$omp end parallel
; end

; end INTEL_CUSTOMIZATION
; With opaque pointers, the zero-offset bitcast/GEP is a no-op, so the first operand
; just becomes "ptr @global", and the second operand is an offset into this same operand.
; This becomes a problem because replacement is done in order for operands
; of the same clause type. So when renaming the first operand, the uses of "@global"
; in the region will be replaced with its renamed value, which includes all its uses in
; in the const-expr representing the second operand.

; The passing scenario would have been for the two args of "bar" to be
; loaded from the two private locations corresponding to the private
; clause operands, but without the frontend change, both args are
; using the private copy corresponding to "ptr global", i.d. operand 1.

; TFORM:      define internal void @wibble.DIR.OMP.PARALLEL{{.*}}(ptr %tid, ptr %bid)
; TFORM:      [[OPND2_PRIV:%.+]] = alloca i32, align 4
; TFORM:      [[OPND1_PRIV:%.+]] = alloca [4 x i8], align 4
; TFORM:      %tmp2 = load i32, ptr [[OPND2_PRIV]], align 4
; TFORM:      %tmp3 = load i32, ptr [[OPND1_PRIV]], align 4
; TFORM:      call void @bar(i32 %tmp2, i32 %tmp3)

; Currently, tmp2 is loading from "%2 = getelementptr inbounds [20 x i8], ptr [[OPND1_PRIV]], i64 0, i64 4".
; XFAIL: *

; For this reason, we need the frontend to use Instructions instead of constant-exprs
; with opaque pointers. The expected IR from the frontend for this case would be:
;
;  %global.4 = getelementptr inbounds [20 x i8], ptr @global, i32 0, i32 4
;
;  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1),
;    "QUAL.OMP.PRIVATE:TYPED"(ptr %global.4, i32 0, i32 1) ]
;
;  %tmp2 = load i32, ptr %global.4, align 4
;  %tmp3 = load i32, ptr @global, align 4
;  call void @bar(i32 %tmp2, i32 %tmp3)
;

; Furthermore, vpo-restore-operands would need to be updated, so that it
; changes constant-expressions into instructions before restoring them as
; clause operands.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = common unnamed_addr global [20 x i8] zeroinitializer, align 4

declare void @bar(i32, i32)

define void @wibble() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %tmp1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @global, [4 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), i32 0, i32 1) ]

  %tmp2 = load i32, ptr getelementptr inbounds ([20 x i8], ptr @global, i32 0, i32 4), align 4
  %tmp3 = load i32, ptr @global, align 4
  call void @bar(i32 %tmp2, i32 %tmp3)

  call void @llvm.directive.region.exit(token %tmp1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
