; REQUIRES: system-linux
; RUN: %lli -force-interpreter %s | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: %lli %s | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: 3.000000
; RUN: opt -S -lower-subscript %s -o - | FileCheck -check-prefix=CHECK-LOWER %s
; Lowering of 3 intrinsics

; CHECK-LOWER:       %{{.*}} = sdiv exact i32 %conv.i.i, 8
; CHECK-LOWER-NEXT:  %{{.*}} = sub nsw i32 3, %conv3.i.i
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = mul nsw i64
; CHECK-LOWER-NEXT:  %{{.*}} = getelementptr inbounds double, double* %{{.*}}, i64

; CHECK-LOWER:       %{{.*}} = sdiv exact i32 %conv.i.i.i, 8
; CHECK-LOWER-NEXT:  %{{.*}} = sub nsw i32 0, %conv3.i.i.i
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = mul nsw i64
; CHECK-LOWER-NEXT:  %{{.*}} = getelementptr inbounds double, double* %{{.*}}, i64

; CHECK-LOWER:       %{{.*}} = sdiv exact i32 %conv.i.i.i.i, 8
; CHECK-LOWER-NEXT:  %{{.*}} = sub nsw i32 1, %conv5.i.i.i.i
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = mul nsw i64
; CHECK-LOWER-NEXT:  %{{.*}} = getelementptr inbounds double, double* %{{.*}}, i64
; CHECK-LOWER-NEXT:  %{{.*}} = load double, double* %{{.*}}, align 8
; CHECK-LOWER-NEXT:  %add4 = fadd double %{{.*}}, 1.000000e+00
; CHECK-LOWER-NEXT:  store double %add4, double* %{{.*}}, align 8

; icx -restrict -DEXECUTABLE -DSIMPLE -std=c++11 -O3 llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp -emit-llvm -S  -o intel-subscript-arr.ll
; ModuleID = 'llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp'
source_filename = "llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ArrDesc = type { double*, i64, i64, i64, i64, i64, [32 x %struct.DimDesc] }
%struct.DimDesc = type { i64, i64, i64 }

@.str = private unnamed_addr constant [4 x i8] c"%f \00", align 1

; Function Attrs: uwtable
define void @_Z4testPK7ArrDescIdEii(%struct.ArrDesc* noalias nocapture readonly %inout, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %Base.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 0
  %0 = load double*, double** %Base.i, align 8
  %stride.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 2, i32 1
  %1 = load i64, i64* %stride.i.i, align 8
  %conv.i.i = trunc i64 %1 to i32
  %lb.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 2, i32 2
  %2 = load i64, i64* %lb.i.i, align 8
  %conv3.i.i = trunc i64 %2 to i32
  %call.i.i.i = tail call double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8 zeroext 2, i32 %conv3.i.i, i32 %conv.i.i, double* %0, i32 3) #5
  %stride.i.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 1, i32 1
  %3 = load i64, i64* %stride.i.i.i, align 8
  %conv.i.i.i = trunc i64 %3 to i32
  %lb.i.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 1, i32 2
  %4 = load i64, i64* %lb.i.i.i, align 8
  %conv3.i.i.i = trunc i64 %4 to i32
  %call.i.i.i.i = tail call double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8 zeroext 1, i32 %conv3.i.i.i, i32 %conv.i.i.i, double* %call.i.i.i, i32 0) #5
  %stride.i.i.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 0, i32 1
  %5 = load i64, i64* %stride.i.i.i.i, align 8
  %conv.i.i.i.i = trunc i64 %5 to i32
  %lb.i.i.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 0, i32 2
  %6 = load i64, i64* %lb.i.i.i.i, align 8
  %conv5.i.i.i.i = trunc i64 %6 to i32
  %call.i.i.i.i.i = tail call double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8 zeroext 0, i32 %conv5.i.i.i.i, i32 %conv.i.i.i.i, double* %call.i.i.i.i, i32 1) #5
  %7 = load double, double* %call.i.i.i.i.i, align 8
  %add4 = fadd double %7, 1.000000e+00
  store double %add4, double* %call.i.i.i.i.i, align 8
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  %A = alloca [10 x [10 x [10 x double]]], align 16
  %0 = bitcast [10 x [10 x [10 x double]]]* %A to i8*
  call void @llvm.lifetime.start.p0i8(i64 8000, i8* nonnull %0) #5
  %arrayidx2 = getelementptr inbounds [10 x [10 x [10 x double]]], [10 x [10 x [10 x double]]]* %A, i64 0, i64 0, i64 0, i64 0
  %arrayidx21 = getelementptr inbounds [10 x [10 x [10 x double]]], [10 x [10 x [10 x double]]]* %A, i64 0, i64 3, i64 0, i64 1
  store double 2.000000e+00, double* %arrayidx21, align 8
  %call.i.i.i.i = call double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8 zeroext 2, i32 0, i32 800, double* nonnull %arrayidx2, i32 3) #5
  %call.i.i.i.i.i = call double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8 zeroext 1, i32 0, i32 80, double* %call.i.i.i.i, i32 0) #5
  %call.i.i.i.i.i.i = call double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8 zeroext 0, i32 0, i32 8, double* %call.i.i.i.i.i, i32 1) #5
  %1 = load double, double* %call.i.i.i.i.i.i, align 8
  %add4.i = fadd double %1, 1.000000e+00
  store double %add4.i, double* %call.i.i.i.i.i.i, align 8
  %2 = load double, double* %arrayidx21, align 8
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %2)
  call void @llvm.lifetime.end.p0i8(i64 8000, i8* nonnull %0) #5
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i32.i32.p0f64.i32(i8, i32, i32, double*, i32) #4

attributes #0 = { uwtable}
attributes #1 = { argmemonly nounwind }
attributes #2 = { uwtable}
attributes #3 = { nounwind }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }
