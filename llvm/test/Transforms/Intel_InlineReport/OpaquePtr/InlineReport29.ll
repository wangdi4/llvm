; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost < %s -S 2>&1 | FileCheck --check-prefix=CHECK-CL %s
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost < %s -S 2>&1 | FileCheck --check-prefix=CHECK-CL %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -pre-lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-MD

; This test checks that the function _ZN12cMessageHeap7shiftupEi is not inlined
; because the inlining decision is delayed from the compile step to the link
; step of an LTO compilation.

; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-MD: _ZN12cMessageHeap7shiftupEi{{.*}}Inline decision is delayed until link time
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi
; CHECK-CL: call void @_ZN12cMessageHeap7shiftupEi
; CHECK-CL: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi
; CHECK-CL: COMPILE FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-CL: _ZN12cMessageHeap7shiftupEi{{.*}}Inline decision is delayed until link time

target triple = "x86_64-unknown-linux-gnu"

%class.cMessageHeap = type { %class.cOwnedObject.base, ptr, i32, i32, i64 }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], ptr, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, ptr, i16, i16 }>
%class.cObject = type { ptr }
%class.cMessage = type { %class.cOwnedObject.base, i16, i16, i16, ptr, ptr, ptr, i32, i32, i32, i32, %class.SimTime, %class.SimTime, %class.SimTime, %class.SimTime, i32, i64, i64, i64, i64 }
%class.SimTime = type { i64 }

declare dso_local i32 @_ZgtR8cMessageS0_(ptr dereferenceable(160), ptr dereferenceable(160))

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #0

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #2

define dso_local ptr @_ZN12cMessageHeap11removeFirstEv(ptr %this) local_unnamed_addr align 2 {
entry:
  %n = getelementptr inbounds %class.cMessageHeap, ptr %this, i64 0, i32 2
  %i = load i32, ptr %n, align 8
  %cmp = icmp sgt i32 %i, 0
  br i1 %cmp, label %if.then, label %return

if.then:                                          ; preds = %entry
  %h = getelementptr inbounds %class.cMessageHeap, ptr %this, i64 0, i32 1
  %i1 = load ptr, ptr %h, align 8
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 1
  %i2 = load ptr, ptr %arrayidx, align 8
  %dec = add nsw i32 %i, -1
  store i32 %dec, ptr %n, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx4 = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i3 = load ptr, ptr %arrayidx4, align 8
  %i4 = load ptr, ptr %h, align 8
  %arrayidx6 = getelementptr inbounds ptr, ptr %i4, i64 1
  store ptr %i3, ptr %arrayidx6, align 8
  %heapindex = getelementptr inbounds %class.cMessage, ptr %i3, i64 0, i32 15
  store i32 1, ptr %heapindex, align 8
  call void @_ZN12cMessageHeap7shiftupEi(ptr %this, i32 1)
  %i5 = getelementptr inbounds %class.cMessageHeap, ptr %this, i64 0, i32 0, i32 0, i32 0
  %vtable = load ptr, ptr %this, align 8
  %i8 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i8, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %i10 = call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTS7cObject")
  call void @llvm.assume(i1 %i10)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 13
  %i11 = load ptr, ptr %vfn, align 8
  call void %i11(ptr %i5, ptr %i2)
  %heapindex7 = getelementptr inbounds %class.cMessage, ptr %i2, i64 0, i32 15
  store i32 -1, ptr %heapindex7, align 8
  br label %return

return:                                           ; preds = %whpr.continue, %entry
  %retval.0 = phi ptr [ %i2, %whpr.continue ], [ null, %entry ]
  ret ptr %retval.0
}

define dso_local void @_ZN12cMessageHeap7shiftupEi(ptr %this, i32 %from) align 2 {
entry:
  br label %while.cond

while.cond:                                       ; preds = %if.then15, %entry
  %i.0 = phi i32 [ %from, %entry ], [ %j.0, %if.then15 ]
  %shl = shl i32 %i.0, 1
  %n = getelementptr inbounds %class.cMessageHeap, ptr %this, i32 0, i32 2
  %i = load i32, ptr %n, align 8
  %cmp = icmp sle i32 %shl, %i
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %cmp3 = icmp slt i32 %shl, %i
  br i1 %cmp3, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %while.body
  %h = getelementptr inbounds %class.cMessageHeap, ptr %this, i32 0, i32 1
  %i1 = load ptr, ptr %h, align 8
  %idxprom = sext i32 %shl to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i1, i64 %idxprom
  %i2 = load ptr, ptr %arrayidx, align 8
  %add = add nsw i32 %shl, 1
  %idxprom5 = sext i32 %add to i64
  %arrayidx6 = getelementptr inbounds ptr, ptr %i1, i64 %idxprom5
  %i3 = load ptr, ptr %arrayidx6, align 8
  %call = call i32 @_ZgtR8cMessageS0_(ptr dereferenceable(160) %i2, ptr dereferenceable(160) %i3)
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %while.body
  %j.0 = phi i32 [ %add, %if.then ], [ %shl, %land.lhs.true ], [ %shl, %while.body ]
  %h7 = getelementptr inbounds %class.cMessageHeap, ptr %this, i32 0, i32 1
  %i4 = load ptr, ptr %h7, align 8
  %idxprom8 = sext i32 %i.0 to i64
  %arrayidx9 = getelementptr inbounds ptr, ptr %i4, i64 %idxprom8
  %i5 = load ptr, ptr %arrayidx9, align 8
  %idxprom11 = sext i32 %j.0 to i64
  %arrayidx12 = getelementptr inbounds ptr, ptr %i4, i64 %idxprom11
  %i6 = load ptr, ptr %arrayidx12, align 8
  %call13 = call i32 @_ZgtR8cMessageS0_(ptr dereferenceable(160) %i5, ptr dereferenceable(160) %i6)
  %tobool14 = icmp ne i32 %call13, 0
  br i1 %tobool14, label %if.then15, label %while.end

if.then15:                                        ; preds = %if.end
  %i7 = load ptr, ptr %h7, align 8
  %arrayidx18 = getelementptr inbounds ptr, ptr %i7, i64 %idxprom11
  %i8 = load ptr, ptr %arrayidx18, align 8
  %arrayidx21 = getelementptr inbounds ptr, ptr %i7, i64 %idxprom8
  %i9 = load ptr, ptr %arrayidx21, align 8
  store ptr %i9, ptr %arrayidx18, align 8
  %heapindex = getelementptr inbounds %class.cMessage, ptr %i9, i32 0, i32 15
  store i32 %j.0, ptr %heapindex, align 8
  %i10 = load ptr, ptr %h7, align 8
  %arrayidx27 = getelementptr inbounds ptr, ptr %i10, i64 %idxprom8
  store ptr %i8, ptr %arrayidx27, align 8
  %heapindex28 = getelementptr inbounds %class.cMessage, ptr %i8, i32 0, i32 15
  store i32 %i.0, ptr %heapindex28, align 8
  br label %while.cond

while.end:                                        ; preds = %if.end, %while.cond
  ret void
}

attributes #0 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
; end INTEL_FEATURE_SW_ADVANCED
