; REQUIRES: system-linux
; RUN: %lli -force-interpreter %s | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: %lli %s | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: 3.000000

; RUN: opt -passes="lower-subscript" -S %s -o - | FileCheck -check-prefix=CHECK-LOWER %s
; Lowering of 2 intrinsics

; icx -restrict -DEXECUTABLE -USIMPLE -std=c++11 -O3 llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp -emit-llvm -S  -o intel-subscript-arr.ll
; ModuleID = 'llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp'
source_filename = "llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ArrDesc = type { ptr, i64, i64, i64, i64, i64, [32 x %struct.DimDesc] }
%struct.A = type { float, i16 }
%struct.DimDesc = type { i64, i64, i64 }

@.str = private unnamed_addr constant [4 x i8] c"%f \00", align 1

; Function Attrs: nounwind uwtable
define void @_Z4testPK7ArrDescI1AEii(ptr noalias nocapture readonly %inout, i32 %N, i32 %K) local_unnamed_addr #0 {
; CHECK-LOWER-LABEL: @_Z4testPK7ArrDescI1AEii(
; First subscript. Load base, stride, lower bound.
; CHECK-LOWER:         [[BASE_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC:%.*]], ptr [[INOUT:%.*]], i64 0, i32 0
; CHECK-LOWER-NEXT:    [[TMP0:%.*]] = load ptr, ptr [[BASE_I]], align 8
; CHECK-LOWER-NEXT:    [[STRIDE_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 0, i32 1
; CHECK-LOWER-NEXT:    [[TMP1:%.*]] = load i64, ptr [[STRIDE_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV_I_I:%.*]] = trunc i64 [[TMP1]] to i32
; CHECK-LOWER-NEXT:    [[LB_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 0, i32 2
; CHECK-LOWER-NEXT:    [[TMP2:%.*]] = load i64, ptr [[LB_I_I]], align 8

; Compute offset in bytes, (index-lb)*stride.
; Apply byte offset to base pointer.
; CHECK-LOWER-NEXT:    [[CONV5_I_I:%.*]] = trunc i64 [[TMP2]] to i32
; CHECK-LOWER-NEXT:    [[TMP3:%.*]] = sub nsw i32 1, [[CONV5_I_I]]
; CHECK-LOWER-NEXT:    [[TMP4:%.*]] = sext i32 [[CONV_I_I]] to i64
; CHECK-LOWER-NEXT:    [[TMP5:%.*]] = sext i32 [[TMP3]] to i64
; CHECK-LOWER-NEXT:    [[TMP6:%.*]] = mul nsw i64 [[TMP4]], [[TMP5]]
; CHECK-LOWER-NEXT:    [[TMP8:%.*]] = getelementptr inbounds i8, ptr [[TMP0]], i64 [[TMP6]]
; Get the field address from the ptr.
; CHECK-LOWER-NEXT:    [[I5:%.*]] = getelementptr inbounds [[STRUCT_A:%.*]], ptr [[TMP8]], i64 0, i32 1

; 2nd subscript. Base is 1st subscript address. Same pattern as 1st subscript.
; CHECK-LOWER-NEXT:    [[TMP10:%.*]] = load i16, ptr [[I5]], align 4
; CHECK-LOWER-NEXT:    [[TMP11:%.*]] = sext i16 [[TMP10]] to i32
; CHECK-LOWER-NEXT:    [[TMP12:%.*]] = sub nsw i32 [[TMP11]], [[CONV5_I_I]]
; CHECK-LOWER-NEXT:    [[TMP13:%.*]] = sext i32 [[CONV_I_I]] to i64
; CHECK-LOWER-NEXT:    [[TMP14:%.*]] = sext i32 [[TMP12]] to i64
; CHECK-LOWER-NEXT:    [[TMP15:%.*]] = mul nsw i64 [[TMP13]], [[TMP14]]
; CHECK-LOWER-NEXT:    [[TMP17:%.*]] = getelementptr inbounds i8, ptr [[TMP0]], i64 [[TMP15]]
; CHECK-LOWER-NEXT:    [[F:%.*]] = getelementptr inbounds [[STRUCT_A]], ptr [[TMP17]], i64 0, i32 0

; Fianlly, do the load.
; CHECK-LOWER-NEXT:    [[TMP19:%.*]] = load float, ptr [[F]], align 4
; CHECK-LOWER-NEXT:    [[CONV10:%.*]] = fadd float [[TMP19]], 1.000000e+00
; CHECK-LOWER-NEXT:    store float [[CONV10]], ptr [[F]], align 4
; CHECK-LOWER-NEXT:    ret void
;
entry:
  %Base.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 0
  %0 = load ptr, ptr %Base.i, align 8
  %stride.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 0, i32 1
  %1 = load i64, ptr %stride.i.i, align 8
  %conv.i.i = trunc i64 %1 to i32
  %lb.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 0, i32 2
  %2 = load i64, ptr %lb.i.i, align 8
  %conv5.i.i = trunc i64 %2 to i32
  %call.i.i.i = tail call ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 %conv5.i.i, i32 %conv.i.i, ptr elementtype(%struct.A) %0, i16 signext 1) #5
  %i5 = getelementptr inbounds %struct.A, ptr %call.i.i.i, i64 0, i32 1
  %3 = load i16, ptr %i5, align 4
  %call.i.i.i19 = tail call ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 %conv5.i.i, i32 %conv.i.i, ptr elementtype(%struct.A) %0, i16 signext %3) #5
  %f = getelementptr inbounds %struct.A, ptr %call.i.i.i19, i64 0, i32 0
  %4 = load float, ptr %f, align 4
  %conv10 = fadd float %4, 1.000000e+00
  store float %conv10, ptr %f, align 4
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  %Arr = alloca [10 x %struct.A], align 16
  %0 = bitcast ptr %Arr to ptr
  call void @llvm.lifetime.start.p0(i64 80, ptr nonnull %0) #5
  %arrayidx = getelementptr inbounds [10 x %struct.A], ptr %Arr, i64 0, i64 0
  %i5 = getelementptr inbounds [10 x %struct.A], ptr %Arr, i64 0, i64 1, i32 1
  store i16 9, ptr %i5, align 4
  %f = getelementptr inbounds [10 x %struct.A], ptr %Arr, i64 0, i64 9, i32 0
  store float 2.000000e+00, ptr %f, align 8
  %call.i.i.i.i = call ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 0, i32 8, ptr elementtype(%struct.A) nonnull %arrayidx, i16 signext 1) #5
  %i5.i = getelementptr inbounds %struct.A, ptr %call.i.i.i.i, i64 0, i32 1
  %1 = load i16, ptr %i5.i, align 4
  %call.i.i.i19.i = call ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 0, i32 8, ptr elementtype(%struct.A) nonnull %arrayidx, i16 signext %1) #5
  %f.i = getelementptr inbounds %struct.A, ptr %call.i.i.i19.i, i64 0, i32 0
  %2 = load float, ptr %f.i, align 4
  %conv10.i = fadd float %2, 1.000000e+00
  store float %conv10.i, ptr %f.i, align 4
  %3 = load float, ptr %f, align 8
  %conv9 = fpext float %3 to double
  %call = tail call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([4 x i8], ptr @.str, i64 0, i64 0), double %conv9)
  call void @llvm.lifetime.end.p0(i64 80, ptr nonnull %0) #5
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8, i32, i32, ptr, i16) #4

attributes #0 = { uwtable  }
attributes #1 = { argmemonly nounwind }
attributes #2 = { uwtable  }
attributes #3 = { nounwind }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }
