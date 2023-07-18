; RUN: opt -passes="gvn" < %s -S | FileCheck %s
; ModuleID = 'builtin-intel-subscript.cpp'
source_filename = "builtin-intel-subscript.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ArrDesc = type { ptr, i64, i64, i64, i64, i64, [32 x %struct.DimDesc] }
%struct.A = type { float, i16 }
%struct.DimDesc = type { i64, i64, i64 }

; Function Attrs: nounwind uwtable
define dso_local void @_Z4testPK7ArrDescI1AEii(ptr noalias nocapture readonly %inout, i32 %N, i32 %K) local_unnamed_addr #0 {
; CHECK-LABEL: @_Z4testPK7ArrDescI1AEii
entry:
  %Base.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 0
  %0 = load ptr, ptr %Base.i, align 8
  %stride.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 0, i32 1
  %1 = load i64, ptr %stride.i.i, align 8
  %conv.i.i = trunc i64 %1 to i32
  %lb.i.i = getelementptr inbounds %struct.ArrDesc, ptr %inout, i64 0, i32 6, i64 0, i32 2
  %2 = load i64, ptr %lb.i.i, align 8
  %conv5.i.i = trunc i64 %2 to i32
  %call.i.i.i = tail call ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 %conv5.i.i, i32 %conv.i.i, ptr elementtype(%struct.A) %0, i16 signext 1) #2
  %i5 = getelementptr inbounds %struct.A, ptr %call.i.i.i, i64 0, i32 1
  %3 = load i16, ptr %i5, align 4
  %call.i.i.i19 = tail call ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8 zeroext 0, i32 %conv5.i.i, i32 %conv.i.i, ptr elementtype(%struct.A) %0, i16 signext %3) #2
  %f = getelementptr inbounds %struct.A, ptr %call.i.i.i19, i64 0, i32 0
  %4 = load float, ptr %f, align 4
  %conv10 = fadd float %4, 1.000000e+00
  store float %conv10, ptr %f, align 4
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0s_struct.As.i32.i32.p0s_struct.As.i16(i8, i32, i32, ptr, i16) #1

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
