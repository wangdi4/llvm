; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='lto-pre-link<O2>' -inline-report=0xe807 -dtrans-inline-heuristics < %s -S 2>&1 | FileCheck --check-prefixes=CHECK-CL-PRE %s
; RUN: opt -passes='lto<O2>' -inline-report=0xe807 -dtrans-inline-heuristics < %s -S 2>&1 | FileCheck --check-prefixes=CHECK-CL-POST %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='lto-pre-link<O2>' -inline-report=0xe886 -dtrans-inline-heuristics -pre-lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD-PRE
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='lto<O2>' -inline-report=0xe886 -dtrans-inline-heuristics -pre-lto-inline-cost -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD-POST

; This test case checks that the function _ZN12cMessageHeap7shiftupEi was
; selected for delay inlining by the inliner in the new pass manager.
; It will verify that the message was printed correctly in the pre-lto
; phase ("Inline decision is delayed until link time") and post-lto
; phase ("Inline decision is delayed").

; CHECK-CL-PRE: call void @_ZN12cMessageHeap7shiftupEi
; CHECK-CL-PRE: COMPILE FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-CL-PRE: _ZN12cMessageHeap7shiftupEi{{.*}}Inline decision is delayed until link time

; CHECK-CL-POST: call void @_ZN12cMessageHeap7shiftupEi
; CHECK-CL-POST: COMPILE FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-CL-POST: _ZN12cMessageHeap7shiftupEi{{.*}}Inline decision is delayed


; CHECK-MD-PRE: COMPILE FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-MD-PRE: _ZN12cMessageHeap7shiftupEi{{.*}}Inline decision is delayed until link time
; CHECK-MD-PRE: call void @_ZN12cMessageHeap7shiftupEi

; CHECK-MD-POST: COMPILE FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-MD-POST: _ZN12cMessageHeap7shiftupEi{{.*}}Inline decision is delayed
; CHECK-MD-POST: call void @_ZN12cMessageHeap7shiftupEi

%class.cNamedObject = type <{ %class.cObject, i8*, i16, i16, [4 x i8] }>
%class.cMessageHeap = type { %class.cOwnedObject.base, %class.cMessage**, i32, i32, i64 }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, i8*, i16, i16 }>
%class.cObject = type { i32 (...)** }
%class.cMessage = type { %class.cOwnedObject.base, i16, i16, i16, %class.cArray*, %class.cObject*, i8*, i32, i32, i32, i32, %class.SimTime, %class.SimTime, %class.SimTime, %class.SimTime, i32, i64, i64, i64, i64 }
%class.cArray = type { %class.cOwnedObject.base, %class.cObject**, i32, i32, i32, i32 }
%class.SimTime = type { i64 }
%class.cOwnedObject = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32, [4 x i8] }>

declare dso_local i32 @_ZgtR8cMessageS0_(%class.cMessage* dereferenceable(160) %a, %class.cMessage* dereferenceable(160) %b)

declare void @llvm.assume(i1)

declare i1 @llvm.intel.wholeprogramsafe()

declare i1 @llvm.type.test(i8*, metadata)

define dso_local %class.cMessage* @_ZN12cMessageHeap11removeFirstEv(%class.cMessageHeap* %this) local_unnamed_addr #0 align 2 {
entry:
  %n = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %this, i64 0, i32 2
  %0 = load i32, i32* %n, align 8
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %if.then, label %return

if.then:                                          ; preds = %entry
  %h = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %this, i64 0, i32 1
  %1 = load %class.cMessage**, %class.cMessage*** %h, align 8
  %arrayidx = getelementptr inbounds %class.cMessage*, %class.cMessage** %1, i64 1
  %2 = load %class.cMessage*, %class.cMessage** %arrayidx, align 8
  %dec = add nsw i32 %0, -1
  store i32 %dec, i32* %n, align 8
  %idxprom = sext i32 %0 to i64
  %arrayidx4 = getelementptr inbounds %class.cMessage*, %class.cMessage** %1, i64 %idxprom
  %3 = load %class.cMessage*, %class.cMessage** %arrayidx4, align 8
  %4 = load %class.cMessage**, %class.cMessage*** %h, align 8
  %arrayidx6 = getelementptr inbounds %class.cMessage*, %class.cMessage** %4, i64 1
  store %class.cMessage* %3, %class.cMessage** %arrayidx6, align 8
  %heapindex = getelementptr inbounds %class.cMessage, %class.cMessage* %3, i64 0, i32 15
  store i32 1, i32* %heapindex, align 8
  call void @_ZN12cMessageHeap7shiftupEi(%class.cMessageHeap* %this, i32 1)
  %5 = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %this, i64 0, i32 0, i32 0, i32 0
  %6 = bitcast %class.cMessage* %2 to %class.cOwnedObject*
  %7 = bitcast %class.cMessageHeap* %this to void (%class.cObject*, %class.cOwnedObject*)***
  %vtable = load void (%class.cObject*, %class.cOwnedObject*)**, void (%class.cObject*, %class.cOwnedObject*)*** %7, align 8
  %8 = call i1 @llvm.intel.wholeprogramsafe()
  br i1 %8, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.then
  %9 = bitcast void (%class.cObject*, %class.cOwnedObject*)** %vtable to i8*
  %10 = call i1 @llvm.type.test(i8* %9, metadata !"_ZTS7cObject")
  call void @llvm.assume(i1 %10)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.then
  %vfn = getelementptr inbounds void (%class.cObject*, %class.cOwnedObject*)*, void (%class.cObject*, %class.cOwnedObject*)** %vtable, i64 13
  %11 = load void (%class.cObject*, %class.cOwnedObject*)*, void (%class.cObject*, %class.cOwnedObject*)** %vfn, align 8
  call void %11(%class.cObject* %5, %class.cOwnedObject* %6)
  %heapindex7 = getelementptr inbounds %class.cMessage, %class.cMessage* %2, i64 0, i32 15
  store i32 -1, i32* %heapindex7, align 8
  br label %return

return:                                           ; preds = %entry, %whpr.continue
  %retval.0 = phi %class.cMessage* [ %2, %whpr.continue ], [ null, %entry ]
  ret %class.cMessage* %retval.0
}

define dso_local void @_ZN12cMessageHeap7shiftupEi(%class.cMessageHeap* %this, i32 %from) #0 align 2 {
entry:
  br label %while.cond

while.cond:                                       ; preds = %if.then15, %entry
  %i.0 = phi i32 [ %from, %entry ], [ %j.0, %if.then15 ]
  %shl = shl i32 %i.0, 1
  %n = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %this, i32 0, i32 2
  %0 = load i32, i32* %n, align 8
  %cmp = icmp sle i32 %shl, %0
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %cmp3 = icmp slt i32 %shl, %0
  br i1 %cmp3, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %while.body
  %h = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %this, i32 0, i32 1
  %1 = load %class.cMessage**, %class.cMessage*** %h, align 8
  %idxprom = sext i32 %shl to i64
  %arrayidx = getelementptr inbounds %class.cMessage*, %class.cMessage** %1, i64 %idxprom
  %2 = load %class.cMessage*, %class.cMessage** %arrayidx, align 8
  %add = add nsw i32 %shl, 1
  %idxprom5 = sext i32 %add to i64
  %arrayidx6 = getelementptr inbounds %class.cMessage*, %class.cMessage** %1, i64 %idxprom5
  %3 = load %class.cMessage*, %class.cMessage** %arrayidx6, align 8
  %call = call i32 @_ZgtR8cMessageS0_(%class.cMessage* dereferenceable(160) %2, %class.cMessage* dereferenceable(160) %3)
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %while.body
  %j.0 = phi i32 [ %add, %if.then ], [ %shl, %land.lhs.true ], [ %shl, %while.body ]
  %h7 = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %this, i32 0, i32 1
  %4 = load %class.cMessage**, %class.cMessage*** %h7, align 8
  %idxprom8 = sext i32 %i.0 to i64
  %arrayidx9 = getelementptr inbounds %class.cMessage*, %class.cMessage** %4, i64 %idxprom8
  %5 = load %class.cMessage*, %class.cMessage** %arrayidx9, align 8
  %idxprom11 = sext i32 %j.0 to i64
  %arrayidx12 = getelementptr inbounds %class.cMessage*, %class.cMessage** %4, i64 %idxprom11
  %6 = load %class.cMessage*, %class.cMessage** %arrayidx12, align 8
  %call13 = call i32 @_ZgtR8cMessageS0_(%class.cMessage* dereferenceable(160) %5, %class.cMessage* dereferenceable(160) %6)
  %tobool14 = icmp ne i32 %call13, 0
  br i1 %tobool14, label %if.then15, label %while.end

if.then15:                                        ; preds = %if.end
  %7 = load %class.cMessage**, %class.cMessage*** %h7, align 8
  %arrayidx18 = getelementptr inbounds %class.cMessage*, %class.cMessage** %7, i64 %idxprom11
  %8 = load %class.cMessage*, %class.cMessage** %arrayidx18, align 8
  %arrayidx21 = getelementptr inbounds %class.cMessage*, %class.cMessage** %7, i64 %idxprom8
  %9 = load %class.cMessage*, %class.cMessage** %arrayidx21, align 8
  store %class.cMessage* %9, %class.cMessage** %arrayidx18, align 8
  %heapindex = getelementptr inbounds %class.cMessage, %class.cMessage* %9, i32 0, i32 15
  store i32 %j.0, i32* %heapindex, align 8
  %10 = load %class.cMessage**, %class.cMessage*** %h7, align 8
  %arrayidx27 = getelementptr inbounds %class.cMessage*, %class.cMessage** %10, i64 %idxprom8
  store %class.cMessage* %8, %class.cMessage** %arrayidx27, align 8
  %heapindex28 = getelementptr inbounds %class.cMessage, %class.cMessage* %8, i32 0, i32 15
  store i32 %i.0, i32* %heapindex28, align 8
  br label %while.cond

while.end:                                        ; preds = %if.end, %while.cond
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
