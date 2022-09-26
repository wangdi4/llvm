; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers -passes=inline -inline-threshold=0 -inline-report=0xf867 -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -opaque-pointers -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-BG
; RUN: opt -opaque-pointers -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -sycl-host -sycl-optimization-mode -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -opaque-pointers -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -sycl-host -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -opaque-pointers -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -sycl-optimization-mode -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xf8e6 < %s -S | opt -passes=inline -inline-threshold=0 -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD
; RUN: opt -opaque-pointers -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefix=CHECK-BG
; RUN: opt -opaque-pointers -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -sycl-host -sycl-optimization-mode -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD
; RUN: opt -opaque-pointers -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -sycl-host -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD
; RUN: opt -opaque-pointers -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -sycl-optimization-mode -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD

; Check that without -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0
; that single callsite link once ODR functions get inlined regardless of size.
; Also check that adding -sycl-host and/or -sycl-optimization-mode has the same
; effect, even if -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 are passed.
; This LIT test tests the behavior when -lto-inline-cost is used.

; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE
; CHECK-SM-CL-NOT: call {{.*}} @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_
; CHECK-SM-CL-NOT: call {{.*}} @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj
; CHECK-SM-CL-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv

; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_ {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_ {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj {{.*}}Callee has single callsite and local linkage
; CHECK-SM-DAG: INLINE: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv {{.*}}Callee has single callsite and local linkage

; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE
; CHECK-SM-MD-NOT: call {{.*}} @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_
; CHECK-SM-MD-NOT: call {{.*}} @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj
; CHECK-SM-MD-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv

; Check that with -inlining-huge-odr-bb-count=0 -inline-threshold=0
; that single callsite link once ODR functions DO NOT get inlined regardless of
; size.

; CHECK-BG-CL-DAG: invoke {{.*}} @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE
; CHECK-BG-CL-DAG: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj
; CHECK-BG-CL-DAG: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev
; CHECK-BG-CL-DAG: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev
; CHECK-BG-CL-DAG: invoke {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_
; CHECK-BG-CL-DAG: invoke {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_
; CHECK-BG-CL-DAG: invoke {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-BG-CL-DAG: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev
; CHECK-BG-CL-DAG: invoke {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj
; CHECK-BG-CL-DAG: invoke {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt
; CHECK-BG-CL-DAG: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev
; CHECK-BG-CL-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE
; CHECK-BG-CL-NOT: call {{.*}} @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv
; CHECK-BG-CL-NOT: call {{.*}} @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv
; CHECK-BG-CL-NOT: call {{.*}} @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-BG-CL-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv
; CHECK-BG-CL-NOT: call {{.*}} @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE
; CHECK-BG-CL-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv

; CHECK-BG-DAG: O _ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE {{.*}}Inlining is not profitable
; CHECK-BG-DAG: O _ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj {{.*}}Inlining is not profitable
; CHECK-BG-DAG: O _ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev {{.*}}Inlining is not profitable
; CHECK-BG-DAG: O _ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev {{.*}}Inlining is not profitable
; CHECK-BG-DAG: INLINE: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE {{.*}}Callee is single basic block
; CHECK-BG-DAG: INLINE: O _ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv {{.*}}Callee is single basic block
; CHECK-BG-DAG: O _ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_ {{.*}}Inlining is not profitable
; CHECK-BG-DAG: O _ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_ {{.*}}Inlining is not profitable
; CHECK-BG-DAG: INLINE: O _ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv {{.*}}Callee is single basic block
; CHECK-BG-DAG: INLINE: O _ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE {{.*}}Callee is single basic block
; CHECK-BG-DAG: O _ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE {{.*}}Inlining is not profitable
; CHECK-BG-DAG: INLINE: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv {{.*}}Callee is single basic block
; CHECK-BG-DAG: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev {{.*}}Inlining is not profitable
; CHECK-BG-DAG: O _ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj {{.*}}Inlining is not profitable
; CHECK-BG-DAG: INLINE: O _ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE {{.*}}Inlining is profitable
; CHECK-BG-DAG: O _ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt {{.*}}Inlining is not profitable
; CHECK-BG-DAG: INLINE: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv {{.*}}Callee is single basic block
; CHECK-BG-DAG: O _ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev {{.*}}Callsite is cold

; CHECK-BG-MD-DAG: invoke {{.*}} @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE
; CHECK-BG-MD-DAG: call {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj
; CHECK-BG-MD-DAG: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev
; CHECK-BG-MD-DAG: call {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev
; CHECK-BG-MD-DAG: invoke {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_
; CHECK-BG-MD-DAG: invoke {{.*}} @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_
; CHECK-BG-MD-DAG: invoke {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-BG-MD-DAG: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev
; CHECK-BG-MD-DAG: invoke {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj
; CHECK-BG-MD-DAG: invoke {{.*}} @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt
; CHECK-BG-MD-DAG: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev
; CHECK-BG-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE
; CHECK-BG-NOT: call {{.*}} @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv
; CHECK-BG-NOT: call {{.*}} @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv
; CHECK-BG-NOT: call {{.*}} @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE
; CHECK-BG-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv
; CHECK-BG-NOT: call {{.*}} @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE
; CHECK-BG-NOT: call {{.*}} @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.xercesc_2_7::JanitorMemFunCall" = type { ptr, { i64, i64 } }
%"class.xercesc_2_7::FieldValueMap" = type { ptr, ptr, ptr, ptr }
%"class.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"class.xercesc_2_7::ValueVectorOf.0" = type { i8, i32, i32, ptr, ptr }
%"class.xercesc_2_7::BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"class.xercesc_2_7::RefArrayVectorOf" = type { %"class.xercesc_2_7::BaseRefVectorOf" }
%"class.xercesc_2_7::ArrayIndexOutOfBoundsException" = type { %"class.xercesc_2_7::XMLException" }
%"class.xercesc_2_7::XMLException" = type { ptr, i32, ptr, i32, ptr, ptr }

$_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE = comdat any

$_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_ = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_ = comdat any

$_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv = comdat any

$_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt = comdat any

$_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj = comdat any

$_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv = comdat any

$__clang_call_terminate = comdat any

$_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev = comdat any

$_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev = comdat any

$_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE = comdat any

$_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj = comdat any

$_ZTSN11xercesc_2_720OutOfMemoryExceptionE = comdat any

$_ZTSN11xercesc_2_77XMemoryE = comdat any

$_ZTIN11xercesc_2_77XMemoryE = comdat any

$_ZTIN11xercesc_2_720OutOfMemoryExceptionE = comdat any

$_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = comdat any

$_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = comdat any

$_ZTSN11xercesc_2_716RefArrayVectorOfItEE = comdat any

$_ZTSN11xercesc_2_715BaseRefVectorOfItEE = comdat any

$_ZTIN11xercesc_2_715BaseRefVectorOfItEE = comdat any

$_ZTIN11xercesc_2_716RefArrayVectorOfItEE = comdat any

@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTSN11xercesc_2_720OutOfMemoryExceptionE = linkonce_odr dso_local constant [38 x i8] c"N11xercesc_2_720OutOfMemoryExceptionE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTSN11xercesc_2_77XMemoryE = linkonce_odr dso_local constant [24 x i8] c"N11xercesc_2_77XMemoryE\00", comdat, align 1
@_ZTIN11xercesc_2_77XMemoryE = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTSN11xercesc_2_77XMemoryE }, comdat, align 8
@_ZTIN11xercesc_2_720OutOfMemoryExceptionE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSN11xercesc_2_720OutOfMemoryExceptionE, ptr @_ZTIN11xercesc_2_77XMemoryE }, comdat, align 8
@.str = private unnamed_addr constant [33 x i8] c"./xercesc/util/BaseRefVectorOf.c\00", align 1
@_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = linkonce_odr dso_local constant [48 x i8] c"N11xercesc_2_730ArrayIndexOutOfBoundsExceptionE\00", comdat, align 1
@_ZTIN11xercesc_2_712XMLExceptionE = external dso_local constant ptr
@_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr @_ZTIN11xercesc_2_712XMLExceptionE }, comdat, align 8
@_ZN11xercesc_2_76XMLUni37fgArrayIndexOutOfBoundsException_NameE = external dso_local constant [0 x i16], align 2
@.str.1 = private unnamed_addr constant [31 x i8] c"./xercesc/util/ValueVectorOf.c\00", align 1
@_ZTSN11xercesc_2_716RefArrayVectorOfItEE = linkonce_odr dso_local constant [37 x i8] c"N11xercesc_2_716RefArrayVectorOfItEE\00", comdat, align 1
@_ZTSN11xercesc_2_715BaseRefVectorOfItEE = linkonce_odr dso_local constant [36 x i8] c"N11xercesc_2_715BaseRefVectorOfItEE\00", comdat, align 1
@_ZTIN11xercesc_2_715BaseRefVectorOfItEE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSN11xercesc_2_715BaseRefVectorOfItEE, ptr @_ZTIN11xercesc_2_77XMemoryE }, comdat, align 8
@_ZTIN11xercesc_2_716RefArrayVectorOfItEE = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTSN11xercesc_2_716RefArrayVectorOfItEE, ptr @_ZTIN11xercesc_2_715BaseRefVectorOfItEE }, comdat, align 8
@.str.2 = private unnamed_addr constant [34 x i8] c"./xercesc/util/RefArrayVectorOf.c\00", align 1

@_ZN11xercesc_2_713FieldValueMapC1ERKS0_ = dso_local unnamed_addr alias void (ptr, ptr), ptr @_ZN11xercesc_2_713FieldValueMapC2ERKS0_

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #0

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(ptr noundef %this, ptr noundef nonnull align 8 dereferenceable(32) %other) unnamed_addr #1 align 2 personality ptr @__gxx_personality_v0 {
entry:
  %cleanup = alloca %"class.xercesc_2_7::JanitorMemFunCall", align 8
  %fFields = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 0, !intel-tbaa !6
  store ptr null, ptr %fFields, align 8, !tbaa !6
  %fValidators = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 1, !intel-tbaa !14
  store ptr null, ptr %fValidators, align 8, !tbaa !14
  %fValues = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 2, !intel-tbaa !15
  store ptr null, ptr %fValues, align 8, !tbaa !15
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 3, !intel-tbaa !16
  %fMemoryManager2 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %other, i64 0, i32 3, !intel-tbaa !16
  %i = load ptr, ptr %fMemoryManager2, align 8, !tbaa !16
  store ptr %i, ptr %fMemoryManager, align 8, !tbaa !16
  %fFields3 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %other, i64 0, i32 0, !intel-tbaa !6
  %i1 = load ptr, ptr %fFields3, align 8, !tbaa !6
  %tobool.not = icmp eq ptr %i1, null
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %i2 = bitcast ptr %cleanup to ptr
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %i2) #15
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE(ptr noundef nonnull %cleanup, ptr noundef %this, i64 ptrtoint (ptr @_ZN11xercesc_2_713FieldValueMap7cleanUpEv to i64), i64 0)
  %fValues4 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %other, i64 0, i32 2, !intel-tbaa !15
  %i3 = bitcast ptr %fValues4 to ptr
  %i4 = load ptr, ptr %i3, align 8, !tbaa !15
  %call = call noundef i32 @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv(ptr noundef %i4)
  %i5 = load ptr, ptr %fMemoryManager, align 8, !tbaa !16
  %call7 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i5)
          to label %invoke.cont6 unwind label %lpad

invoke.cont6:                                     ; preds = %if.then
  %i6 = bitcast ptr %call7 to ptr
  %i7 = load ptr, ptr %fFields3, align 8, !tbaa !6
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr noundef %i6, ptr noundef nonnull align 8 dereferenceable(32) %i7)
          to label %invoke.cont10 unwind label %lpad9

invoke.cont10:                                    ; preds = %invoke.cont6
  store ptr %i6, ptr %fFields, align 8, !tbaa !6
  %i8 = load ptr, ptr %fMemoryManager, align 8, !tbaa !16
  %call14 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, ptr noundef %i8)
          to label %invoke.cont13 unwind label %lpad

invoke.cont13:                                    ; preds = %invoke.cont10
  %i9 = bitcast ptr %call14 to ptr
  %fValidators15 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %other, i64 0, i32 1, !intel-tbaa !14
  %i10 = load ptr, ptr %fValidators15, align 8, !tbaa !14
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_(ptr noundef %i9, ptr noundef nonnull align 8 dereferenceable(32) %i10)
          to label %invoke.cont17 unwind label %lpad16

invoke.cont17:                                    ; preds = %invoke.cont13
  store ptr %i9, ptr %fValidators, align 8, !tbaa !14
  %i11 = load ptr, ptr %fMemoryManager, align 8, !tbaa !16
  %call21 = invoke noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 40, ptr noundef %i11)
          to label %invoke.cont20 unwind label %lpad

invoke.cont20:                                    ; preds = %invoke.cont17
  %i12 = bitcast ptr %call21 to ptr
  %i13 = load ptr, ptr %fFields3, align 8, !tbaa !6
  %call25 = call noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv(ptr noundef %i13)
  %i14 = load ptr, ptr %fMemoryManager, align 8, !tbaa !16
  invoke void @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE(ptr noundef %i12, i32 noundef %call25, i1 noundef zeroext true, ptr noundef %i14)
          to label %invoke.cont27 unwind label %lpad23

invoke.cont27:                                    ; preds = %invoke.cont20
  store ptr %i12, ptr %fValues, align 8, !tbaa !15
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %invoke.cont27
  %i.0 = phi i32 [ 0, %invoke.cont27 ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv(ptr noundef nonnull %cleanup)
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev(ptr noundef nonnull %cleanup) #15
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %i2) #15
  br label %if.end

lpad:                                             ; preds = %invoke.cont17, %invoke.cont10, %if.then
  %i15 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %ehcleanup

lpad9:                                            ; preds = %invoke.cont6
  %i16 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %call7, ptr noundef %i5) #15
  br label %ehcleanup

lpad16:                                           ; preds = %invoke.cont13
  %i17 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %call14, ptr noundef %i8) #15
  br label %ehcleanup

lpad23:                                           ; preds = %invoke.cont20
  %i18 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef %call21, ptr noundef %i11) #15
  br label %ehcleanup

for.body:                                         ; preds = %for.cond
  %i19 = bitcast ptr %fValues to ptr
  %i20 = load ptr, ptr %i19, align 8, !tbaa !15
  %i21 = bitcast ptr %fValues4 to ptr
  %i22 = load ptr, ptr %i21, align 8, !tbaa !15
  %call33 = invoke noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj(ptr noundef %i22, i32 noundef %i.0)
          to label %invoke.cont32 unwind label %lpad31

invoke.cont32:                                    ; preds = %for.body
  %i23 = load ptr, ptr %fMemoryManager, align 8, !tbaa !16
  %call36 = invoke noundef ptr @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(ptr noundef %call33, ptr noundef %i23)
          to label %invoke.cont35 unwind label %lpad31

invoke.cont35:                                    ; preds = %invoke.cont32
  invoke void @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt(ptr noundef %i20, ptr noundef %call36)
          to label %for.inc unwind label %lpad31

for.inc:                                          ; preds = %invoke.cont35
  %inc = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !17

lpad31:                                           ; preds = %invoke.cont35, %invoke.cont32, %for.body
  %i24 = landingpad { ptr, i32 }
          cleanup
          catch ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad31, %lpad23, %lpad16, %lpad9, %lpad
  %.pn = phi { ptr, i32 } [ %i24, %lpad31 ], [ %i18, %lpad23 ], [ %i15, %lpad ], [ %i17, %lpad16 ], [ %i16, %lpad9 ]
  %ehselector.slot.0 = extractvalue { ptr, i32 } %.pn, 1
  %i25 = call i32 @llvm.eh.typeid.for(ptr @_ZTIN11xercesc_2_720OutOfMemoryExceptionE) #15
  %matches = icmp eq i32 %ehselector.slot.0, %i25
  br i1 %matches, label %catch, label %ehcleanup46

catch:                                            ; preds = %ehcleanup
  %exn.slot.0 = extractvalue { ptr, i32 } %.pn, 0
  %i26 = call ptr @__cxa_begin_catch(ptr %exn.slot.0) #15
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv(ptr noundef nonnull %cleanup)
  invoke void @__cxa_rethrow() #26
          to label %unreachable unwind label %lpad39

lpad39:                                           ; preds = %catch
  %i27 = landingpad { ptr, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %ehcleanup46 unwind label %terminate.lpad

ehcleanup46:                                      ; preds = %lpad39, %ehcleanup
  %lpad.val50.merged = phi { ptr, i32 } [ %.pn, %ehcleanup ], [ %i27, %lpad39 ]
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev(ptr noundef nonnull %cleanup) #15
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %i2) #15
  resume { ptr, i32 } %lpad.val50.merged

if.end:                                           ; preds = %for.cond.cleanup, %entry
  ret void

terminate.lpad:                                   ; preds = %lpad39
  %i28 = landingpad { ptr, i32 }
          catch ptr null
  %i29 = extractvalue { ptr, i32 } %i28, 0
  call void @__clang_call_terminate(ptr %i29) #27
  unreachable

unreachable:                                      ; preds = %catch
  unreachable
}

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(ptr nocapture noundef readonly %this) #2 align 2 {
entry:
  %fFields = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 0, !intel-tbaa !6
  %i = load ptr, ptr %fFields, align 8, !tbaa !6
  %isnull = icmp eq ptr %i, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev(ptr noundef nonnull %i) #15
  %i1 = getelementptr %"class.xercesc_2_7::ValueVectorOf", ptr %i, i64 0, i32 0
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef %i1) #15
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %fValidators = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 1, !intel-tbaa !14
  %i2 = load ptr, ptr %fValidators, align 8, !tbaa !14
  %isnull2 = icmp eq ptr %i2, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev(ptr noundef nonnull %i2) #15
  %i3 = getelementptr %"class.xercesc_2_7::ValueVectorOf.0", ptr %i2, i64 0, i32 0
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef %i3) #15
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  %fValues = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", ptr %this, i64 0, i32 2, !intel-tbaa !15
  %i4 = load ptr, ptr %fValues, align 8, !tbaa !15
  %isnull5 = icmp eq ptr %i4, null
  br i1 %isnull5, label %delete.end7, label %delete.notnull6

delete.notnull6:                                  ; preds = %delete.end4
  %i5 = bitcast ptr %i4 to ptr
  %vtable = load ptr, ptr %i5, align 8, !tbaa !19
  %i6 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i6, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %delete.notnull6
  %i7 = bitcast ptr %vtable to ptr
  %i8 = tail call i1 @llvm.type.test(ptr %i7, metadata !"_ZTSN11xercesc_2_716RefArrayVectorOfItEE")
  tail call void @llvm.assume(i1 %i8)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %delete.notnull6
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 1
  %i9 = load ptr, ptr %vfn, align 8
  tail call void %i9(ptr noundef nonnull %i4) #15
  br label %delete.end7

delete.end7:                                      ; preds = %whpr.continue, %delete.end4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE(ptr noundef %this, ptr noundef %object, i64 %toCall.coerce0, i64 %toCall.coerce1) unnamed_addr #3 comdat align 2 {
entry:
  %fObject = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", ptr %this, i64 0, i32 0, !intel-tbaa !21
  store ptr %object, ptr %fObject, align 8, !tbaa !21
  %fToCall = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", ptr %this, i64 0, i32 1, !intel-tbaa !24
  %fToCall.repack = getelementptr inbounds { i64, i64 }, ptr %fToCall, i64 0, i32 0
  store i64 %toCall.coerce0, ptr %fToCall.repack, align 8, !tbaa !24
  %fToCall.repack3 = getelementptr inbounds { i64, i64 }, ptr %fToCall, i64 0, i32 1
  store i64 %toCall.coerce1, ptr %fToCall.repack3, align 8, !tbaa !24
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i32 @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv(ptr noundef %this) local_unnamed_addr #2 comdat align 2 {
entry:
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !25
  %i = load i32, ptr %fCurCount, align 4, !tbaa !25
  ret i32 %i
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local noundef ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef, ptr noundef) local_unnamed_addr #4

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(ptr noundef %this, ptr noundef nonnull align 8 dereferenceable(32) %toCopy) unnamed_addr #1 comdat align 2 {
entry:
  %fCallDestructor = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 0, !intel-tbaa !30
  %fCallDestructor2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %toCopy, i64 0, i32 0, !intel-tbaa !30
  %i = load i8, ptr %fCallDestructor2, align 8, !tbaa !30, !range !33
  store i8 %i, ptr %fCallDestructor, align 8, !tbaa !30
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 1, !intel-tbaa !34
  %fCurCount3 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %toCopy, i64 0, i32 1, !intel-tbaa !34
  %i1 = load i32, ptr %fCurCount3, align 4, !tbaa !34
  store i32 %i1, ptr %fCurCount, align 4, !tbaa !34
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !35
  %fMaxCount4 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %toCopy, i64 0, i32 2, !intel-tbaa !35
  %i2 = load i32, ptr %fMaxCount4, align 8, !tbaa !35
  store i32 %i2, ptr %fMaxCount, align 8, !tbaa !35
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 3, !intel-tbaa !36
  store ptr null, ptr %fElemList, align 8, !tbaa !36
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !37
  %fMemoryManager5 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %toCopy, i64 0, i32 4, !intel-tbaa !37
  %i3 = load ptr, ptr %fMemoryManager5, align 8, !tbaa !37
  store ptr %i3, ptr %fMemoryManager, align 8, !tbaa !37
  %conv = zext i32 %i2 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %i4 = bitcast ptr %i3 to ptr
  %vtable = load ptr, ptr %i4, align 8, !tbaa !19
  %i5 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i5, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i6 = bitcast ptr %vtable to ptr
  %i7 = tail call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i7)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i8 = load ptr, ptr %vfn, align 8
  %call = tail call noundef ptr %i8(ptr noundef nonnull %i3, i64 noundef %mul)
  %i9 = bitcast ptr %call to ptr
  store ptr %i9, ptr %fElemList, align 8, !tbaa !36
  %i10 = load i32, ptr %fMaxCount, align 8, !tbaa !35
  %conv11 = zext i32 %i10 to i64
  %mul12 = shl nuw nsw i64 %conv11, 3
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul12, i1 false)
  %i11 = load i32, ptr %fCurCount, align 4, !tbaa !34
  %cmp27.not = icmp eq i32 %i11, 0
  br i1 %cmp27.not, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %whpr.continue
  %fElemList14 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %toCopy, i64 0, i32 3, !intel-tbaa !36
  %i12 = load ptr, ptr %fElemList14, align 8, !tbaa !36
  %i13 = load ptr, ptr %fElemList, align 8, !tbaa !36
  %wide.trip.count = zext i32 %i11 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %whpr.continue
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i12, i64 %indvars.iv
  %i14 = load ptr, ptr %arrayidx, align 8, !tbaa !38
  %arrayidx17 = getelementptr inbounds ptr, ptr %i13, i64 %indvars.iv
  store ptr %i14, ptr %arrayidx17, align 8, !tbaa !38
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !40
}

; Function Attrs: nounwind
declare dso_local void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(ptr noundef, ptr noundef) local_unnamed_addr #5

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_(ptr noundef %this, ptr noundef nonnull align 8 dereferenceable(32) %toCopy) unnamed_addr #1 comdat align 2 {
entry:
  %fCallDestructor = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 0, !intel-tbaa !41
  %fCallDestructor2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %toCopy, i64 0, i32 0, !intel-tbaa !41
  %i = load i8, ptr %fCallDestructor2, align 8, !tbaa !41, !range !33
  store i8 %i, ptr %fCallDestructor, align 8, !tbaa !41
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 1, !intel-tbaa !44
  %fCurCount3 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %toCopy, i64 0, i32 1, !intel-tbaa !44
  %i1 = load i32, ptr %fCurCount3, align 4, !tbaa !44
  store i32 %i1, ptr %fCurCount, align 4, !tbaa !44
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 2, !intel-tbaa !45
  %fMaxCount4 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %toCopy, i64 0, i32 2, !intel-tbaa !45
  %i2 = load i32, ptr %fMaxCount4, align 8, !tbaa !45
  store i32 %i2, ptr %fMaxCount, align 8, !tbaa !45
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 3, !intel-tbaa !46
  store ptr null, ptr %fElemList, align 8, !tbaa !46
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 4, !intel-tbaa !47
  %fMemoryManager5 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %toCopy, i64 0, i32 4, !intel-tbaa !47
  %i3 = load ptr, ptr %fMemoryManager5, align 8, !tbaa !47
  store ptr %i3, ptr %fMemoryManager, align 8, !tbaa !47
  %conv = zext i32 %i2 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %i4 = bitcast ptr %i3 to ptr
  %vtable = load ptr, ptr %i4, align 8, !tbaa !19
  %i5 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i5, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i6 = bitcast ptr %vtable to ptr
  %i7 = tail call i1 @llvm.type.test(ptr %i6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i7)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i8 = load ptr, ptr %vfn, align 8
  %call = tail call noundef ptr %i8(ptr noundef nonnull %i3, i64 noundef %mul)
  %i9 = bitcast ptr %call to ptr
  store ptr %i9, ptr %fElemList, align 8, !tbaa !46
  %i10 = load i32, ptr %fMaxCount, align 8, !tbaa !45
  %conv11 = zext i32 %i10 to i64
  %mul12 = shl nuw nsw i64 %conv11, 3
  tail call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul12, i1 false)
  %i11 = load i32, ptr %fCurCount, align 4, !tbaa !44
  %cmp27.not = icmp eq i32 %i11, 0
  br i1 %cmp27.not, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %whpr.continue
  %fElemList14 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %toCopy, i64 0, i32 3, !intel-tbaa !46
  %i12 = load ptr, ptr %fElemList14, align 8, !tbaa !46
  %i13 = load ptr, ptr %fElemList, align 8, !tbaa !46
  %wide.trip.count = zext i32 %i11 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %whpr.continue
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i12, i64 %indvars.iv
  %i14 = load ptr, ptr %arrayidx, align 8, !tbaa !48
  %arrayidx17 = getelementptr inbounds ptr, ptr %i13, i64 %indvars.iv
  store ptr %i14, ptr %arrayidx17, align 8, !tbaa !48
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !50
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv(ptr noundef %this) local_unnamed_addr #2 comdat align 2 {
entry:
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !35
  %i = load i32, ptr %fMaxCount, align 8, !tbaa !35
  ret i32 %i
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE(ptr noundef %this, i32 noundef %maxElems, i1 noundef zeroext %adoptElems, ptr noundef %manager) unnamed_addr #1 comdat align 2 {
entry:
  %i = getelementptr %"class.xercesc_2_7::RefArrayVectorOf", ptr %this, i64 0, i32 0
  tail call void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE(ptr noundef %i, i32 noundef %maxElems, i1 noundef zeroext %adoptElems, ptr noundef %manager)
  %i1 = getelementptr %"class.xercesc_2_7::RefArrayVectorOf", ptr %this, i64 0, i32 0, i32 0
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt(ptr noundef %this, ptr noundef %toAdd) local_unnamed_addr #6 comdat align 2 {
entry:
  tail call void @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj(ptr noundef %this, i32 noundef 1)
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !51
  %i = load ptr, ptr %fElemList, align 8, !tbaa !51
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !25
  %i1 = load i32, ptr %fCurCount, align 4, !tbaa !25
  %idxprom = zext i32 %i1 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i, i64 %idxprom
  store ptr %toAdd, ptr %arrayidx, align 8, !tbaa !52
  %inc = add i32 %i1, 1
  store i32 %inc, ptr %fCurCount, align 4, !tbaa !25
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef ptr @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(ptr noundef %toRep, ptr noundef %manager) local_unnamed_addr #7 comdat align 2 {
entry:
  %tobool.not = icmp eq ptr %toRep, null
  br i1 %tobool.not, label %if.end, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %i = load i16, ptr %toRep, align 2, !tbaa !54
  %cmp1.i = icmp eq i16 %i, 0
  br i1 %cmp1.i, label %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit, label %while.cond.i

while.cond.i:                                     ; preds = %while.cond.i, %lor.lhs.false.i
  %src.pn.i = phi ptr [ %pszTmp.0.i, %while.cond.i ], [ %toRep, %lor.lhs.false.i ]
  %pszTmp.0.i = getelementptr inbounds i16, ptr %src.pn.i, i64 1
  %i1 = load i16, ptr %pszTmp.0.i, align 2, !tbaa !54
  %tobool.not.i = icmp eq i16 %i1, 0
  br i1 %tobool.not.i, label %while.end.i, label %while.cond.i, !llvm.loop !56

while.end.i:                                      ; preds = %while.cond.i
  %sub.ptr.lhs.cast.i = ptrtoint ptr %pszTmp.0.i to i64
  %sub.ptr.rhs.cast.i = ptrtoint ptr %toRep to i64
  %sub.ptr.sub.i = sub i64 2, %sub.ptr.rhs.cast.i
  %phi.cast = add i64 %sub.ptr.sub.i, %sub.ptr.lhs.cast.i
  %phi.bo11 = and i64 %phi.cast, 8589934590
  br label %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit

_ZN11xercesc_2_79XMLString9stringLenEPKt.exit:    ; preds = %while.end.i, %lor.lhs.false.i
  %retval.0.i = phi i64 [ %phi.bo11, %while.end.i ], [ 2, %lor.lhs.false.i ]
  %i2 = bitcast ptr %manager to ptr
  %vtable = load ptr, ptr %i2, align 8, !tbaa !19
  %i3 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit
  %i4 = bitcast ptr %vtable to ptr
  %i5 = tail call i1 @llvm.type.test(ptr %i4, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i5)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i6 = load ptr, ptr %vfn, align 8
  %call1 = tail call noundef ptr %i6(ptr noundef nonnull %manager, i64 noundef %retval.0.i)
  %i7 = bitcast ptr %call1 to ptr
  %i8 = bitcast ptr %toRep to ptr
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 2 %call1, ptr nonnull align 2 %i8, i64 %retval.0.i, i1 false)
  br label %if.end

if.end:                                           ; preds = %whpr.continue, %entry
  %ret.0 = phi ptr [ %i7, %whpr.continue ], [ null, %entry ]
  ret ptr %ret.0
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef ptr @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj(ptr noundef %this, i32 noundef %getAt) local_unnamed_addr #6 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !25
  %i = load i32, ptr %fCurCount, align 4, !tbaa !25
  %cmp.not = icmp ugt i32 %i, %getAt
  br i1 %cmp.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = tail call ptr @__cxa_allocate_exception(i64 48) #15
  %i1 = bitcast ptr %exception to ptr
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 5, !intel-tbaa !57
  %i2 = load ptr, ptr %fMemoryManager, align 8, !tbaa !57
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef %i1, ptr noundef @.str, i32 noundef 249, i32 noundef 116, ptr noundef %i2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.then
  tail call void @__cxa_throw(ptr %exception, ptr @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, ptr @_ZN11xercesc_2_712XMLExceptionD2Ev) #26
  unreachable

lpad:                                             ; preds = %if.then
  %i3 = landingpad { ptr, i32 }
          cleanup
  tail call void @__cxa_free_exception(ptr %exception) #15
  resume { ptr, i32 } %i3

if.end:                                           ; preds = %entry
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !51
  %i4 = load ptr, ptr %fElemList, align 8, !tbaa !51
  %idxprom = zext i32 %getAt to i64
  %arrayidx = getelementptr inbounds ptr, ptr %i4, i64 %idxprom
  %i5 = load ptr, ptr %arrayidx, align 8, !tbaa !52
  ret ptr %i5
}

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(ptr) #8

; Function Attrs: nofree
declare dso_local ptr @__cxa_begin_catch(ptr) local_unnamed_addr #9

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv(ptr noundef %this) local_unnamed_addr #10 comdat align 2 {
entry:
  %fObject = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", ptr %this, i64 0, i32 0, !intel-tbaa !21
  store ptr null, ptr %fObject, align 8, !tbaa !21
  %fToCall = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", ptr %this, i64 0, i32 1, !intel-tbaa !24
  %fToCall.repack = getelementptr inbounds { i64, i64 }, ptr %fToCall, i64 0, i32 0
  store i64 0, ptr %fToCall.repack, align 8, !tbaa !24
  %fToCall.repack2 = getelementptr inbounds { i64, i64 }, ptr %fToCall, i64 0, i32 1
  store i64 0, ptr %fToCall.repack2, align 8, !tbaa !24
  ret void
}

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #9

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #9

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %arg) local_unnamed_addr #11 comdat {
bb:
  %i = tail call ptr @__cxa_begin_catch(ptr %arg) #15
  tail call void @_ZSt9terminatev() #27
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #12

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev(ptr noundef %this) unnamed_addr #13 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %fObject = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", ptr %this, i64 0, i32 0, !intel-tbaa !21
  %i = load ptr, ptr %fObject, align 8, !tbaa !21
  %cmp.not = icmp eq ptr %i, null
  br i1 %cmp.not, label %if.end, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %fToCall = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", ptr %this, i64 0, i32 1, !intel-tbaa !24
  %.elt = getelementptr inbounds { i64, i64 }, ptr %fToCall, i64 0, i32 0
  %.unpack = load i64, ptr %.elt, align 8, !tbaa !24
  %cmp.ptr.not = icmp eq i64 %.unpack, 0
  br i1 %cmp.ptr.not, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %.elt4 = getelementptr inbounds { i64, i64 }, ptr %fToCall, i64 0, i32 1
  %.unpack5 = load i64, ptr %.elt4, align 8, !tbaa !24
  %i1 = bitcast ptr %i to ptr
  %i2 = getelementptr inbounds i8, ptr %i1, i64 %.unpack5
  %this.adjusted = bitcast ptr %i2 to ptr
  %i3 = and i64 %.unpack, 1
  %memptr.isvirtual.not = icmp eq i64 %i3, 0
  br i1 %memptr.isvirtual.not, label %memptr.nonvirtual, label %memptr.virtual

memptr.virtual:                                   ; preds = %if.then
  %i4 = bitcast ptr %i2 to ptr
  %vtable = load ptr, ptr %i4, align 8, !tbaa !19
  %i5 = add i64 %.unpack, -1
  %i6 = getelementptr i8, ptr %vtable, i64 %i5, !nosanitize !58
  %i7 = bitcast ptr %i6 to ptr, !nosanitize !58
  %memptr.virtualfn = load ptr, ptr %i7, align 8, !nosanitize !58
  br label %memptr.end

memptr.nonvirtual:                                ; preds = %if.then
  %memptr.nonvirtualfn = inttoptr i64 %.unpack to ptr
  br label %memptr.end

memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  %i8 = phi ptr [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  invoke void %i8(ptr noundef nonnull %this.adjusted)
          to label %if.end unwind label %terminate.lpad

if.end:                                           ; preds = %memptr.end, %land.lhs.true, %entry
  ret void

terminate.lpad:                                   ; preds = %memptr.end
  %i9 = landingpad { ptr, i32 }
          catch ptr null
  %i10 = extractvalue { ptr, i32 } %i9, 0
  tail call void @__clang_call_terminate(ptr %i10) #27
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev(ptr noundef %this) unnamed_addr #14 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !37
  %i = load ptr, ptr %fMemoryManager, align 8, !tbaa !37
  %fElemList2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", ptr %this, i64 0, i32 3, !intel-tbaa !36
  %i1 = bitcast ptr %fElemList2 to ptr
  %i2 = load ptr, ptr %i1, align 8, !tbaa !36
  %i3 = bitcast ptr %i to ptr
  %vtable = load ptr, ptr %i3, align 8, !tbaa !19
  %i4 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i5 = bitcast ptr %vtable to ptr
  %i6 = tail call i1 @llvm.type.test(ptr %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 3
  %i7 = load ptr, ptr %vfn, align 8
  invoke void %i7(ptr noundef nonnull %i, ptr noundef %i2)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %whpr.continue
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %i8 = landingpad { ptr, i32 }
          catch ptr null
  %i9 = extractvalue { ptr, i32 } %i8, 0
  tail call void @__clang_call_terminate(ptr %i9) #27
  unreachable
}

; Function Attrs: nounwind
declare dso_local void @_ZN11xercesc_2_77XMemorydlEPv(ptr noundef) local_unnamed_addr #5

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev(ptr noundef %this) unnamed_addr #14 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 4, !intel-tbaa !47
  %i = load ptr, ptr %fMemoryManager, align 8, !tbaa !47
  %fElemList2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", ptr %this, i64 0, i32 3, !intel-tbaa !46
  %i1 = bitcast ptr %fElemList2 to ptr
  %i2 = load ptr, ptr %i1, align 8, !tbaa !46
  %i3 = bitcast ptr %i to ptr
  %vtable = load ptr, ptr %i3, align 8, !tbaa !19
  %i4 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i5 = bitcast ptr %vtable to ptr
  %i6 = tail call i1 @llvm.type.test(ptr %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 3
  %i7 = load ptr, ptr %vfn, align 8
  invoke void %i7(ptr noundef nonnull %i, ptr noundef %i2)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %whpr.continue
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %i8 = landingpad { ptr, i32 }
          catch ptr null
  %i9 = extractvalue { ptr, i32 } %i8, 0
  tail call void @__clang_call_terminate(ptr %i9) #27
  unreachable
}

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #15

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata) #16

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #17

; Function Attrs: nofree
declare dso_local noalias ptr @__cxa_allocate_exception(i64) local_unnamed_addr #9

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(ptr noundef %this, ptr noundef %srcFile, i32 noundef %srcLine, i32 noundef %toThrow, ptr noundef %memoryManager) unnamed_addr #18 comdat align 2 personality ptr @__gxx_personality_v0 {
entry:
  %i = getelementptr %"class.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %this, i64 0, i32 0
  tail call void @_ZN11xercesc_2_712XMLExceptionC2EPKcjPNS_13MemoryManagerE(ptr noundef %i, ptr noundef %srcFile, i32 noundef %srcLine, ptr noundef %memoryManager)
  %i1 = getelementptr %"class.xercesc_2_7::ArrayIndexOutOfBoundsException", ptr %this, i64 0, i32 0, i32 0
  invoke void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef %i, i32 noundef %toThrow)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret void

lpad:                                             ; preds = %entry
  %i2 = landingpad { ptr, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef %i) #15
  resume { ptr, i32 } %i2
}

; Function Attrs: nofree
declare dso_local void @__cxa_free_exception(ptr) local_unnamed_addr #9

; Function Attrs: nounwind
declare dso_local void @_ZN11xercesc_2_712XMLExceptionD2Ev(ptr noundef) unnamed_addr #19

; Function Attrs: nofree noreturn
declare dso_local void @__cxa_throw(ptr, ptr, ptr) local_unnamed_addr #20

declare dso_local void @_ZN11xercesc_2_712XMLExceptionC2EPKcjPNS_13MemoryManagerE(ptr noundef, ptr noundef, i32 noundef, ptr noundef) unnamed_addr #21

declare dso_local void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(ptr noundef, i32 noundef) local_unnamed_addr #4

declare dso_local void @_ZN11xercesc_2_712XMLExceptionC2ERKS0_(ptr noundef, ptr noundef nonnull align 8 dereferenceable(48)) unnamed_addr #21

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE(ptr noundef %this, i32 noundef %maxElems, i1 noundef zeroext %adoptElems, ptr noundef %manager) unnamed_addr #1 comdat align 2 {
entry:
  %frombool = zext i1 %adoptElems to i8
  %i = getelementptr %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 0
  %fAdoptedElems = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 1, !intel-tbaa !59
  store i8 %frombool, ptr %fAdoptedElems, align 8, !tbaa !59
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !25
  store i32 0, ptr %fCurCount, align 4, !tbaa !25
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 3, !intel-tbaa !60
  store i32 %maxElems, ptr %fMaxCount, align 8, !tbaa !60
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !51
  store ptr null, ptr %fElemList, align 8, !tbaa !51
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 5, !intel-tbaa !57
  store ptr %manager, ptr %fMemoryManager, align 8, !tbaa !57
  %conv = zext i32 %maxElems to i64
  %mul = shl nuw nsw i64 %conv, 3
  %i1 = bitcast ptr %manager to ptr
  %vtable = load ptr, ptr %i1, align 8, !tbaa !19
  %i2 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %i3 = bitcast ptr %vtable to ptr
  %i4 = tail call i1 @llvm.type.test(ptr %i3, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i5 = load ptr, ptr %vfn, align 8
  %call = tail call noundef ptr %i5(ptr noundef nonnull %manager, i64 noundef %mul)
  %i6 = bitcast ptr %call to ptr
  store ptr %i6, ptr %fElemList, align 8, !tbaa !51
  %cmp11.not = icmp eq i32 %maxElems, 0
  br i1 %cmp11.not, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %whpr.continue
  call void @llvm.memset.p0.i64(ptr align 8 %call, i8 0, i64 %mul, i1 false)
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body.preheader, %whpr.continue
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj(ptr noundef %this, i32 noundef %length) local_unnamed_addr #6 comdat align 2 {
entry:
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 2, !intel-tbaa !25
  %i = load i32, ptr %fCurCount, align 4, !tbaa !25
  %add = add i32 %i, %length
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 3, !intel-tbaa !60
  %i1 = load i32, ptr %fMaxCount, align 8, !tbaa !60
  %cmp.not = icmp ugt i32 %add, %i1
  br i1 %cmp.not, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %div = lshr i32 %i1, 1
  %add4 = add i32 %div, %i1
  %cmp5 = icmp ult i32 %add, %add4
  %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 5, !intel-tbaa !57
  %i2 = load ptr, ptr %fMemoryManager, align 8, !tbaa !57
  %conv = zext i32 %spec.select to i64
  %mul = shl nuw nsw i64 %conv, 3
  %i3 = bitcast ptr %i2 to ptr
  %vtable = load ptr, ptr %i3, align 8, !tbaa !19
  %i4 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.end
  %i5 = bitcast ptr %vtable to ptr
  %i6 = tail call i1 @llvm.type.test(ptr %i5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.end
  %vfn = getelementptr inbounds ptr, ptr %vtable, i64 2
  %i7 = load ptr, ptr %vfn, align 8
  %call = tail call noundef ptr %i7(ptr noundef nonnull %i2, i64 noundef %mul)
  %i8 = bitcast ptr %call to ptr
  %i9 = load i32, ptr %fCurCount, align 4, !tbaa !25
  %cmp1347.not = icmp eq i32 %i9, 0
  br i1 %cmp1347.not, label %for.cond16.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %whpr.continue
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !51
  %i10 = load ptr, ptr %fElemList, align 8, !tbaa !51
  %wide.trip.count = zext i32 %i9 to i64
  br label %for.body

for.cond16.preheader:                             ; preds = %for.body, %whpr.continue
  %cmp1749 = icmp ult i32 %i9, %spec.select
  br i1 %cmp1749, label %for.body18.preheader, label %for.end23

for.body18.preheader:                             ; preds = %for.cond16.preheader
  %i11 = zext i32 %i9 to i64
  %i12 = shl nuw nsw i64 %i11, 3
  %scevgep = getelementptr i8, ptr %call, i64 %i12
  %i13 = xor i32 %i9, -1
  %i14 = add i32 %spec.select, %i13
  %i15 = zext i32 %i14 to i64
  %i16 = shl nuw nsw i64 %i15, 3
  %i17 = add nuw nsw i64 %i16, 8
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(1) %scevgep, i8 0, i64 %i17, i1 false)
  br label %for.end23

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %i10, i64 %indvars.iv
  %i18 = load ptr, ptr %arrayidx, align 8, !tbaa !52
  %arrayidx15 = getelementptr inbounds ptr, ptr %i8, i64 %indvars.iv
  store ptr %i18, ptr %arrayidx15, align 8, !tbaa !52
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond16.preheader, label %for.body, !llvm.loop !61

for.end23:                                        ; preds = %for.body18.preheader, %for.cond16.preheader
  %i19 = load ptr, ptr %fMemoryManager, align 8, !tbaa !57
  %fElemList25 = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", ptr %this, i64 0, i32 4, !intel-tbaa !51
  %i20 = bitcast ptr %fElemList25 to ptr
  %i21 = load ptr, ptr %i20, align 8, !tbaa !51
  %i22 = bitcast ptr %i19 to ptr
  %vtable26 = load ptr, ptr %i22, align 8, !tbaa !19
  %i23 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %i23, label %whpr.wrap27, label %whpr.continue28

whpr.wrap27:                                      ; preds = %for.end23
  %i24 = bitcast ptr %vtable26 to ptr
  %i25 = tail call i1 @llvm.type.test(ptr %i24, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %i25)
  br label %whpr.continue28

whpr.continue28:                                  ; preds = %whpr.wrap27, %for.end23
  %vfn29 = getelementptr inbounds ptr, ptr %vtable26, i64 3
  %i26 = load ptr, ptr %vfn29, align 8
  tail call void %i26(ptr noundef nonnull %i19, ptr noundef %i21)
  store ptr %i8, ptr %fElemList25, align 8, !tbaa !51
  store i32 %spec.select, ptr %fMaxCount, align 8, !tbaa !60
  br label %cleanup

cleanup:                                          ; preds = %whpr.continue28, %entry
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.umax.i32(i32, i32) #16

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #22

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #22

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #23

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.fakeload.p0(ptr, metadata) #24

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #25

attributes #0 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #8 = { nounwind readnone }
attributes #9 = { nofree }
attributes #10 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #11 = { noinline noreturn nounwind "pre_loopopt" }
attributes #12 = { nofree noreturn nounwind }
attributes #13 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #14 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #15 = { nounwind }
attributes #16 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #17 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #18 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #19 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #20 = { nofree noreturn }
attributes #21 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #22 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #23 = { argmemonly nofree nounwind willreturn }
attributes #24 = { nounwind readnone speculatable }
attributes #25 = { argmemonly nofree nounwind willreturn writeonly }
attributes #26 = { noreturn }
attributes #27 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = !{!7, !8, i64 0}
!7 = !{!"struct@_ZTSN11xercesc_2_713FieldValueMapE", !8, i64 0, !11, i64 8, !12, i64 16, !13, i64 24}
!8 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C++ TBAA"}
!11 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !9, i64 0}
!12 = !{!"pointer@_ZTSPN11xercesc_2_716RefArrayVectorOfItEE", !9, i64 0}
!13 = !{!"pointer@_ZTSPN11xercesc_2_713MemoryManagerE", !9, i64 0}
!14 = !{!7, !11, i64 8}
!15 = !{!7, !12, i64 16}
!16 = !{!7, !13, i64 24}
!17 = distinct !{!17, !18}
!18 = !{!"llvm.loop.mustprogress"}
!19 = !{!20, !20, i64 0}
!20 = !{!"vtable pointer", !10, i64 0}
!21 = !{!22, !23, i64 0}
!22 = !{!"struct@_ZTSN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEE", !23, i64 0, !9, i64 8}
!23 = !{!"pointer@_ZTSPN11xercesc_2_713FieldValueMapE", !9, i64 0}
!24 = !{!22, !9, i64 8}
!25 = !{!26, !28, i64 12}
!26 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfItEE", !27, i64 8, !28, i64 12, !28, i64 16, !29, i64 24, !13, i64 32}
!27 = !{!"bool", !9, i64 0}
!28 = !{!"int", !9, i64 0}
!29 = !{!"pointer@_ZTSPPt", !9, i64 0}
!30 = !{!31, !27, i64 0}
!31 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !27, i64 0, !28, i64 4, !28, i64 8, !32, i64 16, !13, i64 24}
!32 = !{!"pointer@_ZTSPPN11xercesc_2_78IC_FieldE", !9, i64 0}
!33 = !{i8 0, i8 2}
!34 = !{!31, !28, i64 4}
!35 = !{!31, !28, i64 8}
!36 = !{!31, !32, i64 16}
!37 = !{!31, !13, i64 24}
!38 = !{!39, !39, i64 0}
!39 = !{!"pointer@_ZTSPN11xercesc_2_78IC_FieldE", !9, i64 0}
!40 = distinct !{!40, !18}
!41 = !{!42, !27, i64 0}
!42 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !27, i64 0, !28, i64 4, !28, i64 8, !43, i64 16, !13, i64 24}
!43 = !{!"pointer@_ZTSPPN11xercesc_2_717DatatypeValidatorE", !9, i64 0}
!44 = !{!42, !28, i64 4}
!45 = !{!42, !28, i64 8}
!46 = !{!42, !43, i64 16}
!47 = !{!42, !13, i64 24}
!48 = !{!49, !49, i64 0}
!49 = !{!"pointer@_ZTSPN11xercesc_2_717DatatypeValidatorE", !9, i64 0}
!50 = distinct !{!50, !18}
!51 = !{!26, !29, i64 24}
!52 = !{!53, !53, i64 0}
!53 = !{!"pointer@_ZTSPt", !9, i64 0}
!54 = !{!55, !55, i64 0}
!55 = !{!"short", !9, i64 0}
!56 = distinct !{!56, !18}
!57 = !{!26, !13, i64 32}
!58 = !{}
!59 = !{!26, !27, i64 8}
!60 = !{!26, !28, i64 16}
!61 = distinct !{!61, !18}
; end INTEL_FEATURE_SW_ADVANCED
