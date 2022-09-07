; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks how call to the non-return function versus regular function
; influences setting 'unsafe pointer merge' on the type of interest.


%struct.good = type { i32, i32 }
%struct.bad = type { i32, i32 }
%struct.S = type { i32, i32 }
%struct.bad_alloc = type { %struct.exception }
%struct.exception = type { i32 (...)** }

@g_good = external dso_local global %struct.good*, align 8
@g_bad = external dso_local global %struct.bad*, align 8
@g_S = external dso_local global %struct.S, align 8
@_ZTISt9bad_alloc = external dso_local constant i8*
@_ZTVSt9bad_alloc = available_externally dso_local unnamed_addr constant { [5 x i8*] } { [5 x i8*] [i8* null, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%struct.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*), i8* bitcast (void (%struct.bad_alloc*)* @_ZNSt9bad_allocD0Ev to i8*), i8* bitcast (i8* (%struct.bad_alloc*)* @_ZNKSt9bad_alloc4whatEv to i8*)] }, align 8


; type.good should not get 'unsafe pointer merge' since @dummy() is
; a non-return function.
define internal i32 @foo1 (i32 %arg) {
  %cmp = icmp eq i32 %arg, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:
  %call1 = call i8* @dummy(%struct.S* @g_S, i64 16)
  br label %merge

if.else:
  %call2 = tail call i8* @malloc(i64 16)
  br label %merge

merge:
  %phi = phi i8* [ %call1, %if.then ], [ %call2, %if.else ]
  %bc = bitcast i8* %phi to %struct.good*
  store %struct.good* %bc, %struct.good** @g_good, align 8
  ret i32 0
}

; type.bad should get 'unsafe pointer merge' since @not_dummy() is
; a regular function.
define internal i32 @foo2 (i32 %arg) {
  %cmp = icmp eq i32 %arg, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:
  %call1 = call i8* @not_dummy(%struct.S* @g_S, i64 16)
  br label %merge

if.else:
  %call2 = tail call i8* @malloc(i64 16)
  br label %merge

merge:
  %phi = phi i8* [ %call1, %if.then ], [ %call2, %if.else ]
  %bc = bitcast i8* %phi to %struct.bad*
  store %struct.bad* %bc, %struct.bad** @g_bad, align 8
  ret i32 0
}

define internal i8* @dummy (%struct.S* %this, i64 %size) {
  entry:
  %call = tail call i8* @__cxa_allocate_exception(i64 8)
  %bc = bitcast i8* %call to %struct.bad_alloc*
  %gep = getelementptr inbounds %struct.bad_alloc, %struct.bad_alloc* %bc, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9bad_alloc, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %gep, align 8
  tail call void @__cxa_throw(i8* nonnull %call, i8* bitcast (i8** @_ZTISt9bad_alloc to i8*), i8* bitcast (void (%struct.bad_alloc*)* @_ZNSt9bad_allocD1Ev to i8*))
  unreachable
}

define internal i8* @not_dummy (%struct.S* %this, i64 %size) {
  %bc = inttoptr i64 %size to i8*
  ret i8* %bc
}

$__clang_call_terminate = comdat any

$_ZTS2Ex = comdat any

$_ZTI2Ex = comdat any

declare dso_local void @_ZNSt9bad_allocD1Ev(%struct.bad_alloc*) unnamed_addr

declare dso_local nonnull i8* @_ZNKSt9bad_alloc4whatEv(%struct.bad_alloc*) unnamed_addr

declare dso_local void @_ZNSt9bad_allocD0Ev(%struct.bad_alloc*) unnamed_addr

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

declare noalias i8* @malloc(i64)

; CHECK-LABEL:  LLVMType: %struct.bad = type { i32, i32 }
; CHECK:  Name: struct.bad
; CHECK:  Number of fields: 2
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Field info:
; CHECK:    Frequency: 0
; CHECK:    Multiple Value: [  ] <incomplete>
; CHECK:    Bottom Alloc Function
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Field info:
; CHECK:    Frequency: 0
; CHECK:    Multiple Value: [  ] <incomplete>
; CHECK:    Bottom Alloc Function
; CHECK:  Total Frequency: 0
; CHECK:  Call graph: top
; CHECK:  Safety data: Unsafe pointer merge

; CHECK-LABEL:  LLVMType: %struct.good = type { i32, i32 }
; CHECK:  Name: struct.good
; CHECK:  Number of fields: 2
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Field info:
; CHECK:    Frequency: 0
; CHECK:    No Value
; CHECK:    Top Alloc Function
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Field info:
; CHECK:    Frequency: 0
; CHECK:    No Value
; CHECK:    Top Alloc Function
; CHECK:  Total Frequency: 0
; CHECK:  Call graph: top
; CHECK:  Safety data: No issues found
