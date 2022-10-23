; RUN: opt -S -passes='cgscc(inline)' -inline-report=0xe807 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that the inline report can be produced without dying because an active
; inlined call site is replaced without another during inlining. The check that
; DELETE does not appear below indicates that the active inlined call site being
; replaced was recognized and the replacing call site was substituted for it.

; CHECK-CL-LABEL: COMPILE FUNC: ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z
; CHECK-LABEL: COMPILE FUNC: ??$?6U?$char_traits@D@std@@@std@@YAAAV?$basic_ostream@DU?$char_traits@D@std@@@0@AAV10@PBD@Z
; CHECK-NEXT: INLINE: ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z <<Callee is single basic block>>
; CHECK-NOT: DELETE
; CHECK-MD-LABEL: COMPILE FUNC: ?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z

%"class.std::basic_ostream" = type { i32*, i32, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_streambuf"*, %"class.std::basic_ostream"*, i8 }
%"class.std::ios_base" = type { i32 (...)**, [4 x i8], i32, i32, i32, i32, i64, i64, %"struct.std::ios_base::_Iosarray"*, %"struct.std::ios_base::_Fnarray"*, %"class.std::locale"* }
%"struct.std::ios_base::_Iosarray" = type { %"struct.std::ios_base::_Iosarray"*, i32, i32, i8* }
%"struct.std::ios_base::_Fnarray" = type { %"struct.std::ios_base::_Fnarray"*, i32, void (i32, %"class.std::ios_base"*, i32)* }
%"class.std::locale" = type { [4 x i8], %"class.std::locale::_Locimp"* }
%"class.std::locale::_Locimp" = type { %"class.std::locale::facet", %"class.std::locale::facet"**, i32, i32, i8, %"class.std::_Yarn" }
%"class.std::locale::facet" = type { %"class.std::_Facet_base", i32 }
%"class.std::_Facet_base" = type { i32 (...)** }
%"class.std::_Yarn" = type { i8*, i8 }
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8**, i8**, i8*, i8*, i8**, i8**, i32, i32, i32*, i32*, %"class.std::locale"* }

$"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z" = comdat any

define %"class.std::basic_ostream"* @"??$?6U?$char_traits@D@std@@@std@@YAAAV?$basic_ostream@DU?$char_traits@D@std@@@0@AAV10@PBD@Z"() personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
if.end87:
  ret %"class.std::basic_ostream"* null

catch.dispatch:                                   ; No predecessors!
  %0 = catchswitch within none [label %catch] unwind label %ehcleanup105

catch:                                            ; preds = %catch.dispatch
  %1 = catchpad within %0 [i8* null, i32 0, i8* null]
  invoke x86_thiscallcc void @"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z"(%"class.std::basic_ios"* null, i32 0, i1 false) [ "funclet"(token %1) ]
          to label %invoke.cont98 unwind label %ehcleanup105

invoke.cont98:                                    ; preds = %catch
  ret %"class.std::basic_ostream"* null

ehcleanup105:                                     ; preds = %catch, %catch.dispatch
  %2 = cleanuppad within none []
  cleanupret from %2 unwind to caller
}

declare i32 @__CxxFrameHandler3(...)

define x86_thiscallcc void @"?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QAEXH_N@Z"(%"class.std::basic_ios"* %this, i32 %_State, i1 %_Reraise) comdat {
entry:
  %call = call x86_thiscallcc i32 undef(%"class.std::ios_base"* null)
  ret void
}

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
