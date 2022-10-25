; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-soatoaosop-deps          \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-DEP %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output              \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed                           \
; RUN:          -passes=soatoaosop-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-MOD %s
;
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output                            \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s -opaque-pointers  -whole-program-assume -intel-libirc-allowed                                        \
; RUN:          -passes=soatoaosop-struct-methods-transform                                       \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtransop-optbase-process-function-declaration                                    \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       | FileCheck --check-prefix=CHECK-OP-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that approximations work as expected.
; Checks special cases in approximation computations (see inlined checks).
; CHECK-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; CHECK-OP-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-OP-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-TRANS: ; Checking structure's method FieldValueMap::getValueAt(unsigned int) const
; CHECK-TRANS: ; IR: no need to analyze: no accesses to arrays
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::getValueAt(unsigned int) const
; CHECK-OP-TRANS: ; IR: no need to analyze: no accesses to arrays

; Checks instructions related to transformations:
; No references to arrays of interest.
; CHECK-TRANS: ; Dump instructions needing update. Total = 0
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 0

; Checks transformation. Only types change.

; inline XMLCh *FieldValueMap::getValueAt(const unsigned int index) const {
;   if (fValues) {
;     return fValues->elementAt(index);
;   }
;   return 0;
; }

%class.XMLPlatformUtilsException = type opaque
%class.XMLMsgLoader = type { i32 (...)** }
%class.FieldValueMap = type { %class.ValueVectorOf.0*, %class.ValueVectorOf.1*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
%class.ValueVectorOf.0 = type { i8, i32, i32, %class.IC_Field**, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_class.FieldValueMap = type { %__SOA_AR_class.ValueVectorOf.0*, float*, %class.RefArrayVectorOf*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, %__SOA_EL_class.FieldValueMap*, %class.XMLMsgLoader* }
; CHECK-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { %class.IC_Field*, %class.DatatypeValidator* }
; CHECK-MOD-NOT: ValueVectorOf.1
; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%class.ValueVectorOf.1 = type { i8, i32, i32, %class.DatatypeValidator**, %class.XMLMsgLoader* }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { i32 (...)**, i8, i32, i32, i16**, %class.XMLMsgLoader* }
%class.XMLException = type opaque

@"typeinfo for ArrayIndexOutOfBoundsException" = external hidden constant { i8*, i8*, i8* }
@.str.2.2091 = external hidden constant [33 x i8]

declare i32 @__gxx_personality_v0(...)

declare noalias i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

declare void @__cxa_free_exception(i8*)

declare !intel.dtrans.func.type !30 hidden void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i32, i32, %class.XMLMsgLoader* "intel_dtrans_func_index"="3")

; CHECK-MOD: @"FieldValueMap::getValueAt(unsigned int) const.1"(%__SOA_class.FieldValueMap* "intel_dtrans_func_index"="2" %this, i32 %index) personality i32 (...)* @__gxx_personality_v0
; CHECK-OP-MOD: @"FieldValueMap::getValueAt(unsigned int) const"(ptr "intel_dtrans_func_index"="2" %this, i32 %index) personality ptr @__gxx_personality_v0
define hidden "intel_dtrans_func_index"="1" i16* @"FieldValueMap::getValueAt(unsigned int) const"(%class.FieldValueMap* "intel_dtrans_func_index"="2" %this, i32 %index) personality i32 (...)* @__gxx_personality_v0 !intel.dtrans.func.type !25 {
entry:
; CHECK-MOD:  %fValues = getelementptr inbounds %__SOA_class.FieldValueMap, %__SOA_class.FieldValueMap* %this, i64 0, i32 2
; CHECK-OP-MOD:  %fValues = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 2
  %fValues = getelementptr inbounds %class.FieldValueMap, %class.FieldValueMap* %this, i64 0, i32 2
  %tmp = load %class.RefArrayVectorOf*, %class.RefArrayVectorOf** %fValues
  %tobool = icmp eq %class.RefArrayVectorOf* %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
  %fCurCount.i = getelementptr inbounds %class.RefArrayVectorOf, %class.RefArrayVectorOf* %tmp, i64 0, i32 0, i32 2
  %tmp1 = load i32, i32* %fCurCount.i
  %cmp.i = icmp ugt i32 %tmp1, %index
  br i1 %cmp.i, label %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]", label %if.then.i

if.then.i:                                        ; preds = %if.then
  %exception.i = tail call i8* @__cxa_allocate_exception(i64 48)
  %tmp2 = bitcast i8* %exception.i to %class.XMLPlatformUtilsException*
  %fMemoryManager.i = getelementptr inbounds %class.RefArrayVectorOf, %class.RefArrayVectorOf* %tmp, i64 0, i32 0, i32 5
  %tmp3 = load %class.XMLMsgLoader*, %class.XMLMsgLoader** %fMemoryManager.i
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(%class.XMLPlatformUtilsException* %tmp2, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.2.2091, i64 0, i64 0), i32 249, i32 116, %class.XMLMsgLoader* %tmp3)
          to label %invoke.cont.i unwind label %lpad.i

invoke.cont.i:                                    ; preds = %if.then.i
  tail call void @__cxa_throw(i8* %exception.i, i8* bitcast ({ i8*, i8*, i8* }* @"typeinfo for ArrayIndexOutOfBoundsException" to i8*), i8* bitcast (void (%class.XMLException*)* @"XMLException::~XMLException()" to i8*))
  unreachable

lpad.i:                                           ; preds = %if.then.i
  %tmp4 = landingpad { i8*, i32 }
          cleanup
  tail call void @__cxa_free_exception(i8* %exception.i)
  resume { i8*, i32 } %tmp4

"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]": ; preds = %if.then
  %fElemList.i = getelementptr inbounds %class.RefArrayVectorOf, %class.RefArrayVectorOf* %tmp, i64 0, i32 0, i32 4
  %tmp5 = load i16**, i16*** %fElemList.i
  %idxprom.i = zext i32 %index to i64
  %arrayidx.i = getelementptr inbounds i16*, i16** %tmp5, i64 %idxprom.i
  %tmp6 = load i16*, i16** %arrayidx.i
  br label %return

return:                                           ; preds = %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]", %entry
  %retval.0 = phi i16* [ %tmp6, %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]" ], [ null, %entry ]
  ret i16* %retval.0
}

declare !intel.dtrans.func.type !28 hidden void @"XMLException::~XMLException()"(%class.XMLException* "intel_dtrans_func_index"="1")

; XCHECK-DEP: Deps computed: 16, Queries: 36

!intel.dtrans.types = !{ !1, !2, !3, !7, !14, !16, !18, !20, !26, !29}

!1 = !{!"S", %"class.DatatypeValidator" zeroinitializer, i32 -1}
!2 = !{!"S", %"class.IC_Field" zeroinitializer, i32 -1}
!3 = !{!"S", %"class.XMLMsgLoader" zeroinitializer, i32 1, !4}
!4 = !{!5, i32 2}
!5 = !{!"F", i1 true, i32 0, !6}
!6 = !{i32 0, i32 0}
!7 = !{!"S", %"class.BaseRefVectorOf" zeroinitializer, i32 6, !9, !12, !11, !11, !8, !13}
!8 = !{i16 0, i32 2}
!9 = !{!10, i32 2}
!10 = !{!"F", i1 true, i32 0, !11}
!11 = !{i32 0, i32 0}
!12 = !{i8 0, i32 0}
!13 = !{%"class.XMLMsgLoader" zeroinitializer, i32 1}
!14 = !{!"S", %"class.RefArrayVectorOf" zeroinitializer, i32 1, !15}
!15 = !{%"class.BaseRefVectorOf" zeroinitializer, i32 0}
!16 = !{!"S", %"class.ValueVectorOf.1" zeroinitializer, i32 5, !12, !11, !11, !17, !13}
!17 = !{%"class.DatatypeValidator" zeroinitializer, i32 2}
!18 = !{!"S", %"class.ValueVectorOf.0" zeroinitializer, i32 5, !12, !11, !11, !19, !13}
!19 = !{%"class.IC_Field" zeroinitializer, i32 2}
!20 = !{!"S", %"class.FieldValueMap" zeroinitializer, i32 4, !21, !22, !23, !13}
!21 = !{%"class.ValueVectorOf.0" zeroinitializer, i32 1}
!22 = !{%"class.ValueVectorOf.1" zeroinitializer, i32 1}
!23 = !{%"class.RefArrayVectorOf" zeroinitializer, i32 1}
!24 = !{%"class.FieldValueMap" zeroinitializer, i32 1}
!25 = distinct !{!33, !24}
!26 = !{!"S", %"class.XMLException" zeroinitializer, i32 -1}
!27 = !{%"class.XMLException" zeroinitializer, i32 1}
!28 = distinct !{!27}
!29 = !{!"S", %"class.XMLPlatformUtilsException" zeroinitializer, i32 -1}
!30 = distinct !{!31, !32, !13}
!31 = !{%"class.XMLPlatformUtilsException" zeroinitializer, i32 1}
!32 = !{i8 0, i32 1}
!33 = !{i16 0, i32 1}
