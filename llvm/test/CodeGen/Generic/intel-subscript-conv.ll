; REQUIRES: system-linux
; RUN: opt -S -convert-to-subscript %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: opt -S %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: passed

; RUN: opt -S -convert-to-subscript %s 2>&1 | FileCheck -check-prefix=CHECK-CONV %s
; @_Z3fooi (GetElementPtrInst)
; CHECK-CONV: call %"struct.A::B"* @"llvm.intel.subscript.p0s_struct.A::Bs.i64.i64.p0s_struct.A::Bs.i64"(i8 0, i64 0, i64 132
; CHECK-CONV: getelementptr inbounds %"struct.A::B", %"struct.A::B"* {{.*}}, i64 0, i32 3, i64 0
; CHECK-CONV: call %"struct.A::B::C"* @"llvm.intel.subscript.p0s_struct.A::B::Cs.i64.i64.p0s_struct.A::B::Cs.i64"(i8 0, i64 0, i64 24
; CHECK-CONV: getelementptr inbounds %"struct.A::B::C", %"struct.A::B::C"* {{.*}}, i64 0, i32 5
; @_Z4bar1v (GEPOperator)
; CHECK-CONV: call %"struct.A::B"* @"llvm.intel.subscript.p0s_struct.A::Bs.i64.i64.p0s_struct.A::Bs.i64"(i8 0, i64 0, i64 132, %"struct.A::B"* getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i64 0), i64 2)
; CHECK-CONV: getelementptr inbounds %"struct.A::B", %"struct.A::B"* {{.*}}, i64 0, i32 3, i64 0
; CHECK-CONV: call %"struct.A::B::C"* @"llvm.intel.subscript.p0s_struct.A::B::Cs.i64.i64.p0s_struct.A::B::Cs.i64"(i8 0, i64 0, i64 24, %"struct.A::B::C"*
; CHECK-CONV: getelementptr inbounds %"struct.A::B::C", %"struct.A::B::C"*  {{.*}}, i64 0, i32 5
; @_Z3bazv (GEPOperator + 0th array offsets)
; CHECK-CONV: ret i32* getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i64 0, i32 3, i64 0, i32 5)

; ModuleID = 'm.cc'
source_filename = "m.cc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type { i32, [3 x %"struct.A::B"] }
%"struct.A::B" = type { i32, i32, i32, [5 x %"struct.A::B::C"] }
%"struct.A::B::C" = type { i32, i32, i32, i32, i32, i32 }

@a = global %struct.A zeroinitializer, align 4
@.str = private unnamed_addr constant [7 x i8] c"passed\00", align 1

; Function Attrs: nounwind readnone uwtable
define i32* @_Z3fooi(i32 %i) local_unnamed_addr #0 {
entry:
  %idxprom = sext i32 %i to i64
  %add = add nsw i32 %i, 2
  %idxprom1 = sext i32 %add to i64
  %b5 = getelementptr inbounds %struct.A, %struct.A* @a, i64 0, i32 1, i64 %idxprom, i32 3, i64 %idxprom1, i32 5
  ret i32* %b5
}

; Function Attrs: noinline nounwind optnone uwtable
define i32* @_Z4foo1i(i32 %i) local_unnamed_addr #2 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [3 x %"struct.A::B"], [3 x %"struct.A::B"]* getelementptr inbounds (%struct.A, %struct.A* @a, i32 0, i32 1), i64 0, i64 %idxprom
  %c = getelementptr inbounds %"struct.A::B", %"struct.A::B"* %arrayidx, i32 0, i32 3
  %1 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %1, 2
  %idxprom1 = sext i32 %add to i64
  %arrayidx2 = getelementptr inbounds [5 x %"struct.A::B::C"], [5 x %"struct.A::B::C"]* %c, i64 0, i64 %idxprom1
  %b5 = getelementptr inbounds %"struct.A::B::C", %"struct.A::B::C"* %arrayidx2, i32 0, i32 5
  ret i32* %b5
}

; Function Attrs: nounwind readnone uwtable
define i32* @_Z3barv() local_unnamed_addr #0 {
entry:
  ret i32* getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i64 2, i32 3, i64 4, i32 5)
}

; Function Attrs: noinline nounwind optnone uwtable
define i32* @_Z4bar1v() local_unnamed_addr #2 {
entry:
  ret i32* getelementptr inbounds (%struct.A, %struct.A* @a, i32 0, i32 1, i64 2, i32 3, i64 4, i32 5)
}

; Function Attrs: nounwind readnone uwtable
define i32* @_Z3bazv() local_unnamed_addr #0 {
entry:
  ret i32* getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i64 0, i32 3, i64 0, i32 5)
}

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #3 {
entry:
  %call1 = tail call i32* @_Z4foo1i(i32 2)
  %cmp = icmp eq i32* %call1, getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i64 2, i32 3, i64 4, i32 5)
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %call3 = tail call i32* @_Z4bar1v()
  %cmp4 = icmp eq i32* %call3, getelementptr inbounds (%struct.A, %struct.A* @a, i64 0, i32 1, i64 2, i32 3, i64 4, i32 5)
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %call5 = tail call i32 @puts(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i64 0, i64 0))
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #4

attributes #0 = { nounwind readnone uwtable}
attributes #1 = { nounwind readnone }
attributes #2 = { noinline nounwind optnone uwtable}
attributes #3 = { norecurse nounwind uwtable}
attributes #4 = { nounwind }
attributes #5 = { nounwind }
