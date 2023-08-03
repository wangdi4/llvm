; RUN: opt < %s -S -passes=vplan-pragma-omp-ordered-simd-extract,print -disable-output 2>&1 | FileCheck %s

; Check whether the value of i1 type liveout from extracted region is allocated as i8.
; i1 can't reside in memory, so allocate as i8, zext to i8 before storing and truncate
; back to i1 after reload before use,
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @var_tripcount(ptr %ip, i64 %n) local_unnamed_addr {
; CHECK-LABEL: @var_tripcount(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VAL_LOC:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[C_LOC:%.*]] = alloca i8, align 1
; CHECK-NEXT:    [[ENTRY_REGION:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr [[C_LOC]], i8 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr [[VAL_LOC]], i32 0, i32 1) ]
; CHECK:       codeRepl:
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 -1, ptr [[C_LOC]])
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 -1, ptr [[VAL_LOC]])
; CHECK-NEXT:    call void @var_tripcount.ordered.simd.region(ptr [[ARRAYIDX:%.*]], ptr [[C_LOC]], ptr [[VAL_LOC]])
; CHECK-NEXT:    [[C_RELOAD:%.*]] = load i8, ptr [[C_LOC]], align 1
; CHECK-NEXT:    [[TMP0:%.*]] = trunc i8 [[C_RELOAD]] to i1
; CHECK-NEXT:    [[VAL_RELOAD:%.*]] = load i32, ptr [[VAL_LOC]], align 4
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 -1, ptr [[C_LOC]])
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 -1, ptr [[VAL_LOC]])
; CHECK-NEXT:    br label [[PRE_LATCH:%.*]]
; CHECK:       pre.latch:
; CHECK-NEXT:    br i1 [[TMP0]], label [[LATCH:%.*]], label [[STORE_VAL:%.*]]
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %latch ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32, ptr %ip, i64 %indvars.iv
  br label %ordered.entry

ordered.entry:
  %tok.ordered = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.ORDERED.SIMD"() ]
  br label %ordered

ordered:
  %val1 = load i32, ptr %arrayidx
  %c = icmp sgt i32 %val1, 2
  br i1 %c, label %ordered.exit, label %inc.val

inc.val:
  %val2 = add i32 %val1, 4
  br label %ordered.exit

ordered.exit:
  %val = phi i32 [%val1, %ordered], [%val2, %inc.val]
  call void @llvm.directive.region.exit(token %tok.ordered) [ "DIR.OMP.END.ORDERED"() ]
  br label %pre.latch

pre.latch:
  br i1 %c, label %latch, label %store.val

store.val:
  store i32 %val, ptr %arrayidx, align 4
  br label %latch

latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  br label %for.cond.cleanup

for.cond.cleanup:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}
; Extracted region
; CHECK:       define internal void @var_tripcount.ordered.simd.region(
; CHECK-SAME        i32* %arrayidx, i8* %c.out, i32* %val.out) {
; CHECK:      newFuncRoot:
; CHECK-NEXT:   br label %ordered.entry
; CHECK:      ordered.entry:
; CHECK-NEXT:   %tok.ordered = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.ORDERED.SIMD"() ]
; CHECK-NEXT:   br label %ordered
; CHECK:      ordered:
; CHECK-NEXT:   %val1 = load i32, ptr %arrayidx, align 4
; CHECK-NEXT:   %c = icmp sgt i32 %val1, 2
; CHECK-NEXT:   [[TMP1:%.*]] = zext i1 %c to i8
; CHECK-NEXT:   store i8 [[TMP1]], ptr %c.out, align 1
; CHECK-NEXT:   br i1 %c, label %ordered.exit, label %inc.val
;

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
