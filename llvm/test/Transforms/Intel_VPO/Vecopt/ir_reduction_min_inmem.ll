;RUN: opt -VPlanDriver -S %s | FileCheck %s

; CHECK:    %Min.vec = alloca <8 x i32>
; CHECK:  vector.ph:                                        ; preds = %min.iters.checked
; CHECK:    %MinInitVal = load i32, i32* %Min
; CHECK:    %MinInitVec.splatinsert = insertelement <8 x i32> undef, i32 %MinInitVal, i32 0
; CHECK:    %MinInitVec.splat = shufflevector <8 x i32> %MinInitVec.splatinsert, <8 x i32> undef, <8 x i32> zeroinitializer
; CHECK:    store <8 x i32> %MinInitVec.splat, <8 x i32>* %Min.vec

; CHECK:  vector.body
; CHECK:  load{{.*}}%Min.vec
; CHECK:  masked.store{{.*}}%Min.vec

; CHECK:  middle.block:
; CHECK:    %Red.vec = load <8 x i32>, <8 x i32>* %Min.vec
; CHECK:    shufflevector <8 x i32> %Red.vec
; CHECK:    icmp slt <8 x i32>
; CHECK:    select <8 x i1> 
; CHECK:    shufflevector <8 x i32> 
; CHECK:    icmp slt <8 x i32> 
; CHECK:    select <8 x i1> 
; CHECK:    shufflevector <8 x i32> 
; CHECK:    icmp slt <8 x i32> 
; CHECK:    select <8 x i1> 
; CHECK:    %[[RES:.*]] = extractelement <8 x i32> 
; CHECK:    store i32 %[[RES]], i32* %Min
 

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@.str = private unnamed_addr addrspace(2) constant [10 x i8] c"Min = %d\0A\00", align 1

define void @reduction_min(i32 %thread_id_from_which_to_print_message) #0 {
entry:
  %A = alloca [19 x i32], align 4
  %Min = alloca i32, align 4
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  %conv = trunc i64 %call to i32
  %cmp = icmp eq i32 %conv, %thread_id_from_which_to_print_message
  br i1 %cmp, label %for.body.preheader, label %if.end18

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv4 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next5, %for.body ]
  %i.03 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %0 = trunc i64 %indvars.iv4 to i32
  %rem1 = and i32 %0, 1
  %tobool = icmp ne i32 %rem1, 0
  %indvars.iv.next5 = add nuw nsw i64 %indvars.iv4, 1
  %add = add nuw nsw i32 %i.03, 1
  %1 = sub nsw i64 9, %indvars.iv4
  %2 = trunc i64 %1 to i32
  %cond = select i1 %tobool, i32 %add, i32 %2
  %arrayidx = getelementptr inbounds [19 x i32], [19 x i32]* %A, i64 0, i64 %indvars.iv4
  store i32 %cond, i32* %arrayidx, align 4
  %exitcond7 = icmp ne i64 %indvars.iv.next5, 19
  br i1 %exitcond7, label %for.body, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %for.body
  store i32 0, i32* %Min, align 4
  br label %DIR.OMP.SIMD.28

DIR.OMP.SIMD.28:                                  ; preds = %DIR.OMP.SIMD.2
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 8)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.MIN", i32* nonnull %Min)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.28, %omp.inner.for.inc
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.28 ], [ %indvars.iv.next, %omp.inner.for.inc ]
  %arrayidx10 = getelementptr inbounds [19 x i32], [19 x i32]* %A, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx10, align 4
  %4 = load i32, i32* %Min, align 4
  %cmp11 = icmp slt i32 %3, %4
  br i1 %cmp11, label %if.then13, label %omp.inner.for.inc

if.then13:                                        ; preds = %omp.inner.for.body
  store i32 %3, i32* %Min, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then13, %omp.inner.for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 19
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.loop.exit
  %5 = load i32, i32* %Min, align 4
  %call17 = call i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* nonnull getelementptr inbounds ([10 x i8], [10 x i8] addrspace(2)* @.str, i64 0, i64 0), i32 %5) #2
  br label %if.end18

if.end18:                                         ; preds = %DIR.QUAL.LIST.END.1, %entry
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive(metadata) #1

declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32)

; Function Attrs: nounwind argmemonly
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: nounwind
declare i32 @printf(i8 addrspace(2)*, ...)
declare i64 @_Z13get_global_idj(i32)


