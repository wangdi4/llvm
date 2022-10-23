; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-MD

; This test checks that the two calls to _ZN12cMessageHeap11removeFirstEv and
; the calls to _ZN12cMessageHeap7shiftupEi inside
; _ZN12cMessageHeap11removeFirstEv are inlined, due to the 'structure test'
; heuristic. One inlining instance of _ZN12cMessageHeap11removeFirstEv
; should be marked as "Callee is single basic block with structure test"
; in the inlining report. After inlining that instance, the other will be
; a single callsite and will be indicated as such in the inlining report.

; CHECK-MD: COMPILE FUNC: _ZN11cSimulation16selectNextModuleEv
; CHECK-MD: INLINE: _ZN12cMessageHeap11removeFirstEv{{.*}}Callee is single basic block with structure test
; CHECK-MD: INLINE: _ZN12cMessageHeap7shiftupEi
; CHECK-MD: INLINE: _ZN12cMessageHeap11removeFirstEv{{.*}}Callee has single callsite and local linkage
; CHECK-MD: INLINE: _ZN12cMessageHeap7shiftupEi
; CHECK-MD: DEAD STATIC FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-MD: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi
; CHECK-MD-NOT: call{{.*}}_ZN12cMessageHeap11removeFirstEv

; CHECK-OLD: DEAD STATIC FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-OLD: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi
; CHECK-OLD: COMPILE FUNC: _ZN11cSimulation16selectNextModuleEv
; CHECK-OLD: INLINE: _ZN12cMessageHeap11removeFirstEv{{.*}}Callee is single basic block with structure test
; CHECK-OLD: INLINE: _ZN12cMessageHeap7shiftupEi
; CHECK-OLD: INLINE: _ZN12cMessageHeap11removeFirstEv{{.*}}Callee has single callsite and local linkage
; CHECK-OLD: INLINE: _ZN12cMessageHeap7shiftupEi
; CHECK-OLD-NOT: call{{.*}}_ZN12cMessageHeap11removeFirstEv
; CHECK-NEW-NOT: call{{.*}}_ZN12cMessageHeap11removeFirstEv
; CHECK-NEW: DEAD STATIC FUNC: _ZN12cMessageHeap11removeFirstEv
; CHECK-NEW: COMPILE FUNC: _ZN12cMessageHeap7shiftupEi
; CHECK-NEW: COMPILE FUNC: _ZN11cSimulation16selectNextModuleEv
; CHECK-NEW: INLINE: _ZN12cMessageHeap11removeFirstEv{{.*}}Callee is single basic block with structure test
; CHECK-NEW: INLINE: _ZN12cMessageHeap7shiftupEi
; CHECK-NEW: INLINE: _ZN12cMessageHeap11removeFirstEv{{.*}}Callee has single callsite and local linkage
; CHECK-NEW: INLINE: _ZN12cMessageHeap7shiftupEi

target triple = "x86_64-unknown-linux-gnu"

%class.cNamedObject = type <{ %class.cObject, i8*, i16, i16, [4 x i8] }>
%class.cMessageHeap = type { %class.cOwnedObject.base, %class.cMessage**, i32, i32, i64 }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, i8*, i16, i16 }>
%class.cObject = type { i32 (...)** }
%class.cMessage = type { %class.cOwnedObject.base, i16, i16, i16, %class.cArray*, %class.cObject*, i8*, i32, i32, i32, i32, %class.SimTime, %class.SimTime, %class.SimTime, %class.SimTime, i32, i64, i64, i64, i64 }
%class.cArray = type { %class.cOwnedObject.base, %class.cObject**, i32, i32, i32, i32 }
%class.SimTime = type { i64 }
%class.cSimulation = type { %class.cNoncopyableOwnedObject.base, i32, i32, %class.cModule**, i32, %class.cEnvir*, %class.cModule*, %class.cSimpleModule*, %class.cComponent*, i32, %class.cModuleType*, %class.cScheduler*, %class.SimTime, i64, %class.cMessage*, %class.cException*, %class.cHasher*, %class.cMessageHeap }
%class.cModule = type { %class.cComponent, i8*, i32, %class.cModule*, %class.cModule*, %class.cModule*, %class.cModule*, i32, %"struct.cGate::Desc"*, i32, i32 }
%class.cComponent = type { %class.cDefaultList, %class.cComponentType*, i16, i32*, i16, i16, %class.cPar*, %class.cDisplayString* }
%class.cNoncopyableOwnedObject.base = type { %class.cOwnedObject.base }
%class.cEnvir = type { i32 (...)**, i8, i8, i8, %"class.std::basic_ostream" }
%class.cSimpleModule = type { %class.cModule, %class.cMessage*, %class.cCoroutine* }
%class.cModuleType = type { %class.cComponentType }
%class.cScheduler = type { %class.cObject, %class.cSimulation* }
%class.cException = type <{ %"class.std::exception", i32, [4 x i8], %"class.std::basic_string", i8, [7 x i8], %"class.std::basic_string", %"class.std::basic_string", i32, [4 x i8] }>
%class.cHasher = type { i32 }
%class.cGate = type { %class.cObject, %"struct.cGate::Desc"*, i32, %class.cChannel*, %class.cGate*, %class.cGate* }
%"struct.cGate::Desc" = type { %class.cModule*, %"struct.cGate::Name"*, i32, %union.anon.112, %union.anon.112 }
%"struct.cGate::Name" = type <{ %class.opp_string, %class.opp_string, %class.opp_string, i32, [4 x i8] }>
%class.cDefaultList = type { %class.cNoncopyableOwnedObject.base, %class.cOwnedObject**, i32, i32 }
%class.cComponentType = type { %class.cNoncopyableOwnedObject.base, %"class.std::basic_string", %"class.std::map", %"class.std::set" }
%class.cPar = type { %class.cObject, %class.cComponent*, %class.cParImpl* }
%class.cParImpl = type { %class.cNamedObject.base, i8* }
%class.cDisplayString = type { i8*, i8*, %"struct.cDisplayString::Tag"*, i32, i8*, i8, %class.cComponent* }
%"struct.cDisplayString::Tag" = type { i8*, i32, [16 x i8*] }
%"class.std::basic_ostream.base" = type { i32 (...)** }
%class.cCoroutine = type { i32 (...)**, %struct._Task* }
%struct._Task = type { i64, [1 x %struct.__jmp_buf_tag], [1 x %struct.__jmp_buf_tag], i32, i32, %struct._Task*, void (i8*)*, i8*, i32, %struct._Task*, i64 }
%struct.__jmp_buf_tag = type { [8 x i64], i32, %struct.__sigset_t }
%struct.__sigset_t = type { [16 x i64] }
%"class.std::exception" = type { i32 (...)** }
%"class.std::basic_string" = type { %"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" }
%"struct.std::basic_string<char, std::char_traits<char>, std::allocator<char> >::_Alloc_hider" = type { i8* }
%union.anon.112 = type { %class.cGate* }
%"class.std::map" = type { %"class.std::_Rb_tree" }
%"class.std::set" = type { %"class.std::_Rb_tree" }
%class.opp_string_map = type { %"class.std::map" }
%"class.std::_Rb_tree" = type { i32 (...)** }
%class.cSequentialScheduler = type { %class.cScheduler }
%class.opp_string = type { i8* }
%class.cChannel = type <{ %class.cComponent, %class.cGate*, i32, [4 x i8] }>
%class.cRealTimeScheduler = type { %class.cScheduler, i8, double, %struct.timeval }
%struct.timeval = type { i64, i64 }
%class.cOwnedObject = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32, [4 x i8] }>
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque

declare dso_local %class.cMessage* @_ZN18cRealTimeScheduler12getNextEventEv(%class.cRealTimeScheduler* nocapture readonly) unnamed_addr #0 align 2

declare dso_local %class.cMessage* @_ZN20cSequentialScheduler12getNextEventEv(%class.cSequentialScheduler* nocapture readonly) unnamed_addr #0 align 2

declare dso_local i32 @_ZgtR8cMessageS0_(%class.cMessage* dereferenceable(160) %a, %class.cMessage* dereferenceable(160) %b)

declare dso_local i32 @__gxx_personality_v0(...)

declare void @llvm.assume(i1) #2

declare i1 @llvm.intel.wholeprogramsafe() #2

declare i1 @llvm.type.test(i8*, metadata) #6

define dso_local %class.cSimpleModule* @_ZN11cSimulation16selectNextModuleEv(%class.cSimulation*) #0 align 2 {
  %2 = getelementptr inbounds %class.cSimulation, %class.cSimulation* %0, i64 0, i32 11
  %3 = getelementptr inbounds %class.cSimulation, %class.cSimulation* %0, i64 0, i32 3
  %4 = getelementptr inbounds %class.cSimulation, %class.cSimulation* %0, i64 0, i32 17
  br label %5

; <label>:5:                                      ; preds = %45, %1
  %6 = load %class.cScheduler*, %class.cScheduler** %2, align 8
  %7 = bitcast %class.cScheduler* %6 to %class.cMessage* (%class.cScheduler*)***
  %8 = load %class.cMessage* (%class.cScheduler*)**, %class.cMessage* (%class.cScheduler*)*** %7, align 8
  %9 = getelementptr inbounds %class.cMessage* (%class.cScheduler*)*, %class.cMessage* (%class.cScheduler*)** %8, i64 23
  %10 = load %class.cMessage* (%class.cScheduler*)*, %class.cMessage* (%class.cScheduler*)** %9, align 8
  %11 = bitcast %class.cMessage* (%class.cScheduler*)* %10 to i8*
  %12 = bitcast %class.cMessage* (%class.cRealTimeScheduler*)* @_ZN18cRealTimeScheduler12getNextEventEv to i8*
  %13 = icmp eq i8* %11, %12
  br i1 %13, label %14, label %16

; <label>:14:                                     ; preds = %5
  %15 = tail call %class.cMessage* bitcast (%class.cMessage* (%class.cRealTimeScheduler*)* @_ZN18cRealTimeScheduler12getNextEventEv to %class.cMessage* (%class.cScheduler*)*)(%class.cScheduler* %6)
  br label %18

; <label>:16:                                     ; preds = %5
  %17 = tail call %class.cMessage* bitcast (%class.cMessage* (%class.cSequentialScheduler*)* @_ZN20cSequentialScheduler12getNextEventEv to %class.cMessage* (%class.cScheduler*)*)(%class.cScheduler* %6)
  br label %18

; <label>:18:                                     ; preds = %16, %14
  %19 = phi %class.cMessage* [ %15, %14 ], [ %17, %16 ]
  br label %20

; <label>:20:                                     ; preds = %18
  %21 = icmp eq %class.cMessage* %19, null
  br i1 %21, label %53, label %22

; <label>:22:                                     ; preds = %20
  %23 = load %class.cModule**, %class.cModule*** %3, align 8
  %24 = getelementptr inbounds %class.cMessage, %class.cMessage* %19, i64 0, i32 9
  %25 = load i32, i32* %24, align 8
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds %class.cModule*, %class.cModule** %23, i64 %26
  %28 = bitcast %class.cModule** %27 to %class.cSimpleModule**
  %29 = load %class.cSimpleModule*, %class.cSimpleModule** %28, align 8
  %30 = icmp eq %class.cSimpleModule* %29, null
  br i1 %30, label %37, label %31

; <label>:31:                                     ; preds = %22
  %32 = bitcast %class.cSimpleModule* %29 to %class.cNamedObject*
  %33 = getelementptr inbounds %class.cNamedObject, %class.cNamedObject* %32, i64 0, i32 2
  %34 = load i16, i16* %33, align 8
  %35 = and i16 %34, 1024
  %36 = icmp eq i16 %35, 0
  br i1 %36, label %46, label %37

; <label>:37:                                     ; preds = %31, %22
  %38 = tail call %class.cMessage* @_ZN12cMessageHeap11removeFirstEv(%class.cMessageHeap* nonnull %4)
  %39 = icmp eq %class.cMessage* %38, null
  br i1 %39, label %46, label %40

; <label>:40:                                     ; preds = %37
  %t38 = tail call %class.cMessage* @_ZN12cMessageHeap11removeFirstEv(%class.cMessageHeap* nonnull %4)
  %t39 = icmp eq %class.cMessage* %t38, null
  br i1 %39, label %46, label %41

; <label>:41:                                     ; preds = %40
  %42 = bitcast %class.cMessage* %38 to void (%class.cMessage*)***
  %43 = load void (%class.cMessage*)**, void (%class.cMessage*)*** %42, align 8
  %44 = getelementptr inbounds void (%class.cMessage*)*, void (%class.cMessage*)** %43, i64 4
  %45 = load void (%class.cMessage*)*, void (%class.cMessage*)** %44, align 8
  tail call void %45(%class.cMessage* nonnull %38)
  br label %46

; <label>:46:                                     ; preds = %40, %37
  br label %5

; <label>:47:                                     ; preds = %31
  %48 = getelementptr inbounds %class.cMessage, %class.cMessage* %19, i64 0, i32 13
  %49 = getelementptr inbounds %class.SimTime, %class.SimTime* %48, i64 0, i32 0
  %50 = load i64, i64* %49, align 8
  %51 = getelementptr inbounds %class.cSimulation, %class.cSimulation* %0, i64 0, i32 12
  %52 = getelementptr inbounds %class.SimTime, %class.SimTime* %51, i64 0, i32 0
  store i64 %50, i64* %52, align 8
  br label %53

; <label>:53:                                     ; preds = %47, %20
  %54 = phi %class.cSimpleModule* [ %29, %47 ], [ null, %20 ]
  ret %class.cSimpleModule* %54
}

define internal %class.cMessage* @_ZN12cMessageHeap11removeFirstEv(%class.cMessageHeap*) #0 align 2 {
  %2 = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %0, i64 0, i32 2
  %3 = load i32, i32* %2, align 8
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %22

; <label>:5:                                      ; preds = %1
  %6 = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %0, i64 0, i32 1
  %7 = load %class.cMessage**, %class.cMessage*** %6, align 8
  %8 = getelementptr inbounds %class.cMessage*, %class.cMessage** %7, i64 1
  %9 = load %class.cMessage*, %class.cMessage** %8, align 8
  %10 = add nsw i32 %3, -1
  store i32 %10, i32* %2, align 8
  %11 = sext i32 %3 to i64
  %12 = getelementptr inbounds %class.cMessage*, %class.cMessage** %7, i64 %11
  %13 = load %class.cMessage*, %class.cMessage** %12, align 8
  store %class.cMessage* %13, %class.cMessage** %8, align 8
  %14 = getelementptr inbounds %class.cMessage, %class.cMessage* %13, i64 0, i32 15
  store i32 1, i32* %14, align 8
  tail call void @_ZN12cMessageHeap7shiftupEi(%class.cMessageHeap* nonnull %0, i32 1)
  %15 = getelementptr inbounds %class.cMessageHeap, %class.cMessageHeap* %0, i64 0, i32 0, i32 0, i32 0
  %16 = bitcast %class.cMessage* %9 to %class.cOwnedObject*
  %17 = bitcast %class.cMessageHeap* %0 to void (%class.cObject*, %class.cOwnedObject*)***
  %18 = load void (%class.cObject*, %class.cOwnedObject*)**, void (%class.cObject*, %class.cOwnedObject*)*** %17, align 8
  %19 = getelementptr inbounds void (%class.cObject*, %class.cOwnedObject*)*, void (%class.cObject*, %class.cOwnedObject*)** %18, i64 13
  %20 = load void (%class.cObject*, %class.cOwnedObject*)*, void (%class.cObject*, %class.cOwnedObject*)** %19, align 8
  tail call void %20(%class.cObject* %15, %class.cOwnedObject* %16)
  %21 = getelementptr inbounds %class.cMessage, %class.cMessage* %9, i64 0, i32 15
  store i32 -1, i32* %21, align 8
  br label %22

; <label>:22:                                     ; preds = %5, %1
  %23 = phi %class.cMessage* [ %9, %5 ], [ null, %1 ]
  ret %class.cMessage* %23
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
