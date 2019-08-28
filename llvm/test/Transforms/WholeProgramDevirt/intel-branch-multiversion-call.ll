; This test check that the multiversioning based on virtual function pointers
; was done correctly using CallInst. The test case is pretty simple, this is
; how it looks like in C++:

; class Base {
;  virtual bool foo(int a) = 0;
; };
;
; class Derived : public Base {
;   bool foo(int a) {
;     return true;
;   }
; };
;
; class Derived2 : public Base {
;   bool foo(int a) {
;     return false;
;   }
; };
;
; Base *b;
; Derived d;
; Derived2 d2;
;
; int main(int argc, char* argv[]) {
;
;   if (argc < 2)
;     b = &d;
;   else
;     b = &d2;
;
;   bool c = b->foo(argc);
;   std::cout << c <<"\n";
;   return 0;
; }

; The devirtualization process should generate a multiversioning similar to
; the following (in pseudo-code):

; if (address of b->foo == address of Derived::foo)
;   call Derived::foo
; else if (address of b->foo == address of Derived2::foo)
;   call Derived2::foo
; else
;   call b->foo

; RUN: opt < %s -wholeprogramdevirt -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify -S 2>&1 | FileCheck %s

%"class.std::ios_base::Init" = type { i8 }
%class.Base = type { i32 (...)** }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Base }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }

$_ZN7Derived3fooEi = comdat any

$_ZN8Derived23fooEi = comdat any

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTV8Derived2 = comdat any

$_ZTS8Derived2 = comdat any

$_ZTI8Derived2 = comdat any

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@b = hidden local_unnamed_addr global %class.Base* null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@d2 = hidden global %class.Derived2 zeroinitializer, align 8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@_ZTV7Derived = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS7Derived = linkonce_odr hidden constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = linkonce_odr hidden constant [6 x i8] c"4Base\00", comdat
@_ZTI4Base = linkonce_odr hidden constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, comdat
@_ZTI7Derived = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat
@_ZTV8Derived2 = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8Derived2 to i8*), i8* bitcast (i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !4, !type !5
@_ZTS8Derived2 = linkonce_odr hidden constant [10 x i8] c"8Derived2\00", comdat
@_ZTI8Derived2 = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8Derived2, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_driver2.cpp, i8* null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define hidden i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #3 {
entry:
  %cmp = icmp slt i32 %argc, 2
  %. = select i1 %cmp, i1 (%class.Base*, i32)*** bitcast (%class.Derived* @d to i1 (%class.Base*, i32)***), i1 (%class.Base*, i32)*** bitcast (%class.Derived2* @d2 to i1 (%class.Base*, i32)***)
  %.4 = select i1 %cmp, %class.Base* getelementptr inbounds (%class.Derived, %class.Derived* @d, i64 0, i32 0), %class.Base* getelementptr inbounds (%class.Derived2, %class.Derived2* @d2, i64 0, i32 0)
  store i1 (%class.Base*, i32)*** %., i1 (%class.Base*, i32)**** bitcast (%class.Base** @b to i1 (%class.Base*, i32)****), align 8, !tbaa !8
  %vtable = load i1 (%class.Base*, i32)**, i1 (%class.Base*, i32)*** %., align 8, !tbaa !12
  %tmp = bitcast i1 (%class.Base*, i32)** %vtable to i8*
  %tmp1 = tail call i1 @llvm.type.test(i8* %tmp, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %tmp1)
  %tmp2 = load i1 (%class.Base*, i32)*, i1 (%class.Base*, i32)** %vtable, align 8
  %call = tail call zeroext i1 %tmp2(%class.Base* %.4, i32 %argc)
  %call.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i1 zeroext %call)
  %call1.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call.i, i8* nonnull getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i64 1)
  ret i32 0
}

; Function Attrs: nounwind readnone
declare i1 @llvm.type.test(i8*, metadata) #4

; Function Attrs: nounwind
declare void @llvm.assume(i1) #2

; Function Attrs: noinline norecurse nounwind uwtable
define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(%class.Derived* %this, i32 %a) unnamed_addr #5 comdat align 2 {
entry:
  ret i1 true
}

; Function Attrs: noinline norecurse nounwind uwtable
define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(%class.Derived2* %this, i32 %a) unnamed_addr #5 comdat align 2 {
entry:
  ret i1 false
}

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"*, i1 zeroext) local_unnamed_addr #0

declare dso_local dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* dereferenceable(272), i8*, i64) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_driver2.cpp() #6 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZSt8__ioinit)
  %tmp = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZSt8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #2
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV7Derived, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** getelementptr inbounds (%class.Derived, %class.Derived* @d, i64 0, i32 0, i32 0), align 8, !tbaa !12
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV8Derived2, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** getelementptr inbounds (%class.Derived2, %class.Derived2* @d2, i64 0, i32 0, i32 0), align 8, !tbaa !12
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }
attributes #5 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!6}
!llvm.ident = !{!7}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbiE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbiE.virtual"}
!4 = !{i64 16, !"_ZTS8Derived2"}
!5 = !{i64 16, !"_ZTSM8Derived2FbiE.virtual"}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ff38d5989c66cc12167cbe397bfb5d6915c4838f)"}
!8 = !{!9, !9, i64 0}
!9 = !{!"unspecified pointer", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"vtable pointer", !11, i64 0}

; Check that the transformation was applied correctly in function main.
; CHECK:       define hidden i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #3 {


; This part checks if the address of the virtual function is the same as Derived::foo
; CHECK:       %0 = bitcast i1 (%class.Base*, i32)* %tmp2 to i8*
; CHECK-NEXT:  %1 = bitcast i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i8*
; CHECK-NEXT:  %2 = icmp eq i8* %0, %1
; CHECK-NEXT:  br i1 %2, label %BBDevirt__ZN7Derived3fooEi_0_0, label %ElseDevirt__ZN7Derived3fooEi_0_0

; If the address is the same, then call Derived::foo
; CHECK-LABEL: BBDevirt__ZN7Derived3fooEi_0_0:
; CHECK:        %3 = tail call zeroext i1 bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i1 (%class.Base*, i32)*)(%class.Base* %.4, i32 %argc)
; CHECK-NEXT:   br label %MergeBB_0_0

; Now check if the address is the same as Derived2::foo
; CHECK-LABEL: ElseDevirt__ZN7Derived3fooEi_0_0:
; CHECK:        %4 = bitcast i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i8*
; CHECK-NEXT:   %5 = icmp eq i8* %0, %4
; CHECK-NEXT:   br i1 %5, label %BBDevirt__ZN8Derived23fooEi_0_0, label %DefaultBB_0_0

; If the address is the same as Derived2::foo, then call it
; CHECK-LABEL: BBDevirt__ZN8Derived23fooEi_0_0:
; CHECK:        %6 = tail call zeroext i1 bitcast (i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i1 (%class.Base*, i32)*)(%class.Base* %.4, i32 %argc)
; CHECK-NEXT:   br label %MergeBB_0_0

; This is the fail safe case. In case the address doesn't match any of the functions, then
; CHECK-LABEL: DefaultBB_0_0:
; CHECK-NEXT:   %7 = tail call zeroext i1 %tmp2(%class.Base* %.4, i32 %argc)
; CHECK-NEXT:   br label %MergeBB_0_0

; We need to collect back the result and generate the PhiNode
; CHECK-LABEL: MergeBB_0_0:
; CHECK-NEXT:   %8 = phi i1 [ %3, %BBDevirt__ZN7Derived3fooEi_0_0 ], [ %6, %BBDevirt__ZN8Derived23fooEi_0_0 ], [ %7, %DefaultBB_0_0 ]
; CHECK-NEXT:   br label %9

; Now check that the users were replaced correctly
; CHECK-LABEL: 9:
; CHECK-NEXT:   %call.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo9_M_insertIbEERSoT_(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i1 zeroext %8)
; CHECK-NEXT:   %call1.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call.i, i8* nonnull getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i64 1)
; CHECK-NEXT:   ret i32 0
; CHECK-NEXT: }

; Check that the metadata was added
; CHECK: define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(%class.Derived* %this, i32 %a) unnamed_addr #{{.*}} comdat align 2 !_Intel.Devirt.Target
; CHECK: define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(%class.Derived2* %this, i32 %a) unnamed_addr #{{.*}} comdat align 2 !_Intel.Devirt.Target
