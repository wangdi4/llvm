; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

; This test checks that _ZN6MemorydlEPv() is recognized as special kind of
; user-defined free() with stored memory manager pointer.

; CHECK: Analyzing for FreePostDom _ZN10MemManager10deallocateEPv
; CHECK: Is FreePostDom _ZN10MemManager10deallocateEPv
; CHECK: Analyzing for FreeWithStoredMMPtr _ZN6MemorydlEPv
; CHECK: Is FreeWithStoredMMPtr _ZN6MemorydlEPv


%class.MemManager = type { i32 (...)** }
%class.bad_alloc = type { %class.exception }
%class.exception = type { i32 (...)** }

@_ZTISt9bad_alloc = external dso_local constant i8*
@_ZTVSt9bad_alloc = available_externally dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD0Ev to i8*), i8* bitcast (i8* (%class.bad_alloc*)* @_ZNKSt9bad_alloc4whatEv to i8*)] }, align 8

define dso_local void @_ZN10MemManager10deallocateEPv(%class.MemManager* nocapture readnone %this, i8* %p) unnamed_addr align 2 {
entry:
  tail call void @_ZdlPv(i8* %p)
  ret void
}


declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

define internal void @dummyDeallocateEPv(%class.MemManager* nocapture readnone, i8* nocapture readnone) unnamed_addr align 2 {
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*))
  unreachable
}

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

declare dso_local void @_ZNSt9bad_allocD1Ev(%class.bad_alloc*) unnamed_addr

declare dso_local nonnull i8* @_ZNKSt9bad_alloc4whatEv(%class.bad_alloc*) unnamed_addr

declare dso_local void @_ZNSt9bad_allocD0Ev(%class.bad_alloc*) unnamed_addr

define internal void @_ZN6MemorydlEPv(i8* %ptr) align 2 {
entry:
  %cmp1 = icmp eq i8* %ptr, null
  br i1 %cmp1, label %if.then, label %if.else

if.else:                                      ; preds = %entry
  %gep1 = getelementptr inbounds i8, i8* %ptr, i64 -8
  %bc1 = bitcast i8* %gep1 to %class.MemManager**
  %ld1 = load %class.MemManager*, %class.MemManager** %bc1, align 8
  %bc2 = bitcast %class.MemManager* %ld1 to void (%class.MemManager*, i8*)***
  %ld2 = load void (%class.MemManager*, i8*)**, void (%class.MemManager*, i8*)*** %bc2, align 8
  %gep2 = getelementptr inbounds void (%class.MemManager*, i8*)*, void (%class.MemManager*, i8*)** %ld2, i64 3
  %ld3 = load void (%class.MemManager*, i8*)*, void (%class.MemManager*, i8*)** %gep2, align 8
  %bc4 = bitcast void (%class.MemManager*, i8*)* %ld3 to i8*
  %bc5 = bitcast void (%class.MemManager*, i8*)* @_ZN10MemManager10deallocateEPv to i8*
  %cmp2 = icmp eq i8* %bc4, %bc5
  br i1 %cmp2, label %if.then.1, label %if.else.1

if.then.1:                                     ; preds = %if.else
  tail call void bitcast (void (%class.MemManager*, i8*)* @_ZN10MemManager10deallocateEPv to void (%class.MemManager*, i8*)*)(%class.MemManager* %ld1, i8* nonnull %gep1)
  br label %merge

if.else.1:                                     ; preds = %if.else
  tail call void bitcast (void (%class.MemManager*, i8*)* @dummyDeallocateEPv to void (%class.MemManager*, i8*)*)(%class.MemManager* %ld1, i8* nonnull %gep1)
  br label %merge

merge:                                         ; preds = %if.else.1, %if.then.1
  br label %next

next:                                          ; preds = %merge
  br label %if.then

if.then:                                      ; preds = %nex, %entry
  ret void
}
