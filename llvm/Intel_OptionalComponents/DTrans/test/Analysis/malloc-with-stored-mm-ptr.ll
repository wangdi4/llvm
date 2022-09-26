; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output 2>&1 | FileCheck %s

; This test checks that _ZN6MemorynwEmPNS_13MemoryManager() is recognized as
; special kind of user-defined malloc() with stored memory manager pointer.

; CHECK: Analyzing for MallocPostDom _ZN10MemManager8allocateEl
; CHECK: Is MallocPostDom _ZN10MemManager8allocateEl
; CHECK: Analyzing for MallocWithStoredMMPtr _ZN6MemorynwEmPNS_13MemoryManager
; CHECK: Is MallocWithStoredMMPtr _ZN6MemorynwEmPNS_13MemoryManager

%class.MemManager = type { i32 (...)** }
%struct.Mem = type { i32 (...) ** }
%struct.MemImpl = type { %struct.Mem*, i32, i32, i8** }
%struct.bad_alloc = type { %struct.exception }
%struct.exception = type { i32 (...)** }

@_ZTISt9bad_alloc = external dso_local constant i8*
@_ZTVSt9bad_alloc = available_externally dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%struct.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*), i8* bitcast (void (%struct.bad_alloc*)* @_ZNSt9bad_allocD0Ev to i8*), i8* bitcast (i8* (%struct.bad_alloc*)* @_ZNKSt9bad_alloc4whatEv to i8*)] }, align 8

$__clang_call_terminate = comdat any

$_ZTS2Ex = comdat any

$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_ZTS2Ex, i32 0, i32 0) }, comdat, align 8

; Check that allocate() function is recognized as malloc post dom.
; Used-defined malloc function could have two arguments. First argument should be a pointer to the structure and never used.
; Second argument should be an integer allocation size.
define dso_local noalias nonnull i8* @_ZN10MemManager8allocateEl(%class.MemManager* nocapture readnone %this, i64 %size) unnamed_addr align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
; invoke instruction has implicit 'nobuiltin' attribute.
  %call = invoke i8* @_Znwm(i64 %size)
          to label %if.then unwind label %lpad

lpad:                                             ; preds = %entry
  %lp1 = landingpad { i8*, i32 }
          catch i8* null
  %ext1 = extractvalue { i8*, i32 } %lp1, 0
  %call1 = tail call i8* @__cxa_begin_catch(i8* %ext1)
  %exception = tail call i8* @__cxa_allocate_exception(i64 1)
  invoke void @__cxa_throw(i8* nonnull %exception, i8* bitcast ({ i8*, i8* }* @_ZTI2Ex to i8*), i8* null)
          to label %unreachable unwind label %lpad2

lpad2:                                            ; preds = %lpad
  %lp2 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:                                     ; preds = %lpad2
  %ext2 = extractvalue { i8*, i32 } %lp2, 0
  call void @_Unwind_Resume(i8* %ext2)
  unreachable

if.then:                                          ; preds = %entry
  ret i8* %call

terminate.lpad:                                   ; preds = %lpad2
  %lp3 = landingpad { i8*, i32 }
          catch i8* null
  %ext3 = extractvalue { i8*, i32 } %lp3, 0
  tail call void @__clang_call_terminate(i8* %ext3)
  unreachable

unreachable:                                      ; preds = %lpad
  unreachable
}

declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

declare dso_local void @__cxa_end_catch() local_unnamed_addr

declare dso_local void @_ZSt9terminatev() local_unnamed_addr

define linkonce_odr hidden void @__clang_call_terminate(i8* %ptr) local_unnamed_addr comdat {
  %call = tail call i8* @__cxa_begin_catch(i8* %ptr)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare void @_Unwind_Resume(i8*)

declare dso_local void @_ZNSt9bad_allocD1Ev(%struct.bad_alloc*) unnamed_addr

declare dso_local nonnull i8* @_ZNKSt9bad_alloc4whatEv(%struct.bad_alloc*) unnamed_addr

declare dso_local void @_ZNSt9bad_allocD0Ev(%struct.bad_alloc*) unnamed_addr

define internal i8* @dummyAlloc (%struct.MemImpl* %this, i32 %conv4) {
entry:
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %struct.bad_alloc*
  %gep = getelementptr inbounds %struct.bad_alloc, %struct.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%struct.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*))
  unreachable
}

declare noalias i8* @malloc(i32)

; Function Attrs: uwtable
define internal nonnull i8* @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 %size, %class.MemManager* %mm) align 2 {
entry:
  %new.size = add i64 %size, 8
  %bc1 = bitcast %class.MemManager* %mm to i8* (%class.MemManager*, i64)***
  %ld1 = load i8* (%class.MemManager*, i64)**, i8* (%class.MemManager*, i64)*** %bc1, align 8
  %gep1 = getelementptr inbounds i8* (%class.MemManager*, i64)*, i8* (%class.MemManager*, i64)** %ld1, i64 2
  %ld2 = load i8* (%class.MemManager*, i64)*, i8* (%class.MemManager*, i64)** %gep1, align 8
  %bc2 = bitcast i8* (%class.MemManager*, i64)* %ld2 to i8*
  %bc3 = bitcast i8* (%class.MemManager*, i64)* @_ZN10MemManager8allocateEl to i8*
  %cmp = icmp eq i8* %bc2, %bc3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                     ; preds = %entry
  %alloc1 = tail call i8* bitcast (i8* (%class.MemManager*, i64)* @_ZN10MemManager8allocateEl to i8* (%class.MemManager*, i64)*)(%class.MemManager* nonnull %mm, i64 %new.size)
  br label %merge

if.else:                                     ; preds = %entry
  %alloc2 = tail call i8* bitcast (i8* (%struct.MemImpl*, i32)* @dummyAlloc to i8* (%class.MemManager*, i64)*)(%class.MemManager* nonnull %mm, i64 %new.size)
  br label %merge

merge:                                     ; preds = %if.else, %if.then
  %phi = phi i8* [ %alloc1, %if.then ], [ %alloc2, %if.else ]
  br label %return

return:                                     ; preds = %merge
  %bc4 = bitcast i8* %phi to %class.MemManager**
  store %class.MemManager* %mm, %class.MemManager** %bc4, align 8
  %gep2 = getelementptr inbounds i8, i8* %phi, i64 8
  ret i8* %gep2
}

