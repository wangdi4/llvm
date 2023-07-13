; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop-deps                                                \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>'               \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-DEP %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output      \
; RUN:          -debug-only=dtrans-soatoaosop,dtrans-soatoaosop-struct                            \
; RUN:          -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>,require<soatoaosop-struct-methods>' \
; RUN:          -dtrans-soatoaosop-mem-off=3                                                      \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.0                               \
; RUN:          -dtrans-soatoaosop-array-type=class.ValueVectorOf.1                               \
; RUN:          -dtrans-soatoaosop-base-ptr-off=3                                                 \
; RUN:       2>&1 | FileCheck --check-prefix=CHECK-OP-TRANS %s
; RUN: opt -S < %s  -whole-program-assume -intel-libirc-allowed                  \
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
; CHECK-OP-DEP-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-OP-DEP-NOT: ; Func(GEP

; Checks that all instructions can be dealt with.
; CHECK-OP-TRANS: ; Checking structure's method FieldValueMap::getValueAt(unsigned int) const
; CHECK-OP-TRANS: ; IR: no need to analyze: no accesses to arrays

; Checks instructions related to transformations:
; No references to arrays of interest.
; CHECK-OP-TRANS: ; Dump instructions needing update. Total = 0

; Checks transformation. Only types change.

; inline XMLCh *FieldValueMap::getValueAt(const unsigned int index) const {
;   if (fValues) {
;     return fValues->elementAt(index);
;   }
;   return 0;
; }

; CHECK-OP-MOD-DAG: %__SOA_class.FieldValueMap = type { ptr, ptr, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_AR_class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
; CHECK-OP-MOD-DAG: %__SOA_EL_class.FieldValueMap = type { ptr, ptr }
; CHECK-OP-MOD-NOT: ValueVectorOf.1
%class.FieldValueMap = type { ptr, ptr, ptr, ptr }
%class.RefArrayVectorOf = type { %class.BaseRefVectorOf }
%class.BaseRefVectorOf = type { ptr, i8, i32, i32, ptr, ptr }
%class.DatatypeValidator = type opaque
%class.IC_Field = type opaque
%class.XMLMsgLoader = type { ptr }
%class.ValueVectorOf.1 = type { i8, i32, i32, ptr, ptr }
%class.ValueVectorOf.0 = type { i8, i32, i32, ptr, ptr }
%class.XMLException = type opaque
%class.XMLPlatformUtilsException = type opaque

@"typeinfo for ArrayIndexOutOfBoundsException" = external hidden constant { ptr, ptr, ptr }
@.str.2.2091 = external hidden constant [33 x i8]

declare i32 @__gxx_personality_v0(...)

declare noalias ptr @__cxa_allocate_exception(i64)

declare void @__cxa_throw(ptr, ptr, ptr)

declare void @__cxa_free_exception(ptr)

; CHECK-OP-MOD: @"FieldValueMap::getValueAt(unsigned int) const"(ptr "intel_dtrans_func_index"="2" %this, i32 %index) personality ptr @__gxx_personality_v0
declare !intel.dtrans.func.type !22 hidden void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i32, i32, ptr "intel_dtrans_func_index"="3")

define hidden "intel_dtrans_func_index"="1" ptr @"FieldValueMap::getValueAt(unsigned int) const"(ptr "intel_dtrans_func_index"="2" %this, i32 %index) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !25 {
entry:
; CHECK-OP-MOD:  %fValues = getelementptr inbounds %__SOA_class.FieldValueMap, ptr %this, i64 0, i32 2
  %fValues = getelementptr inbounds %class.FieldValueMap, ptr %this, i64 0, i32 2
  %tmp = load ptr, ptr %fValues, align 8
  %tobool = icmp eq ptr %tmp, null
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
  %fCurCount.i = getelementptr inbounds %class.RefArrayVectorOf, ptr %tmp, i64 0, i32 0, i32 2
  %tmp1 = load i32, ptr %fCurCount.i, align 4
  %cmp.i = icmp ugt i32 %tmp1, %index
  br i1 %cmp.i, label %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]", label %if.then.i

if.then.i:                                        ; preds = %if.then
  %exception.i = tail call ptr @__cxa_allocate_exception(i64 48)
  %tmp2 = bitcast ptr %exception.i to ptr
  %fMemoryManager.i = getelementptr inbounds %class.RefArrayVectorOf, ptr %tmp, i64 0, i32 0, i32 5
  %tmp3 = load ptr, ptr %fMemoryManager.i, align 8
  invoke void @"ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException(char const*, unsigned int, XMLExcepts::Codes, MemoryManager*)"(ptr %tmp2, ptr @.str.2.2091, i32 249, i32 116, ptr %tmp3)
          to label %invoke.cont.i unwind label %lpad.i

invoke.cont.i:                                    ; preds = %if.then.i
  tail call void @__cxa_throw(ptr %exception.i, ptr @"typeinfo for ArrayIndexOutOfBoundsException", ptr @"XMLException::~XMLException()")
  unreachable

lpad.i:                                           ; preds = %if.then.i
  %tmp4 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %exception.i)
  resume { ptr, i32 } %tmp4

"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]": ; preds = %if.then
  %fElemList.i = getelementptr inbounds %class.RefArrayVectorOf, ptr %tmp, i64 0, i32 0, i32 4
  %tmp5 = load ptr, ptr %fElemList.i, align 8
  %idxprom.i = zext i32 %index to i64
  %arrayidx.i = getelementptr inbounds ptr, ptr %tmp5, i64 %idxprom.i
  %tmp6 = load ptr, ptr %arrayidx.i, align 8
  br label %return

return:                                           ; preds = %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]", %entry
  %retval.0 = phi ptr [ %tmp6, %"BaseRefVectorOf<unsigned short>::elementAt(unsigned int) [clone .exit]" ], [ null, %entry ]
  ret ptr %retval.0
}

declare !intel.dtrans.func.type !28 hidden void @"XMLException::~XMLException()"(ptr "intel_dtrans_func_index"="1")

; XCHECK-DEP: Deps computed: 16, Queries: 36

!intel.dtrans.types = !{!0, !1, !2, !6, !10, !12, !14, !16, !20, !21}

!0 = !{!"S", %class.DatatypeValidator zeroinitializer, i32 -1}
!1 = !{!"S", %class.IC_Field zeroinitializer, i32 -1}
!2 = !{!"S", %class.XMLMsgLoader zeroinitializer, i32 1, !3}
!3 = !{!4, i32 2}
!4 = !{!"F", i1 true, i32 0, !5}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %class.BaseRefVectorOf zeroinitializer, i32 6, !3, !7, !5, !5, !8, !9}
!7 = !{i8 0, i32 0}
!8 = !{i16 0, i32 2}
!9 = !{%class.XMLMsgLoader zeroinitializer, i32 1}
!10 = !{!"S", %class.RefArrayVectorOf zeroinitializer, i32 1, !11}
!11 = !{%class.BaseRefVectorOf zeroinitializer, i32 0}
!12 = !{!"S", %class.ValueVectorOf.1 zeroinitializer, i32 5, !7, !5, !5, !13, !9}
!13 = !{%class.DatatypeValidator zeroinitializer, i32 2}
!14 = !{!"S", %class.ValueVectorOf.0 zeroinitializer, i32 5, !7, !5, !5, !15, !9}
!15 = !{%class.IC_Field zeroinitializer, i32 2}
!16 = !{!"S", %class.FieldValueMap zeroinitializer, i32 4, !17, !18, !19, !9}
!17 = !{%class.ValueVectorOf.0 zeroinitializer, i32 1}
!18 = !{%class.ValueVectorOf.1 zeroinitializer, i32 1}
!19 = !{%class.RefArrayVectorOf zeroinitializer, i32 1}
!20 = !{!"S", %class.XMLException zeroinitializer, i32 -1}
!21 = !{!"S", %class.XMLPlatformUtilsException zeroinitializer, i32 -1}
!22 = distinct !{!23, !24, !9}
!23 = !{%class.XMLPlatformUtilsException zeroinitializer, i32 1}
!24 = !{i8 0, i32 1}
!25 = distinct !{!26, !27}
!26 = !{i16 0, i32 1}
!27 = !{%class.FieldValueMap zeroinitializer, i32 1}
!28 = distinct !{!29}
!29 = !{%class.XMLException zeroinitializer, i32 1}
