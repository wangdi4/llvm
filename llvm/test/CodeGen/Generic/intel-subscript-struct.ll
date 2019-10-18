; REQUIRES: system-linux
; RUN: %lli -force-interpreter %s | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: %lli %s | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: 3.000000

; RUN: opt -S -lower-subscript %s -o - | FileCheck -check-prefix=CHECK-LOWER %s
; Lowering of 2 intrinsics

; CHECK-LOWER:      %{{.*}} = sdiv exact i32 %conv.i.i, 8
; CHECK-LOWER-NEXT: %{{.*}} = sub nsw i32 1, %conv5.i.i
; CHECK-LOWER-NEXT: %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT: %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT: %{{.*}} = mul nsw i64
; CHECK-LOWER-NEXT: %{{.*}} = getelementptr inbounds %struct.A, %struct.A* %{{.*}}, i64
; CHECK-LOWER-NEXT: %i5 = getelementptr inbounds %struct.A, %struct.A* %{{.*}}, i64 0, i32 1
; CHECK-LOWER-NEXT: %{{.*}} = load i16, i16* %i5, align 4

; CHECK-LOWER:       %{{.*}} = sdiv exact i32 %conv.i.i, 8
; CHECK-LOWER-NEXT:  %{{.*}} = sext i16 %{{.*}} to i32
; CHECK-LOWER-NEXT:  %{{.*}} = sub nsw i32 %{{.*}}, %conv5.i.i
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = sext i32 %{{.*}} to i64
; CHECK-LOWER-NEXT:  %{{.*}} = mul nsw i64
; CHECK-LOWER-NEXT:  %{{.*}} = getelementptr inbounds %struct.A, %struct.A* %0, i64
; CHECK-LOWER-NEXT:  %f = getelementptr inbounds %struct.A, %struct.A* %{{.*}}, i64 0, i32 0
; CHECK-LOWER-NEXT:  %15 = load float, float* %f, align 4
; CHECK-LOWER-NEXT:  %conv10 = fadd float %15, 1.000000e+00
; CHECK-LOWER-NEXT:  store float %conv10, float* %f, align 4

; icx -restrict -DEXECUTABLE -USIMPLE -std=c++11 -O3 llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp -emit-llvm -S  -o intel-subscript-arr.ll
; ModuleID = 'llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp'
source_filename = "llvm/tools/clang/test/CodeGenCXX/intel/builtin-intel-subscript.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ArrDesc = type { %struct.A*, i64, i64, i64, i64, i64, [32 x %struct.DimDesc] }
%struct.A = type { float, i16 }
%struct.DimDesc = type { i64, i64, i64 }

@.str = private unnamed_addr constant [4 x i8] c"%f \00", align 1

; Function Attrs: nounwind uwtable
define void @_Z4testPK7ArrDescI1AEii(%struct.ArrDesc* noalias nocapture readonly %inout, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %Base.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 0
  %0 = load %struct.A*, %struct.A** %Base.i, align 8
  %stride.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 0, i32 1
  %1 = load i64, i64* %stride.i.i, align 8
  %conv.i.i = trunc i64 %1 to i32
  %lb.i.i = getelementptr inbounds %struct.ArrDesc, %struct.ArrDesc* %inout, i64 0, i32 6, i64 0, i32 2
  %2 = load i64, i64* %lb.i.i, align 8
  %conv5.i.i = trunc i64 %2 to i32
  %call.i.i.i = tail call %struct.A* @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 %conv5.i.i, i32 %conv.i.i, %struct.A* %0, i16 signext 1) #5
  %i5 = getelementptr inbounds %struct.A, %struct.A* %call.i.i.i, i64 0, i32 1
  %3 = load i16, i16* %i5, align 4
  %call.i.i.i19 = tail call %struct.A* @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 %conv5.i.i, i32 %conv.i.i, %struct.A* %0, i16 signext %3) #5
  %f = getelementptr inbounds %struct.A, %struct.A* %call.i.i.i19, i64 0, i32 0
  %4 = load float, float* %f, align 4
  %conv10 = fadd float %4, 1.000000e+00
  store float %conv10, float* %f, align 4
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  %Arr = alloca [10 x %struct.A], align 16
  %0 = bitcast [10 x %struct.A]* %Arr to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %0) #5
  %arrayidx = getelementptr inbounds [10 x %struct.A], [10 x %struct.A]* %Arr, i64 0, i64 0
  %i5 = getelementptr inbounds [10 x %struct.A], [10 x %struct.A]* %Arr, i64 0, i64 1, i32 1
  store i16 9, i16* %i5, align 4
  %f = getelementptr inbounds [10 x %struct.A], [10 x %struct.A]* %Arr, i64 0, i64 9, i32 0
  store float 2.000000e+00, float* %f, align 8
  %call.i.i.i.i = call %struct.A* @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 0, i32 8, %struct.A* nonnull %arrayidx, i16 signext 1) #5
  %i5.i = getelementptr inbounds %struct.A, %struct.A* %call.i.i.i.i, i64 0, i32 1
  %1 = load i16, i16* %i5.i, align 4
  %call.i.i.i19.i = call %struct.A* @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 0, i32 8, %struct.A* nonnull %arrayidx, i16 signext %1) #5
  %f.i = getelementptr inbounds %struct.A, %struct.A* %call.i.i.i19.i, i64 0, i32 0
  %2 = load float, float* %f.i, align 4
  %conv10.i = fadd float %2, 1.000000e+00
  store float %conv10.i, float* %f.i, align 4
  %3 = load float, float* %f, align 8
  %conv9 = fpext float %3 to double
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), double %conv9)
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %0) #5
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare %struct.A* @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8, i32, i32, %struct.A*, i16) #4

attributes #0 = { uwtable  }
attributes #1 = { argmemonly nounwind }
attributes #2 = { uwtable  }
attributes #3 = { nounwind }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }
