; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume  -passes='require<dtransanalysis>'  -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s

; This test verifies allocation call info collection for the transforms from
; memory allocation calls.

%struct.test01 = type { i32, i32, i32 }
define void @test01user() {
  %call = call i8* @test01a(i64 144)
  %p = bitcast i8* %call to %struct.test01*
  %f0 = getelementptr %struct.test01, %struct.test01* %p, i64 0, i32 0
  store i32 0, i32* %f0
  call void @test01f(i8* %call)
  ret void
}
; CHECK: Function: test01user
; CHECK: Instruction:   %call = call {{.*}} @test01a
; CHECK: AllocCallInfo:
; CHECK:   Kind: UserMalloc{{ *}}

; CHECK: Function: test01user
; CHECK: Instruction:   call void @test01f
; CHECK: FreeCallInfo:
; CHECK:   Kind: UserFree{{ *}}

; Pattern for AK_UserAlloc
define i8* @test01a(i64 %size) {
  %call = tail call i8* @malloc(i64 %size)
  ret i8* %call
}

; Pattern for FK_UserFree
define void @test01f(i8* %ptr) {
  call void @free(i8* %ptr)
  ret void
}

%struct.test02 = type { i32, i32, i32 }
%class.manager02 = type { i8*(...)* }
define void @test02user() {
 %manager = alloca %class.manager02
  %call = call i8* @test02a(%class.manager02* %manager, i64 144)
  %p = bitcast i8* %call to %struct.test02*
  %f0 = getelementptr %struct.test02, %struct.test02* %p, i64 0, i32 0
  store i32 0, i32* %f0
  call void @test02f(%class.manager02* %manager, i8* %call)
  ret void
}
; CHECK: Function: test02user
; CHECK: Instruction:   %call = call {{.*}} @test02a
; CHECK: AllocCallInfo:
; CHECK:   Kind: UserMallocThis{{ *}}

; CHECK: Function: test02user
; CHECK: Instruction:   call void @test02f
; CHECK: FreeCallInfo:
; CHECK:   Kind: UserFreeThis{{ *}}

; Pattern for AK_UserAllocThis
define i8* @test02a(%class.manager02* %manager, i64 %size) {
  %call = tail call i8* @malloc(i64 %size)
  ret i8* %call
}

; Pattern for FK_UserFreeThis
define void @test02f(%class.manager02* %manager, i8* %ptr) {
  call void @free(i8* %ptr)
  ret void
}

; Special case for UserMalloc0 type.
%class.MemManager = type { i32 (...)** }
%struct.Mem = type { i32 (...) ** }
%struct.MemImpl = type { %struct.Mem*, i32, i32, i8** }
%class.bad_alloc = type { %class.exception }
%class.exception = type { i32 (...)** }

@_ZTISt9bad_alloc = external dso_local constant i8*
@_ZTVSt9bad_alloc = available_externally dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD0Ev to i8*), i8* bitcast (i8* (%class.bad_alloc*)* @_ZNKSt9bad_alloc4whatEv to i8*)] }, align 8

$__clang_call_terminate = comdat any
$_ZTS2Ex = comdat any
$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_ZTS2Ex, i32 0, i32 0) }, comdat, align 8

%struct.test = type { i64, i64 }
define void  @test03user() {
  %mm = alloca %class.MemManager
  %res = call i8* @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 16,  %class.MemManager* %mm)
  %bc = bitcast i8* %res to %struct.test*
  %f0 = getelementptr %struct.test, %struct.test* %bc, i64 0, i32 0
  store i64 0, i64* %f0
  call void @_ZN6MemorydlEPv(i8* %res)
  ret void
}
; CHECK: Function: test03user
; CHECK: Instruction:   %res = call i8* @_ZN6MemorynwEmPNS_13MemoryManagerE(i64 16, %class.MemManager* %mm)
; CHECK: AllocCallInfo:
; CHECK:   Kind: UserMalloc0{{ *}}

; CHECK: Function: test03user
; CHECK: Instruction:   call void @_ZN6MemorydlEPv(i8* %res)
; CHECK: FreeCallInfo:
  ; CHECK: Kind: UserFree0{{ *}}


; Check that allocate() function is recognized as malloc post dom.
; Used-defined malloc function could have two arguments.
; First argument should be a pointer to the structure and never used.
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
define internal i8* @dummyAlloc (%struct.MemImpl* %this, i32 %conv4) {
entry:
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*))
  unreachable
}

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

define dso_local void @_ZN10MemManager10deallocateEPv(%class.MemManager* nocapture readnone %this, i8* %p) unnamed_addr align 2 {
entry:
  tail call void @_ZdlPv(i8* %p)
  ret void
}

define internal void @dummyDeallocateEPv(%class.MemManager* nocapture readnone, i8* nocapture readnone) unnamed_addr align 2 {
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %class.bad_alloc*
  %gep = getelementptr inbounds %class.bad_alloc, %class.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%class.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*))
  unreachable
}

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

define linkonce_odr hidden void @__clang_call_terminate(i8* %ptr) local_unnamed_addr comdat {
  %call = tail call i8* @__cxa_begin_catch(i8* %ptr)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare noalias i8* @malloc(i64)
declare void @free(i8*)

declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr
declare dso_local void @_ZdlPv(i8*) local_unnamed_addr

declare dso_local i32 @__gxx_personality_v0(...)
declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr
declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr
declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr
declare dso_local void @__cxa_end_catch() local_unnamed_addr
declare dso_local void @_ZSt9terminatev() local_unnamed_addr
declare void @_Unwind_Resume(i8*)
declare dso_local void @_ZNSt9bad_allocD1Ev(%class.bad_alloc*) unnamed_addr
declare dso_local nonnull i8* @_ZNKSt9bad_alloc4whatEv(%class.bad_alloc*) unnamed_addr
declare dso_local void @_ZNSt9bad_allocD0Ev(%class.bad_alloc*) unnamed_addr
