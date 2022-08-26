; Test if debug information for PrivDescr class are generated correctly.

; RUN: opt -S -vplan-vec -disable-output -vplan-print-legality -vplan-print-after-plain-cfg -vplan-entities-dump < %s 2>&1 | FileCheck %s -check-prefix=LLVMIR
; RUN: opt -S -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -vplan-print-legality -vplan-print-after-plain-cfg -vplan-entities-dump -disable-vplan-codegen < %s 2>&1 | FileCheck %s -check-prefix=HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point2d = type { i32, i32 }

; Function Attrs: nounwind uwtable
define dso_local i32 @test_debug_info(%struct.point2d* nocapture readonly %src, %struct.point2d* nocapture %dst) {
; LLVMIR:       VPOLegality PrivateList:
; LLVMIR-NEXT:  Ref:   [[MYPOINT2_PRIV0:%.*]] = alloca [[STRUCT_POINT2D0:%.*]], align 4
; LLVMIR-EMPTY:
; LLVMIR-NEXT:    UpdateInstructions:
; LLVMIR-NEXT:    none
; LLVMIR-NEXT:  PrivDescr: {IsCond: 0, IsLast: 0, Type: [[STRUCT_POINT2D0]] = type { i32, i32 }}
; LLVMIR-NEXT:  PrivDescrNonPOD: {Ctor: _ZTS7point2d.omp.def_constr, Dtor: _ZTS7point2d.omp.destr, Copy Assign: }

; LLVMIR:       Private list
; LLVMIR-EMPTY:
; LLVMIR-NEXT:    Private tag: Non-POD
; LLVMIR-NEXT:    Linked values: %struct.point2d* [[MYPOINT2_PRIV0]],
; LLVMIR-NEXT:   Memory: %struct.point2d* [[MYPOINT2_PRIV0]]

; HIR:       HIRLegality PrivatesNonPODList:
; HIR-NEXT:  Ref: &(([[MYPOINT2_PRIV0:%.*]])[0])
; HIR-NEXT:    UpdateInstructions:
; HIR-NEXT:    none
; HIR-NEXT:  PrivDescr: {IsCond: 0, IsLast: 0, Type: [[STRUCT_POINT2D0:%.*]] = type { i32, i32 }}
; HIR-NEXT:  PrivDescrNonPOD: {Ctor: _ZTS7point2d.omp.def_constr, Dtor: _ZTS7point2d.omp.destr, Copy Assign: }

; HIR:       Private list
; HIR-EMPTY:
; HIR-NEXT:    Private tag: Non-POD
; HIR-NEXT:    Linked values: %struct.point2d* [[MYPOINT2_PRIV0]],
; HIR-NEXT:   Memory: %struct.point2d* [[MYPOINT2_PRIV0]]
;
omp.inner.for.body.lr.ph:
  %i.linear.iv = alloca i32, align 4
  %myPoint2.priv = alloca %struct.point2d, align 4
  %0 = call %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* nonnull %myPoint2.priv)
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.point2d* %myPoint2.priv, %struct.point2d* (%struct.point2d*)* @_ZTS7point2d.omp.def_constr, void (%struct.point2d*)* @_ZTS7point2d.omp.destr), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %y = getelementptr inbounds %struct.point2d, %struct.point2d* %myPoint2.priv, i64 0, i32 1
  %2 = load i32, i32* %y, align 4
  %x2 = getelementptr inbounds %struct.point2d, %struct.point2d* %myPoint2.priv, i64 0, i32 0
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %3 = trunc i64 %indvars.iv to i32
  %add1 = add nsw i32 %2, %3
  %x3 = getelementptr inbounds %struct.point2d, %struct.point2d* %src, i64 %indvars.iv, i32 0
  %4 = load i32, i32* %x3, align 4
  %add5 = add nsw i32 %4, %add1
  %x8 = getelementptr inbounds %struct.point2d, %struct.point2d* %dst, i64 %indvars.iv, i32 0
  store i32 %add5, i32* %x8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  %5 = add i32 %2, 999
  store i32 %5, i32* %x2, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %_ZN7point2dD2Ev.exit

_ZN7point2dD2Ev.exit:                             ; preds = %DIR.OMP.END.SIMD.3
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nofree norecurse nounwind uwtable writeonly
declare %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* returned %0)

; Function Attrs: nounwind uwtable
declare void @_ZTS7point2d.omp.destr(%struct.point2d* nocapture %0)
