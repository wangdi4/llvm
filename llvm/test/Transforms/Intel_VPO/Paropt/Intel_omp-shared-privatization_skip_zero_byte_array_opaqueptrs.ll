; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-shared-privatization -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-shared-privatization)' -S %s | FileCheck %s

; Skip 0-byte arrays as privatization candidates in shared-privatization pass

; Test src:
; void foo()
; {
;  int i,k,nx;
;  double lhsX[5][5][0];  // Constant size
;  double lhsY[5][5][nx]; // Dynamic (uninitialized)
;
;  #pragma omp parallel
;  lhsX[5];
;
;  #pragma omp parallel
;  lhsY[5];
; }

; CHECK-NOT: "QUAL.OMP.PRIVATE:TYPED"(ptr %lhsX, double 0.000000e+00, i64 0) 
; CHECK-NOT: "QUAL.OMP.PRIVATE:TYPED"(ptr %vla20.sub, double 0.000000e+00, i64 0)

; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() local_unnamed_addr {
DIR.OMP.PARALLEL.316:
  %lhsX = alloca [5 x [5 x [0 x double]]], align 16
  %omp.vla.tmp = alloca i64, align 8
  call void @llvm.lifetime.start.p0(i64 0, ptr nonnull %lhsX)
  %0 = call ptr @llvm.stacksave()
  %vla20.sub = getelementptr inbounds [0 x double], ptr %lhsX, i64 0, i64 0
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %DIR.OMP.PARALLEL.316
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %lhsX, double 0.000000e+00, i64 0),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  br label %DIR.OMP.PARALLEL.321

DIR.OMP.PARALLEL.321:                             ; preds = %DIR.OMP.PARALLEL.2
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.PARALLEL.818, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.321
  br label %DIR.OMP.PARALLEL.818

DIR.OMP.PARALLEL.818:                             ; preds = %DIR.OMP.PARALLEL.3, %DIR.OMP.PARALLEL.321
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.818
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  store i64 0, ptr %omp.vla.tmp, align 8
  %end.dir.temp13 = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.6

DIR.OMP.PARALLEL.6:                               ; preds = %DIR.OMP.END.PARALLEL.5
  br label %DIR.OMP.PARALLEL.7

DIR.OMP.PARALLEL.7:                               ; preds = %DIR.OMP.PARALLEL.6
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %vla20.sub, double 0.000000e+00, i64 0),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp13) ]

  br label %DIR.OMP.PARALLEL.822

DIR.OMP.PARALLEL.822:                             ; preds = %DIR.OMP.PARALLEL.7
  %temp.load14 = load volatile i1, ptr %end.dir.temp13, align 1
  br i1 %temp.load14, label %DIR.OMP.END.PARALLEL.10, label %DIR.OMP.PARALLEL.8

DIR.OMP.PARALLEL.8:                               ; preds = %DIR.OMP.PARALLEL.822
  br label %DIR.OMP.END.PARALLEL.10

DIR.OMP.END.PARALLEL.10:                          ; preds = %DIR.OMP.PARALLEL.822, %DIR.OMP.PARALLEL.8
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.PARALLEL.10
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  br label %DIR.OMP.END.PARALLEL.1023

DIR.OMP.END.PARALLEL.1023:                        ; preds = %DIR.OMP.END.PARALLEL.9
  call void @llvm.stackrestore(ptr %0)
  call void @llvm.lifetime.end.p0(i64 0, ptr nonnull %lhsX)
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare ptr @llvm.stacksave()

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.stackrestore(ptr)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
