; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; WARNING!!!
; WARNING!!!      ** CONTAINS INTEL IP **
; WARNING!!!      DO NOT SHARE EXTERNALLY
; WARNING!!!
;
; FPGA test had "#pragma ivdep" and caused an assertion in WRN graph building.
; void tmp() {
;   int AAA[10];
;   int i;
;   #pragma ivdep array (AAA)
;   for (i = 0; i != 10; i++)
;     AAA[i] = i;
; }

; Verify that the "DIR.PRAGMA.IVDEP" directive is preserved.
; CHECK: %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(){{.*}} ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @tmp() {
entry:
  %AAA = alloca [10 x i32], align 16
  %i = alloca i32, align 4
  %0 = bitcast [10 x i32]* %AAA to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* %0)
  %1 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(),
    "QUAL.PRAGMA.ARRAY"([10 x i32]* %AAA, i32 -1) ]
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %3 = load i32, i32* %i, align 4
  %cmp = icmp ne i32 %3, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load i32, i32* %i, align 4
  %5 = load i32, i32* %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %AAA, i64 0, i64 %idxprom
  store i32 %4, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %6 = load i32, i32* %i, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.PRAGMA.END.IVDEP"() ]
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7)
  %8 = bitcast [10 x i32]* %AAA to i8*
  call void @llvm.lifetime.end.p0i8(i64 40, i8* %8)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)
