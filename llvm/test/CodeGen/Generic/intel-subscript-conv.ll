; REQUIRES: system-linux
; RUN: opt -S -passes=convert-to-subscript %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: opt -S -passes=convert-to-subscript %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; RUN: opt -S %s 2>&1 | %lli | FileCheck -check-prefix=CHECK-EXEC %s
; CHECK-EXEC: passed

; RUN: opt -S -passes=convert-to-subscript %s 2>&1 | FileCheck -check-prefix=OPAQUE-CONV %s
; @_Z3fooi (GetElementPtrInst)
; OPAQUE-CONV: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 132, ptr elementtype(%"struct.A::B")
; OPAQUE-CONV: getelementptr inbounds %"struct.A::B", ptr {{.*}}, i64 0, i32 3, i64 0
; OPAQUE-CONV: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 24, ptr elementtype(%"struct.A::B::C")
; OPAQUE-CONV: getelementptr inbounds %"struct.A::B::C", ptr {{.*}}, i64 0, i32 5
; @_Z4bar1v (GEPOperator)
; OPAQUE-CONV: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 132, ptr elementtype(%"struct.A::B") getelementptr inbounds (%struct.A, ptr @a, i64 0, i32 1, i64 0), i64 2)
; OPAQUE-CONV: getelementptr inbounds %"struct.A::B", ptr {{.*}}, i64 0, i32 3, i64 0
; OPAQUE-CONV: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 24, ptr
; OPAQUE-CONV: getelementptr inbounds %"struct.A::B::C", ptr  {{.*}}, i64 0, i32 5
; @_Z3bazv (GEPOperator + 0th array offsets)
; OPAQUE-CONV: ret ptr getelementptr inbounds (%struct.A, ptr @a, i64 0, i32 1, i64 0, i32 3, i64 0, i32 5)

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
define ptr @_Z3fooi(i32 %i) local_unnamed_addr #0 {
entry:
  %idxprom = sext i32 %i to i64
  %add = add nsw i32 %i, 2
  %idxprom1 = sext i32 %add to i64
  %b5 = getelementptr inbounds %struct.A, ptr @a, i64 0, i32 1, i64 %idxprom, i32 3, i64 %idxprom1, i32 5
  ret ptr %b5
}

; Function Attrs: noinline nounwind optnone uwtable
define ptr @_Z4foo1i(i32 %i) local_unnamed_addr #2 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, ptr %i.addr, align 4
  %0 = load i32, ptr %i.addr, align 4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [3 x %"struct.A::B"], ptr getelementptr inbounds (%struct.A, ptr @a, i32 0, i32 1), i64 0, i64 %idxprom
  %c = getelementptr inbounds %"struct.A::B", ptr %arrayidx, i32 0, i32 3
  %1 = load i32, ptr %i.addr, align 4
  %add = add nsw i32 %1, 2
  %idxprom1 = sext i32 %add to i64
  %arrayidx2 = getelementptr inbounds [5 x %"struct.A::B::C"], ptr %c, i64 0, i64 %idxprom1
  %b5 = getelementptr inbounds %"struct.A::B::C", ptr %arrayidx2, i32 0, i32 5
  ret ptr %b5
}

; Function Attrs: nounwind readnone uwtable
define ptr @_Z3barv() local_unnamed_addr #0 {
entry:
  ret ptr getelementptr inbounds (%struct.A, ptr @a, i64 0, i32 1, i64 2, i32 3, i64 4, i32 5)
}

; Function Attrs: noinline nounwind optnone uwtable
define ptr @_Z4bar1v() local_unnamed_addr #2 {
entry:
  ret ptr getelementptr inbounds (%struct.A, ptr @a, i32 0, i32 1, i64 2, i32 3, i64 4, i32 5)
}

; Function Attrs: nounwind readnone uwtable
define ptr @_Z3bazv() local_unnamed_addr #0 {
entry:
  ret ptr getelementptr inbounds (%struct.A, ptr @a, i64 0, i32 1, i64 0, i32 3, i64 0, i32 5)
}

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #3 {
entry:
  %call1 = tail call ptr @_Z4foo1i(i32 2)
  %cmp = icmp eq ptr %call1, getelementptr inbounds (%struct.A, ptr @a, i64 0, i32 1, i64 2, i32 3, i64 4, i32 5)
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %call3 = tail call ptr @_Z4bar1v()
  %cmp4 = icmp eq ptr %call3, getelementptr inbounds (%struct.A, ptr @a, i64 0, i32 1, i64 2, i32 3, i64 4, i32 5)
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %call5 = tail call i32 @puts(ptr getelementptr inbounds ([7 x i8], ptr @.str, i64 0, i64 0))
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @puts(ptr nocapture readonly) local_unnamed_addr #4

attributes #0 = { nounwind readnone uwtable}
attributes #1 = { nounwind readnone }
attributes #2 = { noinline nounwind optnone uwtable}
attributes #3 = { norecurse nounwind uwtable}
attributes #4 = { nounwind }
attributes #5 = { nounwind }
