; Checks that MapIntrinToIml will do acosd -> __bwr_acosd conversion
; in case of imf-arch-consistency (CMPLRLLVM-10819)

; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -vector-library=SVML -O0 -iml-trans -S < %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -vector-library=SVML -passes="map-intrin-to-iml" -S < %s | FileCheck %s

; CHECK: %call = call double @__bwr_acosd

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

attributes #0 = { nounwind "imf-arch-consistency"="true" }

%struct.testSetType = type { [3 x float], [3 x double], [3 x i32], [3 x i64], [3 x i64], [3 x float], [3 x double], [3 x i32], [3 x i64], [3 x i64], [3 x float], [3 x double], [3 x i32], [3 x i64], [3 x i64] }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @runFunction(%struct.testSetType* %testSet) #0 {
entry:
  %testSet.addr = alloca %struct.testSetType*, align 8
  store %struct.testSetType* %testSet, %struct.testSetType** %testSet.addr, align 8
  %0 = load %struct.testSetType*, %struct.testSetType** %testSet.addr, align 8
  %argDouble = getelementptr inbounds %struct.testSetType, %struct.testSetType* %0, i32 0, i32 1
  %arrayidx = getelementptr inbounds [3 x double], [3 x double]* %argDouble, i64 0, i64 0
  %1 = load double, double* %arrayidx, align 8
  %call = call double @acosd(double %1) #0
  %2 = load %struct.testSetType*, %struct.testSetType** %testSet.addr, align 8
  %actualResDouble = getelementptr inbounds %struct.testSetType, %struct.testSetType* %2, i32 0, i32 11
  %arrayidx1 = getelementptr inbounds [3 x double], [3 x double]* %actualResDouble, i64 0, i64 0
  store double %call, double* %arrayidx1, align 8
  ret void
}

declare dso_local double @acosd(double %0) #0
