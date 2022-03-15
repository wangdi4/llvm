; RUN: opt < %s -enable-new-pm=0 -analyze -hir-region-identification -debug-only=hir-region-identification 2>&1 | FileCheck %s
; RUN: opt < %s -passes="print<hir-region-identification>" -debug-only=hir-region-identification 2>&1 | FileCheck %s

; CHECK: x86_amx type is not supported.

; CHECK-NOT: Region 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z14inner_product2PiS_S_iii(i32* noundef %A_mem, i32* noundef %B_mem, i64 %conv27, i64 %mul19, i64 %mul28, i64 %wide.trip.count246) {
entry:
  %t8 = tail call x86_amx @llvm.x86.tilezero.internal(i16 16, i16 64) #2
  br label %for.body14

for.body14:                                       ; preds = %for.body14, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body14 ]
  %c.sroa.8148.2.in196 = phi x86_amx [ %t8, %entry ], [ %t17, %for.body14 ]
  %t11 = shl nsw i64 %indvars.iv, 4
  %A_mem18 = getelementptr inbounds i32, i32* %A_mem, i64 %t11
  %t12 = bitcast i32* %A_mem18 to i8*
  %t13 = tail call x86_amx @llvm.x86.tileloadd64.internal(i16 16, i16 64, i8* %t12, i64 %mul19) #2
  %t14 = mul nsw i64 %t11, %conv27
  %A_mem26 = getelementptr inbounds i32, i32* %B_mem, i64 %t14
  %t15 = bitcast i32* %A_mem26 to i8*
  %t16 = tail call x86_amx @llvm.x86.tileloadd64.internal(i16 16, i16 64, i8* %t15, i64 %mul28) #2
  %t17 = tail call x86_amx @llvm.x86.tdpbf16ps.internal(i16 16, i16 64, i16 64, x86_amx %c.sroa.8148.2.in196, x86_amx %t13, x86_amx %t16) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count246
  br i1 %exitcond.not, label %for.cond.cleanup13.loopexit, label %for.body14

for.cond.cleanup13.loopexit:                      ; preds = %for.body14
  %.lcssa = phi x86_amx [ %t17, %for.body14 ]
  ret void
}

; Function Attrs: nounwind
declare x86_amx @llvm.x86.tilezero.internal(i16, i16) #2

; Function Attrs: nounwind
declare x86_amx @llvm.x86.tileloadd64.internal(i16, i16, i8*, i64) #2

; Function Attrs: nounwind
declare x86_amx @llvm.x86.tdpbf16ps.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx) #2

attributes #2 = { nounwind }

