; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case checks that devirtualization won't happen since there is a
; downcasting. The issue with downcasting is that the pointer can reference
; memory that could be allocated with another type. If we devirtualize these
; calls then it could produce an incorrect target.
;
; This example can be seen in C++ as follow:
;
; int var = 0;
;
; class Base {
;   public:
;     virtual void foo() = 0;
; };
;
; class DerivedA : public Base {
;
;   public:
;     void foo() { var = 1}
;
; };
;
; class DerivedB : public Base {
;
;   public:
;     void foo() { var = 2}
;
; };
;
; void bar(Base *p){
;   DerivedB *ptr = (Derived *) p;
;   ptr->foo();
; }
;
; The downcasting in bar is converting p from Base to DerivedB. The issue
; is that p can point to a DerivedA object. The devirtualization process
; can convert ptr->foo() into DevirtB::foo() and that is incorrect.

; RUN: opt -S -passes=wholeprogramdevirt -wholeprogramdevirt-multiversion=true -whole-program-assume %s | FileCheck %s

; Check that the indirect call wasn't converted into a direct call
; CHECK: %tmp6 = tail call i32 %tmp5(%class.DerivedB* %tmp)

; Check that the direct call wasn't generated
; CHECK-NOT: %tmp6 = tail call i32 bitcast (void (%class.DerivedB*)* @_ZN8DerivedB3fooEv to i32 (%class.DerivedB*)*)(%class.DerivedB* %tmp), !_Intel.Devirt.Call !9

%class.Base = type { i32 (...)** }
%class.DerivedA = type { %class.Base }
%class.DerivedB = type { %class.Base }

$_ZTI4Base = comdat any
$_ZTS4Base = comdat any

; Information related to the V-Table of Base
@_ZTI4Base = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, comdat, align 8
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = internal constant [6 x i8] c"4Base\00", comdat, align 1


; Information related to the V-Table of Devived A
@_ZTV8DerivedA = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8DerivedA to i8*), i8* bitcast (void (%class.DerivedA*)* @_ZN8DerivedA3fooEv to i8*)] }, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTI8DerivedA = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8DerivedA, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, align 8
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS8DerivedA = internal constant [10 x i8] c"8DerivedA\00", align 1

; Information related to V-Table of Derived B
@_ZTV8DerivedB = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8DerivedB to i8*), i8* bitcast (void (%class.DerivedB*)* @_ZN8DerivedB3fooEv to i8*)] }, align 8, !type !0, !type !1, !type !4, !type !5
@_ZTI8DerivedB = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8DerivedB, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, align 8
@_ZTS8DerivedB = internal constant [10 x i8] c"8DerivedB\00", align 1

declare i1 @llvm.type.test(i8*, metadata) #6
declare void @llvm.assume(i1)

@var = dso_local global i32 0

; DerivedA::foo
define internal void @_ZN8DerivedA3fooEv(%class.DerivedA* nocapture readnone) {
  store i32 1, i32* @var
  ret void
}

; DerivedB:foo
define internal void @_ZN8DerivedB3fooEv(%class.DerivedB* nocapture readnone) {
  store i32 2, i32* @var
  ret void
}

define internal void @_Z3barP4Base(%class.Base*) {
  %tmp = bitcast %class.Base* %0 to %class.DerivedB*

  ; Cast from Base to DerivedB (downcasting)
  %tmp1 = bitcast %class.Base* %0 to i32 (%class.DerivedB*)***

  ; Load the V-Table of DerivedB
  %tmp2 = load i32 (%class.DerivedB*)**, i32 (%class.DerivedB*)*** %tmp1, align 8, !tbaa !6
  %tmp3 = bitcast i32 (%class.DerivedB*)** %tmp2 to i8*
  %tmp4 = tail call i1 @llvm.type.test(i8* %tmp3, metadata !"_ZTS8DerivedB")
  tail call void @llvm.assume(i1 %tmp4)
  %tmp5 = load i32 (%class.DerivedB*)*, i32 (%class.DerivedB*)** %tmp2, align 8

  ; Virtual Call
  %tmp6 = tail call i32 %tmp5(%class.DerivedB* %tmp)
  ret void
}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFivE.virtual"}
!2 = !{i64 16, !"_ZTS8DerivedA"}
!3 = !{i64 16, !"_ZTSM8DerivedAFivE.virtual"}
!4 = !{i64 16, !"_ZTS8DerivedB"}
!5 = !{i64 16, !"_ZTSM8DerivedBFivE.virtual"}
!6 = !{!7, !7, i64 0}
!7 = !{!"vtable pointer", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}

; end INTEL_FEATURE_SW_DTRANS
