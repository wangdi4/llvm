; REQUIRES: system-linux
; RUN: opt -S %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: opt -S -convert-to-subscript %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: passed

; RUN: opt -S -convert-to-subscript %s 2>&1 | FileCheck -check-prefix=CHECK %s
; CHECK:  getelementptr inbounds [9 x [8 x i32]], [9 x [8 x i32]]* %A, i64 0, i64 0, i64 0
; CHECK-NEXT:  call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 0, i64 288, i32*
; CHECK-NEXT:  call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 0, i64 32, i32*
; CHECK-NEXT:  call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 4, i32*

; CHECK: call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 3, i64 0, i64 2880, i32* getelementptr inbounds ([10 x [9 x [8 x i32]]], [10 x [9 x [8 x i32]]]* @A, i64 0, i64 0, i64 0, i64 0), i64 0)
; CHECK-NEXT: call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 0, i64 288, i32* {{.*}}, i64 1)
; CHECK-NEXT: call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 0, i64 32, i32* {{.*}}, i64 2)
; CHECK-NEXT: call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 4, i32* {{.*}}, i64 3)


; ModuleID = 'intel-subscript-conv.cc'
source_filename = "intel-subscript-conv.cc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [10 x [9 x [8 x i32]]] zeroinitializer, align 16
@.str = private unnamed_addr constant [7 x i8] c"passed\00", align 1

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @_Z3barPA9_A8_iiii([9 x [8 x i32]]* nocapture readonly %A, i32 %i, i32 %j, i32 %k) local_unnamed_addr #0 {
entry:
  %idxprom = sext i32 %i to i64
  %idxprom1 = sext i32 %j to i64
  %idxprom3 = sext i32 %k to i64
  %arrayidx4 = getelementptr inbounds [9 x [8 x i32]], [9 x [8 x i32]]* %A, i64 %idxprom, i64 %idxprom1, i64 %idxprom3
  %0 = load i32, i32* %arrayidx4, align 4
  ret i32 %0
}

; Function Attrs: noinline nounwind optnone uwtable
define i32 @_Z4bar1PA9_A8_iiii([9 x [8 x i32]]* %A, i32 %i, i32 %j, i32 %k) local_unnamed_addr #1 {
entry:
  %A.addr = alloca [9 x [8 x i32]]*, align 8
  %i.addr = alloca i32, align 4
  %j.addr = alloca i32, align 4
  %k.addr = alloca i32, align 4
  store [9 x [8 x i32]]* %A, [9 x [8 x i32]]** %A.addr, align 8
  store i32 %i, i32* %i.addr, align 4
  store i32 %j, i32* %j.addr, align 4
  store i32 %k, i32* %k.addr, align 4
  %0 = load [9 x [8 x i32]]*, [9 x [8 x i32]]** %A.addr, align 8
  %1 = load i32, i32* %i.addr, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [9 x [8 x i32]], [9 x [8 x i32]]* %0, i64 %idxprom
  %2 = load i32, i32* %j.addr, align 4
  %idxprom1 = sext i32 %2 to i64
  %arrayidx2 = getelementptr inbounds [9 x [8 x i32]], [9 x [8 x i32]]* %arrayidx, i64 0, i64 %idxprom1
  %3 = load i32, i32* %k.addr, align 4
  %idxprom3 = sext i32 %3 to i64
  %arrayidx4 = getelementptr inbounds [8 x i32], [8 x i32]* %arrayidx2, i64 0, i64 %idxprom3
  %4 = load i32, i32* %arrayidx4, align 4
  ret i32 %4
}

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #2 {
entry:
  %0 = load i32, i32* getelementptr inbounds ([10 x [9 x [8 x i32]]], [10 x [9 x [8 x i32]]]* @A, i64 0, i64 1, i64 2, i64 3), align 4
  %call1 = tail call i32 @_Z4bar1PA9_A8_iiii([9 x [8 x i32]]* getelementptr inbounds ([10 x [9 x [8 x i32]]], [10 x [9 x [8 x i32]]]* @A, i64 0, i64 0), i32 1, i32 2, i32 3)
  %cmp = icmp eq i32 %0, %call1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call2 = tail call i32 @puts(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0))
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #3

attributes #0 = { norecurse nounwind readonly uwtable }
attributes #1 = { noinline nounwind optnone uwtable}
attributes #2 = { norecurse nounwind uwtable}
attributes #3 = { nounwind }
