; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case checks that the partial inliner was executed for functions that are virtual
; function targets. The test cases translates the following example:
;
; #include <iostream>
;
; class cObject {
; public:
;   virtual ~cObject() {};
;
; };
;
; class Base : public cObject {
; public:
;   virtual bool foo(void *arr, int size) = 0;
; };
;
; class Derived : public Base {
; public:
;   bool foo(void *arr, int size) {
;
;     int *arr_new = (int *)arr;
;
;     if (arr_new == NULL) {
;       return false;
;     }
;
;     for(int i = 0; i < size; i++)
;       arr_new[i] = i;
;
;     return true;
;   }
; };
;
; class Derived2 : public Base {
; public:
;   bool foo(void *arr, int size) {
;
;     int *arr_new = (int *)arr;
;
;     if (arr_new == NULL) {
;       return false;
;     }
;
;     for(int i = 0; i < size; i++)
;       arr_new[i] = i+1;
;
;     return true;
;   }
; };
;
; class ClassFactory {
; private:
;   cObject **Classes;
;
; public:
;   ClassFactory() {
;     Classes = new cObject*[2];
;     Classes[0] = new Derived();
;     Classes[1] = new Derived2();
;   }
;
;   cObject *getPointer(int i) {
;     return Classes[i];
;   }
; };
;
; class Manager {
; protected:
;   int *a;
;   Base *b;
;   int size;
;   ClassFactory Factory;
;
; public:
;   Manager(int inSize) {
;     a = new int[inSize];
;     size = inSize;
;   }
;   void printer() {
;     for (int i = 0; i < 10; i++)
;       std::cout << a[i] << "\n";
;   }
;
;   void setup(int val) {
;     if (val == 0)
;       b = dynamic_cast<Derived *>(Factory.getPointer(0));
;     else
;       b = dynamic_cast<Derived2 *>(Factory.getPointer(1));
;   }
;
;   virtual bool runner(int *A, int Size) {
;     return b->foo(A,Size);
;   }
;
;   void run() {
;      bool res = runner(a, size);
;      if (res)
;        printer();
;   }
; };
;
; int main(int argc, char** argv) {
;
;   Manager *m = new Manager(10);
;   m->setup(argc);
;   m->run();
;
;   return 0;
; }
;
; In this example we have a class called ClassFactory that have an array with multiple
; types. The field Base *b in the class Manager will take the derived pointer depending
; on the input. The function runner will call foo depending on which derived class was
; used. The goal of this test case is to make sure that the devirtualization was done
; correctly and then the partial inliner will partially inline the calls to foo.
;
; The whole program devirtualization will convert Manager::runner as follow (pseudocode):
;
; Manager::runner(int *A, int Size) {
;  bool c;
;  if (&b->foo == &Derived::foo)
;    c = Derived::foo(A,Size);
;  else
;    c = Derived2::foo(A,Size);
;
;  return c;
; }
;
; Then the partial inliner will take care of converting the function as follow:
;
; Manager::runner(int *A, int Size) {
;
;   bool c;
;
;   if (&b->foo == &Derived::foo) {
;     if (A == NULL)
;       c = false;
;     else if (Size == 0)
;       c = true;
;     else
;       c = Derived::foo_outline_function(A,Size);
;   }
;
;   else {
;     if (A == NULL)
;       c = false;
;     else if (Size == 0)
;       c = true;
;     else
;       c =Derived2::foo_outline_function(A,Size);
;   }
;   return c;
; }
;
; Derived::foo_outline_function(int *A, int Size) {
;   for (int i = 0; i < Size; i++) {
;     A[i] = i;
;   }
;   return true;
; }
;
; Derived2::foo_outline_function(int *A, int Size) {
;   for (int i = 0; i < Size; i++) {
;     A[i] = i+1;
;   }
;   return true;
; }
;
; The calls to foo were partially inlined and replaced with the outline versions.
; The important functions of this test case are Manager::runner, Derived::foo and
; Derived2::foo. The test case will go though the following passes:
;
; 1) Whole program analysis: -wholeprogramanalysis -whole-program-assume -internalize -intel-fold-wp-intrinsic
; 2) Simplify call graph: -simplifycfg
; 3) Whole program devirtualization: -wholeprogramdevirt -whole-program-visibility -wholeprogramdevirt-multiversion
;                                    -wholeprogramdevirt-multiversion-verify -wholeprogramdevirt-assume-safe
; 4) Partial inliner: -partial-inliner -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions

; RUN: opt < %s -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes='require<wholeprogram>,module(intel-fold-wp-intrinsic),module(internalize),function(simplifycfg),module(wholeprogramdevirt),function(instnamer),module(partial-inliner)' -whole-program-assume -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify -skip-partial-inlining-cost-analysis -partial-inline-virtual-functions -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }
%class.Manager = type { ptr, ptr, ptr, i32, %class.ClassFactory }
%class.ClassFactory = type { ptr }

$_ZN7Manager6runnerEPii = comdat any

$_ZN7DerivedD0Ev = comdat any

$_ZN7Derived3fooEPvi = comdat any

$_ZN7cObjectD2Ev = comdat any

$_ZN8Derived2D0Ev = comdat any

$_ZN8Derived23fooEPvi = comdat any

$_ZTV7Manager = comdat any

$_ZTS7Manager = comdat any

$_ZTI7Manager = comdat any

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTS7cObject = comdat any

$_ZTI7cObject = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTV8Derived2 = comdat any

$_ZTS8Derived2 = comdat any

$_ZTI8Derived2 = comdat any

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZTV7Manager = linkonce_odr dso_local unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI7Manager, ptr @_ZN7Manager6runnerEPii] }, comdat, align 8, !type !0, !type !1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS7Manager = linkonce_odr dso_local constant [9 x i8] c"7Manager\00", comdat, align 1
@_ZTI7Manager = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS7Manager }, comdat, align 8
@_ZTV7Derived = linkonce_odr dso_local unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7cObjectD2Ev, ptr @_ZN7DerivedD0Ev, ptr @_ZN7Derived3fooEPvi] }, comdat, align 8, !type !2, !type !3, !type !4, !type !5, !type !6, !type !7
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS7Derived = linkonce_odr dso_local constant [9 x i8] c"7Derived\00", comdat, align 1
@_ZTS4Base = linkonce_odr dso_local constant [6 x i8] c"4Base\00", comdat, align 1
@_ZTS7cObject = linkonce_odr dso_local constant [9 x i8] c"7cObject\00", comdat, align 1
@_ZTI7cObject = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS7cObject }, comdat, align 8
@_ZTI4Base = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS4Base, ptr @_ZTI7cObject }, comdat, align 8
@_ZTI7Derived = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI4Base }, comdat, align 8
@_ZTV8Derived2 = linkonce_odr dso_local unnamed_addr constant { [5 x ptr] } { [5 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN7cObjectD2Ev, ptr @_ZN8Derived2D0Ev, ptr @_ZN8Derived23fooEPvi] }, comdat, align 8, !type !2, !type !3, !type !6, !type !7, !type !8, !type !9
@_ZTS8Derived2 = linkonce_odr dso_local constant [10 x i8] c"8Derived2\00", comdat, align 1
@_ZTI8Derived2 = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS8Derived2, ptr @_ZTI4Base }, comdat, align 8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_test.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define dso_local i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #3 personality ptr @__gxx_personality_v0 {
entry:
  %call = tail call ptr @_Znwm(i64 40) #12
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Manager, i64 0, inrange i32 0, i64 2), ptr %call, align 8, !tbaa !12
  %call.i.i5 = invoke ptr @_Znam(i64 16) #12
          to label %call.i.i.noexc unwind label %lpad

call.i.i.noexc:                                   ; preds = %entry
  %Factory.i = getelementptr inbounds i8, ptr %call, i64 32
  store ptr %call.i.i5, ptr %Factory.i, align 8, !tbaa !15
  %call2.i.i6 = invoke ptr @_Znwm(i64 8) #12
          to label %call2.i.i.noexc unwind label %lpad

call2.i.i.noexc:                                  ; preds = %call.i.i.noexc
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV7Derived, i64 0, inrange i32 0, i64 2), ptr %call2.i.i6, align 8, !tbaa !12
  store ptr %call2.i.i6, ptr %call.i.i5, align 8, !tbaa !19
  %call4.i.i7 = invoke ptr @_Znwm(i64 8) #12
          to label %call4.i.i.noexc unwind label %lpad

call4.i.i.noexc:                                  ; preds = %call2.i.i.noexc
  store ptr getelementptr inbounds ({ [5 x ptr] }, ptr @_ZTV8Derived2, i64 0, inrange i32 0, i64 2), ptr %call4.i.i7, align 8, !tbaa !12
  %arrayidx6.i.i = getelementptr inbounds i8, ptr %call.i.i5, i64 8
  store ptr %call4.i.i7, ptr %arrayidx6.i.i, align 8, !tbaa !19
  %call.i8 = invoke ptr @_Znam(i64 40) #12
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %call4.i.i.noexc
  %a.i = getelementptr inbounds i8, ptr %call, i64 8
  store ptr %call.i8, ptr %a.i, align 8, !tbaa !20
  %size.i = getelementptr inbounds i8, ptr %call, i64 24
  store i32 10, ptr %size.i, align 8, !tbaa !24
  %cmp.i = icmp eq i32 %argc, 0
  br i1 %cmp.i, label %dynamic_cast.end.i, label %dynamic_cast.end6.i

dynamic_cast.end.i:                               ; preds = %invoke.cont
  %i9 = tail call ptr @__dynamic_cast(ptr nonnull %call2.i.i6, ptr @_ZTI7cObject, ptr @_ZTI7Derived, i64 0) #2
  %b.i = getelementptr inbounds i8, ptr %call, i64 16
  store ptr %i9, ptr %b.i, align 8, !tbaa !25
  br label %_ZN7Manager5setupEi.exit

dynamic_cast.end6.i:                              ; preds = %invoke.cont
  %i11 = tail call ptr @__dynamic_cast(ptr nonnull %call4.i.i7, ptr @_ZTI7cObject, ptr @_ZTI8Derived2, i64 0) #2
  %b7.i = getelementptr inbounds i8, ptr %call, i64 16
  store ptr %i11, ptr %b7.i, align 8, !tbaa !25
  br label %_ZN7Manager5setupEi.exit

_ZN7Manager5setupEi.exit:                         ; preds = %dynamic_cast.end6.i, %dynamic_cast.end.i
  %i13 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i13, label %whpr.wrap.i, label %whpr.continue.i

whpr.wrap.i:                                      ; preds = %_ZN7Manager5setupEi.exit
  %i14 = tail call i1 @llvm.type.test(ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Manager, i64 0, inrange i32 0, i64 2), metadata !"_ZTS7Manager")
  tail call void @llvm.assume(i1 %i14)
  br label %whpr.continue.i

whpr.continue.i:                                  ; preds = %whpr.wrap.i, %_ZN7Manager5setupEi.exit
  %b.i12 = getelementptr inbounds i8, ptr %call, i64 16
  %i16 = load ptr, ptr %b.i12, align 8, !tbaa !25
  %vtable.i = load ptr, ptr %i16, align 8, !tbaa !12
  %i18 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i18, label %whpr.wrap.i13, label %_ZN7Manager6runnerEPii.exit

whpr.wrap.i13:                                    ; preds = %whpr.continue.i
  %i20 = tail call i1 @llvm.type.test(ptr %vtable.i, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %i20)
  br label %_ZN7Manager6runnerEPii.exit

_ZN7Manager6runnerEPii.exit:                      ; preds = %whpr.wrap.i13, %whpr.continue.i
  %vfn.i = getelementptr inbounds ptr, ptr %vtable.i, i64 2
  %i21 = load ptr, ptr %vfn.i, align 8
  %call.i14 = tail call zeroext i1 %i21(ptr %i16, ptr nonnull %call.i8, i32 10)
  br i1 %call.i14, label %for.body.i.i, label %_ZN7Manager3runEv.exit

for.body.i.i:                                     ; preds = %for.body.i.i, %_ZN7Manager6runnerEPii.exit
  %indvars.iv.i.i = phi i64 [ %indvars.iv.next.i.i, %for.body.i.i ], [ 0, %_ZN7Manager6runnerEPii.exit ]
  %i22 = load ptr, ptr %a.i, align 8, !tbaa !20
  %arrayidx.i.i11 = getelementptr inbounds i32, ptr %i22, i64 %indvars.iv.i.i
  %i23 = load i32, ptr %arrayidx.i.i11, align 4, !tbaa !26
  %call.i.i = tail call dereferenceable(272) ptr @_ZNSolsEi(ptr nonnull @_ZSt4cout, i32 %i23)
  %call1.i.i.i = tail call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %call.i.i, ptr nonnull @.str, i64 1)
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i.i, 1
  %exitcond.i.i = icmp eq i64 %indvars.iv.next.i.i, 10
  br i1 %exitcond.i.i, label %_ZN7Manager3runEv.exit, label %for.body.i.i

_ZN7Manager3runEv.exit:                           ; preds = %for.body.i.i, %_ZN7Manager6runnerEPii.exit
  ret i32 0

lpad:                                             ; preds = %call4.i.i.noexc, %call2.i.i.noexc, %call.i.i.noexc, %entry
  %i24 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZdlPv(ptr nonnull %call) #13
  resume { ptr, i32 } %i24
}

; Function Attrs: nobuiltin
declare dso_local noalias nonnull ptr @_Znwm(i64) local_unnamed_addr #4

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr) local_unnamed_addr #5

; Function Attrs: nobuiltin
declare dso_local noalias nonnull ptr @_Znam(i64) local_unnamed_addr #4

; Function Attrs: uwtable
define linkonce_odr dso_local zeroext i1 @_ZN7Manager6runnerEPii(ptr %this, ptr %A, i32 %Size) unnamed_addr #6 comdat align 2 {
entry:
  %b = getelementptr inbounds %class.Manager, ptr %this, i64 0, i32 2, !intel-tbaa !25
  %i0 = load ptr, ptr %b, align 8, !tbaa !25
  %vtable = load ptr, ptr %i0, align 8, !tbaa !12
  %i3 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i5 = tail call i1 @llvm.type.test(ptr %vtable, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %i5)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i6 = load ptr, ptr %vfn, align 8
  %call = tail call zeroext i1 %i6(ptr %A, i32 %Size)
  ret i1 %call
}

; Function Attrs: inlinehint uwtable
define linkonce_odr dso_local void @_ZN7DerivedD0Ev(ptr %this) unnamed_addr #7 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  tail call void @_ZdlPv(ptr %this) #13
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZN7Derived3fooEPvi(ptr %arr, i32 %size) unnamed_addr #8 comdat align 2 {
entry:
  %cmp = icmp eq ptr %arr, null
  br i1 %cmp, label %cleanup, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %cmp29 = icmp sgt i32 %size, 0
  br i1 %cmp29, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %wide.trip.count = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  %i1 = trunc i64 %indvars.iv to i32
  store i32 %i1, ptr %arrayidx, align 4, !tbaa !26
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.body, %for.cond.preheader, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %for.cond.preheader ], [ true, %for.body ]
  ret i1 %retval.0
}

; Function Attrs: norecurse nounwind uwtable
define linkonce_odr dso_local void @_ZN7cObjectD2Ev(ptr %this) unnamed_addr #8 comdat align 2 {
entry:
  ret void
}

; Function Attrs: inlinehint uwtable
define linkonce_odr dso_local void @_ZN8Derived2D0Ev(ptr %this) unnamed_addr #7 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  tail call void @_ZdlPv(ptr %this) #13
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define linkonce_odr dso_local zeroext i1 @_ZN8Derived23fooEPvi(ptr %arr, i32 %size) unnamed_addr #8 comdat align 2 {
entry:
  %cmp = icmp eq ptr %arr, null
  br i1 %cmp, label %cleanup, label %for.cond.preheader

for.cond.preheader:                               ; preds = %entry
  %cmp29 = icmp sgt i32 %size, 0
  br i1 %cmp29, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %for.cond.preheader
  %wide.trip.count = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  %i1 = trunc i64 %indvars.iv.next to i32
  store i32 %i1, ptr %arrayidx, align 4, !tbaa !26
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %cleanup, label %for.body

cleanup:                                          ; preds = %for.body, %for.cond.preheader, %entry
  %retval.0 = phi i1 [ false, %entry ], [ true, %for.cond.preheader ], [ true, %for.body ]
  ret i1 %retval.0
}

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #9

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #10

; Function Attrs: nounwind readonly
declare dso_local ptr @__dynamic_cast(ptr, ptr, ptr, i64) local_unnamed_addr #11

declare dso_local dereferenceable(272) ptr @_ZNSolsEi(ptr, i32) local_unnamed_addr #0

declare dso_local dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr dereferenceable(272), ptr, i64) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_test.cpp() #6 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZSt8__ioinit)
  %i0 = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZSt8__ioinit, ptr nonnull @__dso_handle) #2
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { inlinehint uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #9 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #10 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #11 = { nounwind readonly }
attributes #12 = { builtin }
attributes #13 = { builtin nounwind }

!llvm.module.flags = !{!10}
!llvm.ident = !{!11}

!0 = !{i64 16, !"_ZTS7Manager"}
!1 = !{i64 16, !"_ZTSM7ManagerFbPiiE.virtual"}
!2 = !{i64 16, !"_ZTS4Base"}
!3 = !{i64 32, !"_ZTSM4BaseFbPviE.virtual"}
!4 = !{i64 16, !"_ZTS7Derived"}
!5 = !{i64 32, !"_ZTSM7DerivedFbPviE.virtual"}
!6 = !{i64 16, !"_ZTS7cObject"}
!7 = !{i64 32, !"_ZTSM7cObjectFbPviE.virtual"}
!8 = !{i64 16, !"_ZTS8Derived2"}
!9 = !{i64 32, !"_ZTSM8Derived2FbPviE.virtual"}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{!"clang version 8.0.0"}
!12 = !{!13, !13, i64 0}
!13 = !{!"vtable pointer", !14, i64 0}
!14 = !{!"Simple C++ TBAA"}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@_ZTS12ClassFactory", !17, i64 0}
!17 = !{!"unspecified pointer", !18, i64 0}
!18 = !{!"omnipotent char", !14, i64 0}
!19 = !{!17, !17, i64 0}
!20 = !{!21, !22, i64 8}
!21 = !{!"struct@_ZTS7Manager", !22, i64 8, !17, i64 16, !23, i64 24, !16, i64 32}
!22 = !{!"pointer@_ZTSPi", !18, i64 0}
!23 = !{!"int", !18, i64 0}
!24 = !{!21, !23, i64 24}
!25 = !{!21, !17, i64 16}
!26 = !{!23, !23, i64 0}

; Check that function Manager::runner was devirtualized correctly. Also check that both targets were
; partially inlined correctly.
;
; CHECK: define internal zeroext i1 @_ZN7Manager6runnerEPii(ptr %this, ptr %A, i32 %Size)
; CHECK: br i1 %{{.*}}, label %BBDevirt__ZN7Derived3fooEPvi, label %BBDevirt__ZN8Derived23fooEPvi
;
; Partially inline (A == null) in Derived::foo
; CHECK-LABEL: BBDevirt__ZN7Derived3fooEPvi:                 ; preds = %entry
; CHECK:         %cmp.i[[V0:[0-9]+]] = icmp eq ptr %A, null
; CHECK-NEXT:    br i1 %cmp.i[[V0]], label %_ZN7Derived3fooEPvi.2.exit, label %for.cond.preheader.i[[V5:[0-9]+]]
;
; Partially inline (Size = 0) in Derived::foo
; CHECK: for.cond.preheader.i[[V5]]:                           ; preds = %BBDevirt__ZN7Derived3fooEPvi
; CHECK:         %cmp29.i[[V1:[0-9]+]] = icmp sgt i32 %Size, 0
; CHECK-NEXT:    br i1 %cmp29.i[[V1]], label %codeRepl.i[[V2:[0-9]+]], label %_ZN7Derived3fooEPvi.2.exit
;
; Call the outline function of Derived::foo
; CHECK: codeRepl.i[[V2]]:                                     ; preds = %for.cond.preheader.i[[V5]]
; CHECK:         call void @_ZN7Derived3fooEPvi.2.for.body.preheader(i32 %Size, ptr %A)
; CHECK-NEXT:    br label %_ZN7Derived3fooEPvi.2.exit
;
; CHECK: _ZN7Derived3fooEPvi.2.exit:                       ; preds = %BBDevirt__ZN7Derived3fooEPvi, %for.cond.preheader.i[[V5]], %codeRepl.i[[V2]]
; CHECK:        %retval.0.i[[V3:[0-9]+]] = phi i1 [ false, %BBDevirt__ZN7Derived3fooEPvi ], [ true, %for.cond.preheader.i[[V5]] ], [ true, %codeRepl.i[[V2]] ]
; CHECK-NEXT:    br label %MergeBB
;
; Partially inline (A == null) in Derived2::foo
; CHECK-LABEL: BBDevirt__ZN8Derived23fooEPvi:                ; preds = %entry
; CHECK:         %cmp.i = icmp eq ptr %A, null
; CHECK-NEXT:    br i1 %cmp.i, label %_ZN8Derived23fooEPvi.1.exit, label %for.cond.preheader.i
;
; Partially inline (Size == 0) in Derived2::foo
; CHECK-LABEL: for.cond.preheader.i:                             ; preds = %BBDevirt__ZN8Derived23fooEPvi
; CHECK:         %cmp29.i = icmp sgt i32 %Size, 0
; CHECK-NEXT:    br i1 %cmp29.i, label %codeRepl.i, label %_ZN8Derived23fooEPvi.1.exit
;
; Call the outline function of Derived2:foo
; CHECK-LABEL: codeRepl.i:                                       ; preds = %for.cond.preheader.i
; CHECK:         call void @_ZN8Derived23fooEPvi.1.for.body.preheader(i32 %Size, ptr %A)
; CHECK-NEXT:    br label %_ZN8Derived23fooEPvi.1.exit
;
; CHECK-LABEL: _ZN8Derived23fooEPvi.1.exit:
; CHECK:    %retval.0.i = phi i1 [ false, %BBDevirt__ZN8Derived23fooEPvi ], [ true, %for.cond.preheader.i ], [ true, %codeRepl.i ]
; CHECK-NEXT:    br label %MergeBB
;
; CHECK-LABEL: MergeBB:                                      ; preds = %_ZN8Derived23fooEPvi.1.exit, %_ZN7Derived3fooEPvi.2.exit
; CHECK-NEXT:    %i[[V4:[0-9]+]] = phi i1 [ %retval.0.i[[V3]], %_ZN7Derived3fooEPvi.2.exit ], [ %retval.0.i, %_ZN8Derived23fooEPvi.1.exit ]
; CHECK-NEXT:    br label %bb

; end INTEL_FEATURE_SW_DTRANS
