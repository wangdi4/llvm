; CMPLRLLVM-11269: The loop in the fcmp block was crashing the blender
; pattern matching.

; RUN: opt -passes="simplifycfg" -S < %s | FileCheck %s
; CHECK: %cmp = fcmp fast olt double %11, 1.000000e+00
; CHECK-NEXT: br i1 %cmp, label %for.end, label %for.cond

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27041"

%class1 = type { %class2, i8, double }
%class2 = type { double, double }
%class3 = type { %class4, [256 x i32], i32, double }
%class4 = type { i32 }

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

define dso_local double @fcmploop(%class1* %this, %class3* %_Eng, %class2* %_Par0, i1 %_Keep) #1 align 2 {
entry:
  %_Keep.addr = alloca i8, align 1
  %_Par0.addr = alloca %class2*, align 8
  %_Eng.addr = alloca %class3*, align 8
  %this.addr = alloca %class1*, align 8
  %_Res = alloca double, align 8
  %_V1 = alloca double, align 8
  %_V2 = alloca double, align 8
  %_Sx = alloca double, align 8
  %_Fx = alloca double, align 8
  %frombool = zext i1 %_Keep to i8
  store i8 %frombool, i8* %_Keep.addr, align 1
  store %class2* %_Par0, %class2** %_Par0.addr, align 8
  store %class3* %_Eng, %class3** %_Eng.addr, align 8
  store %class1* %this, %class1** %this.addr, align 8
  %this1 = load %class1*, %class1** %this.addr, align 8
  %0 = bitcast double* %_Res to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #2
  %1 = load i8, i8* %_Keep.addr, align 1
  %tobool = trunc i8 %1 to i1
  %2 = bitcast double* %_V1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #2
  %3 = bitcast double* %_V2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #2
  %4 = bitcast double* %_Sx to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #2
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %5 = load %class3*, %class3** %_Eng.addr, align 8
  %call = call fast double @func0(%class3* nonnull align 8 dereferenceable(1040) %5)
  %mul = fmul fast double 2.000000e+00, %call
  %sub = fsub fast double %mul, 1.000000e+00
  store double %sub, double* %_V1, align 8
  %6 = load %class3*, %class3** %_Eng.addr, align 8
  %call4 = call fast double @func0(%class3* nonnull align 8 dereferenceable(1040) %6)
  %mul5 = fmul fast double 2.000000e+00, %call4
  %sub6 = fsub fast double %mul5, 1.000000e+00
  store double %sub6, double* %_V2, align 8
  %7 = load double, double* %_V1, align 8
  %8 = load double, double* %_V1, align 8
  %mul7 = fmul fast double %7, %8
  %9 = load double, double* %_V2, align 8
  %10 = load double, double* %_V2, align 8
  %mul8 = fmul fast double %9, %10
  %add = fadd fast double %mul7, %mul8
  store double %add, double* %_Sx, align 8
  %11 = load double, double* %_Sx, align 8
  %cmp = fcmp fast olt double %11, 1.000000e+00
  br i1 %cmp, label %for.end, label %for.cond

for.end:                                          ; preds = %for.cond
  %12 = bitcast double* %_Fx to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %12) #2
  %13 = load double, double* %_Sx, align 8
  %call10 = call fast double @"?log@@YAOO@Z"(double %13) #2
  %mul11 = fmul fast double -2.000000e+00, %call10
  %14 = load double, double* %_Sx, align 8
  %div = fdiv fast double %mul11, %14
  %call12 = call fast double @"?sqrt@@YAOO@Z"(double %div) #2
  store double %call12, double* %_Fx, align 8
  %15 = load i8, i8* %_Keep.addr, align 1
  %tobool13 = trunc i8 %15 to i1
  unreachable
}

declare dso_local double @func0(%class3*) #1

declare dso_local double @"?sqrt@@YAOO@Z"(double) #1

declare dso_local double @"?log@@YAOO@Z"(double) #1

attributes #0 = { argmemonly nounwind willreturn }
attributes #1 = { "use-soft-float"="false" }
attributes #2 = { nounwind }


