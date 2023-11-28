; REQUIRES: system-linux
; RUN: %lli -force-interpreter %s | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: %lli %s | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: 3.000000
; RUN: opt -passes="lower-subscript" -S %s -o - | FileCheck -check-prefix=CHECK-LOWER %s
; Lowering of 3 intrinsics

; icx -restrict -DEXECUTABLE -DSIMPLE -std=c++11 -O3 llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp -emit-llvm -S  -o intel-subscript-arr.ll
; ModuleID = 'llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp'
source_filename = "llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ArrDesc = type { ptr, i64, i64, i64, i64, i64, [32 x %struct.DimDesc] }
%struct.DimDesc = type { i64, i64, i64 }

@.str = private unnamed_addr constant [4 x i8] c"%f \00", align 1

; Function Attrs: uwtable
define void @_Z4testPK7ArrDescIdEii(ptr noalias nocapture readonly %inout, i32 %N, i32 %K) local_unnamed_addr #0 {
; CHECK-LOWER-LABEL: @_Z4testPK7ArrDescIdEii(
; CHECK-LOWER-NEXT:  entry:
; First subscript.
; Load base, stride, and lower bound from array descriptor.
; CHECK-LOWER-NEXT:    [[BASE_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC:%.*]], ptr [[INOUT:%.*]], i64 0, i32 0
; CHECK-LOWER-NEXT:    [[TMP0:%.*]] = load ptr, ptr [[BASE_I]], align 8
; CHECK-LOWER-NEXT:    [[STRIDE_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 2, i32 1
; CHECK-LOWER-NEXT:    [[TMP1:%.*]] = load i64, ptr [[STRIDE_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV_I_I:%.*]] = trunc i64 [[TMP1]] to i32
; CHECK-LOWER-NEXT:    [[LB_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 2, i32 2
; CHECK-LOWER-NEXT:    [[TMP2:%.*]] = load i64, ptr [[LB_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV3_I_I:%.*]] = trunc i64 [[TMP2]] to i32

; Compute the offset in bytes.
; CHECK-LOWER-NEXT:    [[TMP3:%.*]] = sub nsw i32 3, [[CONV3_I_I]]
; CHECK-LOWER-NEXT:    [[TMP4:%.*]] = sext i32 [[CONV_I_I]] to i64
; CHECK-LOWER-NEXT:    [[TMP5:%.*]] = sext i32 [[TMP3]] to i64
; CHECK-LOWER-NEXT:    [[TMP6:%.*]] = mul nsw i64 [[TMP4]], [[TMP5]]

; Apply byte offset to base pointer.
; CHECK-LOWER-NEXT:    [[TMP8:%.*]] = getelementptr inbounds i8, ptr [[TMP0]], i64 [[TMP6]]

; 2nd subscript. Load stride and LB. The base is the previous computed
; subscript address.
; CHECK-LOWER-NEXT:    [[STRIDE_I_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 1, i32 1
; CHECK-LOWER-NEXT:    [[TMP10:%.*]] = load i64, ptr [[STRIDE_I_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV_I_I_I:%.*]] = trunc i64 [[TMP10]] to i32
; CHECK-LOWER-NEXT:    [[LB_I_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 1, i32 2
; CHECK-LOWER-NEXT:    [[TMP11:%.*]] = load i64, ptr [[LB_I_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV3_I_I_I:%.*]] = trunc i64 [[TMP11]] to i32

; Compute byte offset, apply it to base (1st subscript address).
; CHECK-LOWER-NEXT:    [[TMP12:%.*]] = sub nsw i32 0, [[CONV3_I_I_I]]
; CHECK-LOWER-NEXT:    [[TMP13:%.*]] = sext i32 [[CONV_I_I_I]] to i64
; CHECK-LOWER-NEXT:    [[TMP14:%.*]] = sext i32 [[TMP12]] to i64
; CHECK-LOWER-NEXT:    [[TMP15:%.*]] = mul nsw i64 [[TMP13]], [[TMP14]]
; CHECK-LOWER-NEXT:    [[TMP17:%.*]] = getelementptr inbounds i8, ptr [[TMP8]], i64 [[TMP15]]

; 3rd subscript. Base is address computed from 2nd subscript.
; CHECK-LOWER-NEXT:    [[STRIDE_I_I_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 0, i32 1
; CHECK-LOWER-NEXT:    [[TMP19:%.*]] = load i64, ptr [[STRIDE_I_I_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV_I_I_I_I:%.*]] = trunc i64 [[TMP19]] to i32
; CHECK-LOWER-NEXT:    [[LB_I_I_I_I:%.*]] = getelementptr inbounds [[STRUCT_ARRDESC]], ptr [[INOUT]], i64 0, i32 6, i64 0, i32 2
; CHECK-LOWER-NEXT:    [[TMP20:%.*]] = load i64, ptr [[LB_I_I_I_I]], align 8
; CHECK-LOWER-NEXT:    [[CONV5_I_I_I_I:%.*]] = trunc i64 [[TMP20]] to i32
; CHECK-LOWER-NEXT:    [[TMP21:%.*]] = sub nsw i32 1, [[CONV5_I_I_I_I]]
; CHECK-LOWER-NEXT:    [[TMP22:%.*]] = sext i32 [[CONV_I_I_I_I]] to i64
; CHECK-LOWER-NEXT:    [[TMP23:%.*]] = sext i32 [[TMP21]] to i64
; CHECK-LOWER-NEXT:    [[TMP24:%.*]] = mul nsw i64 [[TMP22]], [[TMP23]]
; CHECK-LOWER-NEXT:    [[TMP26:%.*]] = getelementptr inbounds i8, ptr [[TMP17]], i64 [[TMP24]]

; Finally, do the load and fadd from the final address.
; CHECK-LOWER-NEXT:    [[TMP28:%.*]] = load double, ptr [[TMP26]], align 8
; CHECK-LOWER-NEXT:    [[ADD4:%.*]] = fadd double [[TMP28]], 1.000000e+00
; CHECK-LOWER-NEXT:    store double [[ADD4]], ptr [[TMP26]], align 8
; CHECK-LOWER-NEXT:    ret void
;
entry:
  %Base.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 0
  %0 = load ptr, ptr %Base.i, align 8
  %stride.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 2, i32 1
  %1 = load i64, ptr %stride.i.i, align 8
  %conv.i.i = trunc i64 %1 to i32
  %lb.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 2, i32 2
  %2 = load i64, ptr %lb.i.i, align 8
  %conv3.i.i = trunc i64 %2 to i32
  %call.i.i.i = tail call ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8 zeroext 2, i32 %conv3.i.i, i32 %conv.i.i, ptr elementtype(double) %0, i32 3) #5
  %stride.i.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 1, i32 1
  %3 = load i64, ptr %stride.i.i.i, align 8
  %conv.i.i.i = trunc i64 %3 to i32
  %lb.i.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 1, i32 2
  %4 = load i64, ptr %lb.i.i.i, align 8
  %conv3.i.i.i = trunc i64 %4 to i32
  %call.i.i.i.i = tail call ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8 zeroext 1, i32 %conv3.i.i.i, i32 %conv.i.i.i, ptr elementtype(double) %call.i.i.i, i32 0) #5
  %stride.i.i.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 0, i32 1
  %5 = load i64, ptr %stride.i.i.i.i, align 8
  %conv.i.i.i.i = trunc i64 %5 to i32
  %lb.i.i.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 0, i32 2
  %6 = load i64, ptr %lb.i.i.i.i, align 8
  %conv5.i.i.i.i = trunc i64 %6 to i32
  %call.i.i.i.i.i = tail call ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8 zeroext 0, i32 %conv5.i.i.i.i, i32 %conv.i.i.i.i, ptr elementtype(double) %call.i.i.i.i, i32 1) #5
  %7 = load double, ptr %call.i.i.i.i.i, align 8
  %add4 = fadd double %7, 1.000000e+00
  store double %add4, ptr %call.i.i.i.i.i, align 8
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  %A = alloca [10 x [10 x [10 x double]]], align 16
  %0 = bitcast ptr %A to ptr
  call void @llvm.lifetime.start.p0(i64 8000, ptr nonnull %0) #5
  %arrayidx2 = getelementptr inbounds [10 x [10 x [10 x double]]], ptr %A, i64 0, i64 0, i64 0, i64 0
  %arrayidx21 = getelementptr inbounds [10 x [10 x [10 x double]]], ptr %A, i64 0, i64 3, i64 0, i64 1
  store double 2.000000e+00, ptr %arrayidx21, align 8
  %call.i.i.i.i = call ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8 zeroext 2, i32 0, i32 800, ptr elementtype(double) nonnull %arrayidx2, i32 3) #5
  %call.i.i.i.i.i = call ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8 zeroext 1, i32 0, i32 80, ptr elementtype(double) %call.i.i.i.i, i32 0) #5
  %call.i.i.i.i.i.i = call ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8 zeroext 0, i32 0, i32 8, ptr elementtype(double) %call.i.i.i.i.i, i32 1) #5
  %1 = load double, ptr %call.i.i.i.i.i.i, align 8
  %add4.i = fadd double %1, 1.000000e+00
  store double %add4.i, ptr %call.i.i.i.i.i.i, align 8
  %2 = load double, ptr %arrayidx21, align 8
  %call = tail call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([4 x i8], ptr @.str, i64 0, i64 0), double %2)
  call void @llvm.lifetime.end.p0(i64 8000, ptr nonnull %0) #5
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i32.i32.p0.i32(i8, i32, i32, ptr, i32) #4

attributes #0 = { uwtable}
attributes #1 = { argmemonly nounwind }
attributes #2 = { uwtable}
attributes #3 = { nounwind }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }
