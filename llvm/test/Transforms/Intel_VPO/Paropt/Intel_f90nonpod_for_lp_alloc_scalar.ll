; INTEL_CUSTOMIZATION
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; The test IR was reduced/hand modified from an original test containing
; something like:
;
; program test
;     type inner_t
;         integer                 :: inner_id
;         integer, allocatable    :: inner_stuff(:)
;     end type inner_t
;
;     type outer_t
;         integer                    :: outer_id
;         type(inner_t), allocatable :: stuff(:)
;     end type outer_t
;
;     type(outer_t), allocatable      :: collection
;     ...
;     !$omp do lastprivate(collection)
;       ...
;     !$omp end do
; end program test

; Check that constructor/copy-assign/destructor functions are called on the
; privatized allocatable scalar "collection".

; CHECK:  %"test_$COLLECTION.lpriv" = alloca ptr, align 8
; CHECK:  call void @"TEST$.btOUTER_T.omp.mold_ctor_deref"(ptr %"test_$COLLECTION.lpriv", ptr %"test_$COLLECTION")
; CHECK:  call void @"TEST$.btOUTER_T.omp.copy_assign_deref"(ptr %"test_$COLLECTION", ptr %"test_$COLLECTION.lpriv")
; CHECK:  call void @"TEST$.btOUTER_T.omp.dtor_deref"(ptr %"test_$COLLECTION.lpriv")

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @MAIN__() {
alloca_0:
  %"test_$I" = alloca i32, align 4
  %"test_$COLLECTION" = alloca ptr, align 8
  store ptr null, ptr %"test_$COLLECTION", align 1
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.lb = alloca i32, align 4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  store volatile i32 9, ptr %omp.pdo.norm.ub, align 4
  %"test_$I.addr" = alloca ptr, align 8
  %omp.pdo.norm.lb.addr = alloca ptr, align 8
  %"test_$COLLECTION.addr" = alloca ptr, align 8
  br label %bb_new5

omp.pdo.body8:                                    ; preds = %omp.pdo.body8.preheader, %omp.pdo.body8
  %omp.pdo.norm.iv_fetch.4 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
  store i32 %add.2, ptr %"test_$I1", align 4
  %"test_$COLLECTION_fetch.5" = load ptr, ptr %"test_$COLLECTION3", align 8
  %omp.pdo.norm.iv_fetch.6 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %add.3 = add nsw i32 %omp.pdo.norm.iv_fetch.6, 1
  store volatile i32 %add.3, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.iv_fetch.7 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.8 = load volatile i32, ptr %omp.pdo.norm.ub, align 4
  %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.7, %omp.pdo.norm.ub_fetch.8
  br i1 %rel.2, label %omp.pdo.body8, label %omp.pdo.epilog9.loopexit

bb_new5:                                          ; preds = %alloca_0
  store ptr %"test_$I", ptr %"test_$I.addr", align 8
  store ptr %omp.pdo.norm.lb, ptr %omp.pdo.norm.lb.addr, align 8
  store ptr %"test_$COLLECTION", ptr %"test_$COLLECTION.addr", align 8
  br label %bb_new5.split

bb_new5.split:                                    ; preds = %bb_new5
  %end.dir.temp = alloca i1, align 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"test_$I", i32 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i64 1),
    "QUAL.OMP.LASTPRIVATE:F90_NONPOD.TYPED"(ptr %"test_$COLLECTION", ptr null, i64 1, ptr @"TEST$.btOUTER_T.omp.mold_ctor_deref", ptr @"TEST$.btOUTER_T.omp.copy_assign_deref", ptr @"TEST$.btOUTER_T.omp.dtor_deref"),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.OPERAND.ADDR"(ptr %"test_$I", ptr %"test_$I.addr"),
    "QUAL.OMP.OPERAND.ADDR"(ptr %omp.pdo.norm.lb, ptr %omp.pdo.norm.lb.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr %"test_$COLLECTION", ptr %"test_$COLLECTION.addr"),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  %cmp = icmp ne i1 %temp.load, false
  br i1 %cmp, label %DIR.OMP.END.LOOP.2.split, label %1

1:                                                ; preds = %bb_new5.split
  %"test_$I1" = load volatile ptr, ptr %"test_$I.addr", align 8
  %omp.pdo.norm.lb2 = load volatile ptr, ptr %omp.pdo.norm.lb.addr, align 8
  %"test_$COLLECTION3" = load volatile ptr, ptr %"test_$COLLECTION.addr", align 8
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %1
  %omp.pdo.norm.lb_fetch.1 = load i32, ptr %omp.pdo.norm.lb2, align 4
  store volatile i32 %omp.pdo.norm.lb_fetch.1, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.iv_fetch.2 = load volatile i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.3 = load volatile i32, ptr %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
  br i1 %rel.1, label %omp.pdo.body8.preheader, label %omp.pdo.epilog9

omp.pdo.body8.preheader:                          ; preds = %DIR.OMP.LOOP.1
  br label %omp.pdo.body8

omp.pdo.epilog9.loopexit:                         ; preds = %omp.pdo.body8
  br label %omp.pdo.epilog9

omp.pdo.epilog9:                                  ; preds = %omp.pdo.epilog9.loopexit, %DIR.OMP.LOOP.1
  br label %DIR.OMP.END.LOOP.2

DIR.OMP.END.LOOP.2:                               ; preds = %omp.pdo.epilog9
  br label %DIR.OMP.END.LOOP.2.split

DIR.OMP.END.LOOP.2.split:                         ; preds = %bb_new5.split, %DIR.OMP.END.LOOP.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]

  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %DIR.OMP.END.LOOP.2.split
  ret void
}

declare token @llvm.directive.region.entry()

declare void @"TEST$.btOUTER_T.omp.mold_ctor_deref"(ptr %dst, ptr %src)

declare void @"TEST$.btOUTER_T.omp.copy_assign_deref"(ptr %dst, ptr %src)

declare void @"TEST$.btOUTER_T.omp.dtor_deref"(ptr %old)

declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
