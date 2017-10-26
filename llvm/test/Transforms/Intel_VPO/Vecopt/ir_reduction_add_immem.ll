;RUN: opt -VPlanDriver -S %s | FileCheck %s

; CHECK:   %Sum.vec = alloca <8 x i32>
; CHECK: vector.ph: 
; CHECK:   %SumInitVal = load i32, i32* %Sum
; CHECK:   %[[StartV:.*]] = insertelement <8 x i32> zeroinitializer, i32 %SumInitVal, i32 0
; CHECK:   store <8 x i32> %[[StartV]], <8 x i32>* %Sum.vec

; CHECK: vector.body:
; CHECK: %[[NEXT_V:.*]] = load
; CHECK: %[[CURRENT:.*]] = load <8 x i32>, <8 x i32>* %Sum.vec, align 4
; CHECK: %[[NEW_VAL:.*]] = add nsw <8 x i32> %[[CURRENT]], %[[NEXT_V]]
; CHECK: call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> %[[NEW_VAL]], <8 x i32>* %Sum.vec
  
; CHECK: middle.block:
; CHECK:   %Red.vec = load <8 x i32>, <8 x i32>* %Sum.vec
; CHECK:   shufflevector <8 x i32> %Red.vec
; CHECK:   add <8 x i32> 
; CHECK:   shufflevector <8 x i32>
; CHECK:   add <8 x i32>
; CHECK:   shufflevector <8 x i32>
; CHECK:   add <8 x i32> 
; CHECK:   %[[RES:.*]] = extractelement <8 x i32> %{{.*}}, i32 0
; CHECK:   store i32 %[[RES]], i32* %Sum

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@.str = private unnamed_addr addrspace(2) constant [10 x i8] c"Min = %d\0A\00", align 1

define void @reduction_add(i32 %thread_id_from_which_to_print_message) #0 {
entry:
  %Sum = alloca i32, align 4
  %A = alloca [1999 x i32], align 4
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5
  %conv = trunc i64 %call to i32
  %cmp = icmp eq i32 %conv, %thread_id_from_which_to_print_message
  br i1 %cmp, label %if.then, label %if.end18

if.then:                                          ; preds = %entry
  store i32 0, i32* %Sum, align 4
  br label %for.body

for.body:                                         ; preds = %if.then, %for.body
  %indvars.iv3 = phi i64 [ 0, %if.then ], [ %indvars.iv.next4, %for.body ]
  %0 = shl i64 %indvars.iv3, 1
  %1 = add nsw i64 %0, -100
  %arrayidx = getelementptr inbounds [1999 x i32], [1999 x i32]* %A, i64 0, i64 %indvars.iv3
  %2 = trunc i64 %1 to i32
  store i32 %2, i32* %arrayidx, align 4
  %indvars.iv.next4 = add nuw nsw i64 %indvars.iv3, 1
  %exitcond7 = icmp ne i64 %indvars.iv.next4, 1999
  br i1 %exitcond7, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 8)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.ADD", i32* nonnull %Sum)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %for.end, %omp.inner.for.inc
  %indvars.iv = phi i64 [ 0, %for.end ], [ %indvars.iv.next, %omp.inner.for.inc ]
  %arrayidx9 = getelementptr inbounds [1999 x i32], [1999 x i32]* %A, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx9, align 4
  %cmp10 = icmp slt i32 %3, 5
  br i1 %cmp10, label %if.then12, label %omp.inner.for.inc

if.then12:                                        ; preds = %omp.inner.for.body
  %4 = load i32, i32* %Sum, align 4
  %add15 = add nsw i32 %4, %3
  store i32 %add15, i32* %Sum, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then12, %omp.inner.for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1999
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.inc
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.loop.exit
  %5 = load i32, i32* %Sum, align 4
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


