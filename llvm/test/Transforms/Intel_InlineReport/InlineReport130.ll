; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes=inline -lto-inline-cost -inline-threshold=0 -inline-report=0xf867 -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-BG
; RUN: opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -sycl-host -sycl-optimization-mode -inline-threshold=0 -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -sycl-host -inline-threshold=0 -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -sycl-optimization-mode -inline-threshold=0 -S -inline-report=0xf867 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inline-threshold=0 -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD
; RUN: opt -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0 -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefix=CHECK-BG
; RUN: opt -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -sycl-host -sycl-optimization-mode -inline-threshold=0 -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD
; RUN: opt -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -sycl-host -inline-threshold=0 -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD
; RUN: opt -passes=inlinereportsetup -inline-report=0xf8e6 < %s -S | opt -passes=inline -lto-inline-cost -inlining-huge-lto-odr-bb-count=0 -sycl-optimization-mode -inline-threshold=0 -S -inline-report=0xf8e6 -S | opt -passes=inlinereportemitter -inline-report=0xf8e6 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-SM,CHECK-SM-MD

; Check that without -inlining-huge-lto-odr-bb-count=0 -inline-threshold=0
; that single callsite link once ODR functions get inlined regardless of size.
; Also check that adding -sycl-host and/or -sycl-optimization-mode has the same effect, even 
; if -inlining-lto-huge-odr-bb-count=0 -inline-threshold=0 are passed.
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

%"class.xercesc_2_7::FieldValueMap" = type { %"class.xercesc_2_7::ValueVectorOf"*, %"class.xercesc_2_7::ValueVectorOf.0"*, %"class.xercesc_2_7::RefArrayVectorOf"*, %"class.xercesc_2_7::MemoryManager"* }
%"class.xercesc_2_7::ValueVectorOf" = type { i8, i32, i32, %"class.xercesc_2_7::IC_Field"**, %"class.xercesc_2_7::MemoryManager"* }
%"class.xercesc_2_7::IC_Field" = type opaque
%"class.xercesc_2_7::ValueVectorOf.0" = type { i8, i32, i32, %"class.xercesc_2_7::DatatypeValidator"**, %"class.xercesc_2_7::MemoryManager"* }
%"class.xercesc_2_7::DatatypeValidator" = type opaque
%"class.xercesc_2_7::RefArrayVectorOf" = type { %"class.xercesc_2_7::BaseRefVectorOf" }
%"class.xercesc_2_7::BaseRefVectorOf" = type { i32 (...)**, i8, i32, i32, i16**, %"class.xercesc_2_7::MemoryManager"* }
%"class.xercesc_2_7::MemoryManager" = type { i32 (...)** }
%"class.xercesc_2_7::JanitorMemFunCall" = type { %"class.xercesc_2_7::FieldValueMap"*, { i64, i64 } }
%"class.xercesc_2_7::ArrayIndexOutOfBoundsException" = type { %"class.xercesc_2_7::XMLException" }
%"class.xercesc_2_7::XMLException" = type { i32 (...)**, i32, i8*, i32, i16*, %"class.xercesc_2_7::MemoryManager"* }

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

@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTSN11xercesc_2_720OutOfMemoryExceptionE = linkonce_odr dso_local constant [38 x i8] c"N11xercesc_2_720OutOfMemoryExceptionE\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTSN11xercesc_2_77XMemoryE = linkonce_odr dso_local constant [24 x i8] c"N11xercesc_2_77XMemoryE\00", comdat, align 1
@_ZTIN11xercesc_2_77XMemoryE = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([24 x i8], [24 x i8]* @_ZTSN11xercesc_2_77XMemoryE, i32 0, i32 0) }, comdat, align 8
@_ZTIN11xercesc_2_720OutOfMemoryExceptionE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([38 x i8], [38 x i8]* @_ZTSN11xercesc_2_720OutOfMemoryExceptionE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTIN11xercesc_2_77XMemoryE to i8*) }, comdat, align 8
@.str = private unnamed_addr constant [33 x i8] c"./xercesc/util/BaseRefVectorOf.c\00", align 1
@_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = linkonce_odr dso_local constant [48 x i8] c"N11xercesc_2_730ArrayIndexOutOfBoundsExceptionE\00", comdat, align 1
@_ZTIN11xercesc_2_712XMLExceptionE = external dso_local constant i8*
@_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([48 x i8], [48 x i8]* @_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE, i32 0, i32 0), i8* bitcast (i8** @_ZTIN11xercesc_2_712XMLExceptionE to i8*) }, comdat, align 8
@_ZN11xercesc_2_76XMLUni37fgArrayIndexOutOfBoundsException_NameE = external dso_local constant [0 x i16], align 2
@.str.1 = private unnamed_addr constant [31 x i8] c"./xercesc/util/ValueVectorOf.c\00", align 1
@_ZTSN11xercesc_2_716RefArrayVectorOfItEE = linkonce_odr dso_local constant [37 x i8] c"N11xercesc_2_716RefArrayVectorOfItEE\00", comdat, align 1
@_ZTSN11xercesc_2_715BaseRefVectorOfItEE = linkonce_odr dso_local constant [36 x i8] c"N11xercesc_2_715BaseRefVectorOfItEE\00", comdat, align 1
@_ZTIN11xercesc_2_715BaseRefVectorOfItEE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([36 x i8], [36 x i8]* @_ZTSN11xercesc_2_715BaseRefVectorOfItEE, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTIN11xercesc_2_77XMemoryE to i8*) }, comdat, align 8
@_ZTIN11xercesc_2_716RefArrayVectorOfItEE = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([37 x i8], [37 x i8]* @_ZTSN11xercesc_2_716RefArrayVectorOfItEE, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_715BaseRefVectorOfItEE to i8*) }, comdat, align 8
@.str.2 = private unnamed_addr constant [34 x i8] c"./xercesc/util/RefArrayVectorOf.c\00", align 1

@_ZN11xercesc_2_713FieldValueMapC1ERKS0_ = dso_local unnamed_addr alias void (%"class.xercesc_2_7::FieldValueMap"*, %"class.xercesc_2_7::FieldValueMap"*), void (%"class.xercesc_2_7::FieldValueMap"*, %"class.xercesc_2_7::FieldValueMap"*)* @_ZN11xercesc_2_713FieldValueMapC2ERKS0_

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8* noundef %0) local_unnamed_addr #0

; Function Attrs: uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMapC2ERKS0_(%"class.xercesc_2_7::FieldValueMap"* noundef %this, %"class.xercesc_2_7::FieldValueMap"* noundef nonnull align 8 dereferenceable(32) %other) unnamed_addr #2 align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %cleanup = alloca %"class.xercesc_2_7::JanitorMemFunCall", align 8
  %fFields = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 0, !intel-tbaa !24
  store %"class.xercesc_2_7::ValueVectorOf"* null, %"class.xercesc_2_7::ValueVectorOf"** %fFields, align 8, !tbaa !24
  %fValidators = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 1, !intel-tbaa !32
  store %"class.xercesc_2_7::ValueVectorOf.0"* null, %"class.xercesc_2_7::ValueVectorOf.0"** %fValidators, align 8, !tbaa !32
  %fValues = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 2, !intel-tbaa !33
  store %"class.xercesc_2_7::RefArrayVectorOf"* null, %"class.xercesc_2_7::RefArrayVectorOf"** %fValues, align 8, !tbaa !33
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 3, !intel-tbaa !34
  %fMemoryManager2 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %other, i64 0, i32 3, !intel-tbaa !34
  %0 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager2, align 8, !tbaa !34
  store %"class.xercesc_2_7::MemoryManager"* %0, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !34
  %fFields3 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %other, i64 0, i32 0, !intel-tbaa !24
  %1 = load %"class.xercesc_2_7::ValueVectorOf"*, %"class.xercesc_2_7::ValueVectorOf"** %fFields3, align 8, !tbaa !24
  %tobool.not = icmp eq %"class.xercesc_2_7::ValueVectorOf"* %1, null
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %2 = bitcast %"class.xercesc_2_7::JanitorMemFunCall"* %cleanup to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %2) #17
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE(%"class.xercesc_2_7::JanitorMemFunCall"* noundef nonnull %cleanup, %"class.xercesc_2_7::FieldValueMap"* noundef %this, i64 ptrtoint (void (%"class.xercesc_2_7::FieldValueMap"*)* @_ZN11xercesc_2_713FieldValueMap7cleanUpEv to i64), i64 0)
  %fValues4 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %other, i64 0, i32 2, !intel-tbaa !33
  %3 = bitcast %"class.xercesc_2_7::RefArrayVectorOf"** %fValues4 to %"class.xercesc_2_7::BaseRefVectorOf"**
  %4 = load %"class.xercesc_2_7::BaseRefVectorOf"*, %"class.xercesc_2_7::BaseRefVectorOf"** %3, align 8, !tbaa !33
  %call = call noundef i32 @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %4)
  %5 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !34
  %call7 = invoke noundef i8* @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, %"class.xercesc_2_7::MemoryManager"* noundef %5)
          to label %invoke.cont6 unwind label %lpad

invoke.cont6:                                     ; preds = %if.then
  %6 = bitcast i8* %call7 to %"class.xercesc_2_7::ValueVectorOf"*
  %7 = load %"class.xercesc_2_7::ValueVectorOf"*, %"class.xercesc_2_7::ValueVectorOf"** %fFields3, align 8, !tbaa !24
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(%"class.xercesc_2_7::ValueVectorOf"* noundef %6, %"class.xercesc_2_7::ValueVectorOf"* noundef nonnull align 8 dereferenceable(32) %7)
          to label %invoke.cont10 unwind label %lpad9

invoke.cont10:                                    ; preds = %invoke.cont6
  store %"class.xercesc_2_7::ValueVectorOf"* %6, %"class.xercesc_2_7::ValueVectorOf"** %fFields, align 8, !tbaa !24
  %8 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !34
  %call14 = invoke noundef i8* @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 32, %"class.xercesc_2_7::MemoryManager"* noundef %8)
          to label %invoke.cont13 unwind label %lpad

invoke.cont13:                                    ; preds = %invoke.cont10
  %9 = bitcast i8* %call14 to %"class.xercesc_2_7::ValueVectorOf.0"*
  %fValidators15 = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %other, i64 0, i32 1, !intel-tbaa !32
  %10 = load %"class.xercesc_2_7::ValueVectorOf.0"*, %"class.xercesc_2_7::ValueVectorOf.0"** %fValidators15, align 8, !tbaa !32
  invoke void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_(%"class.xercesc_2_7::ValueVectorOf.0"* noundef %9, %"class.xercesc_2_7::ValueVectorOf.0"* noundef nonnull align 8 dereferenceable(32) %10)
          to label %invoke.cont17 unwind label %lpad16

invoke.cont17:                                    ; preds = %invoke.cont13
  store %"class.xercesc_2_7::ValueVectorOf.0"* %9, %"class.xercesc_2_7::ValueVectorOf.0"** %fValidators, align 8, !tbaa !32
  %11 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !34
  %call21 = invoke noundef i8* @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef 40, %"class.xercesc_2_7::MemoryManager"* noundef %11)
          to label %invoke.cont20 unwind label %lpad

invoke.cont20:                                    ; preds = %invoke.cont17
  %12 = bitcast i8* %call21 to %"class.xercesc_2_7::RefArrayVectorOf"*
  %13 = load %"class.xercesc_2_7::ValueVectorOf"*, %"class.xercesc_2_7::ValueVectorOf"** %fFields3, align 8, !tbaa !24
  %call25 = call noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv(%"class.xercesc_2_7::ValueVectorOf"* noundef %13)
  %14 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !34
  invoke void @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE(%"class.xercesc_2_7::RefArrayVectorOf"* noundef %12, i32 noundef %call25, i1 noundef zeroext true, %"class.xercesc_2_7::MemoryManager"* noundef %14)
          to label %invoke.cont27 unwind label %lpad23

invoke.cont27:                                    ; preds = %invoke.cont20
  store %"class.xercesc_2_7::RefArrayVectorOf"* %12, %"class.xercesc_2_7::RefArrayVectorOf"** %fValues, align 8, !tbaa !33
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %invoke.cont27
  %i.0 = phi i32 [ 0, %invoke.cont27 ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv(%"class.xercesc_2_7::JanitorMemFunCall"* noundef nonnull %cleanup)
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev(%"class.xercesc_2_7::JanitorMemFunCall"* noundef nonnull %cleanup) #17
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %2) #17
  br label %if.end

lpad:                                             ; preds = %invoke.cont17, %invoke.cont10, %if.then
  %15 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_720OutOfMemoryExceptionE to i8*)
  br label %ehcleanup

lpad9:                                            ; preds = %invoke.cont6
  %16 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_720OutOfMemoryExceptionE to i8*)
  call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(i8* noundef %call7, %"class.xercesc_2_7::MemoryManager"* noundef %5) #17
  br label %ehcleanup

lpad16:                                           ; preds = %invoke.cont13
  %17 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_720OutOfMemoryExceptionE to i8*)
  call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(i8* noundef %call14, %"class.xercesc_2_7::MemoryManager"* noundef %8) #17
  br label %ehcleanup

lpad23:                                           ; preds = %invoke.cont20
  %18 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_720OutOfMemoryExceptionE to i8*)
  call void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(i8* noundef %call21, %"class.xercesc_2_7::MemoryManager"* noundef %11) #17
  br label %ehcleanup

for.body:                                         ; preds = %for.cond
  %19 = bitcast %"class.xercesc_2_7::RefArrayVectorOf"** %fValues to %"class.xercesc_2_7::BaseRefVectorOf"**
  %20 = load %"class.xercesc_2_7::BaseRefVectorOf"*, %"class.xercesc_2_7::BaseRefVectorOf"** %19, align 8, !tbaa !33
  %21 = bitcast %"class.xercesc_2_7::RefArrayVectorOf"** %fValues4 to %"class.xercesc_2_7::BaseRefVectorOf"**
  %22 = load %"class.xercesc_2_7::BaseRefVectorOf"*, %"class.xercesc_2_7::BaseRefVectorOf"** %21, align 8, !tbaa !33
  %call33 = invoke noundef i16* @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %22, i32 noundef %i.0)
          to label %invoke.cont32 unwind label %lpad31

invoke.cont32:                                    ; preds = %for.body
  %23 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !34
  %call36 = invoke noundef i16* @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(i16* noundef %call33, %"class.xercesc_2_7::MemoryManager"* noundef %23)
          to label %invoke.cont35 unwind label %lpad31

invoke.cont35:                                    ; preds = %invoke.cont32
  invoke void @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %20, i16* noundef %call36)
          to label %for.inc unwind label %lpad31

for.inc:                                          ; preds = %invoke.cont35
  %inc = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !35

lpad31:                                           ; preds = %invoke.cont35, %invoke.cont32, %for.body
  %24 = landingpad { i8*, i32 }
          cleanup
          catch i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_720OutOfMemoryExceptionE to i8*)
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad31, %lpad23, %lpad16, %lpad9, %lpad
  %.pn = phi { i8*, i32 } [ %24, %lpad31 ], [ %18, %lpad23 ], [ %15, %lpad ], [ %17, %lpad16 ], [ %16, %lpad9 ]
  %ehselector.slot.0 = extractvalue { i8*, i32 } %.pn, 1
  %25 = call i32 @llvm.eh.typeid.for(i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_720OutOfMemoryExceptionE to i8*)) #17
  %matches = icmp eq i32 %ehselector.slot.0, %25
  br i1 %matches, label %catch, label %ehcleanup46

catch:                                            ; preds = %ehcleanup
  %exn.slot.0 = extractvalue { i8*, i32 } %.pn, 0
  %26 = call i8* @__cxa_begin_catch(i8* %exn.slot.0) #17
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv(%"class.xercesc_2_7::JanitorMemFunCall"* noundef nonnull %cleanup)
  invoke void @__cxa_rethrow() #29
          to label %unreachable unwind label %lpad39

lpad39:                                           ; preds = %catch
  %27 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %ehcleanup46 unwind label %terminate.lpad

ehcleanup46:                                      ; preds = %lpad39, %ehcleanup
  %lpad.val50.merged = phi { i8*, i32 } [ %.pn, %ehcleanup ], [ %27, %lpad39 ]
  call void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev(%"class.xercesc_2_7::JanitorMemFunCall"* noundef nonnull %cleanup) #17
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %2) #17
  resume { i8*, i32 } %lpad.val50.merged

if.end:                                           ; preds = %for.cond.cleanup, %entry
  ret void

terminate.lpad:                                   ; preds = %lpad39
  %28 = landingpad { i8*, i32 }
          catch i8* null
  %29 = extractvalue { i8*, i32 } %28, 0
  call void @__clang_call_terminate(i8* %29) #30
  unreachable

unreachable:                                      ; preds = %catch
  unreachable
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1) #3

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_ZN11xercesc_2_713FieldValueMap7cleanUpEv(%"class.xercesc_2_7::FieldValueMap"* nocapture noundef readonly %this) #4 align 2 {
entry:
  %fFields = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 0, !intel-tbaa !24
  %0 = load %"class.xercesc_2_7::ValueVectorOf"*, %"class.xercesc_2_7::ValueVectorOf"** %fFields, align 8, !tbaa !24
  %isnull = icmp eq %"class.xercesc_2_7::ValueVectorOf"* %0, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev(%"class.xercesc_2_7::ValueVectorOf"* noundef nonnull %0) #17
  %1 = getelementptr %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %0, i64 0, i32 0
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(i8* noundef %1) #17
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %fValidators = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 1, !intel-tbaa !32
  %2 = load %"class.xercesc_2_7::ValueVectorOf.0"*, %"class.xercesc_2_7::ValueVectorOf.0"** %fValidators, align 8, !tbaa !32
  %isnull2 = icmp eq %"class.xercesc_2_7::ValueVectorOf.0"* %2, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  tail call void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev(%"class.xercesc_2_7::ValueVectorOf.0"* noundef nonnull %2) #17
  %3 = getelementptr %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %2, i64 0, i32 0
  tail call void @_ZN11xercesc_2_77XMemorydlEPv(i8* noundef %3) #17
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  %fValues = getelementptr inbounds %"class.xercesc_2_7::FieldValueMap", %"class.xercesc_2_7::FieldValueMap"* %this, i64 0, i32 2, !intel-tbaa !33
  %4 = load %"class.xercesc_2_7::RefArrayVectorOf"*, %"class.xercesc_2_7::RefArrayVectorOf"** %fValues, align 8, !tbaa !33
  %isnull5 = icmp eq %"class.xercesc_2_7::RefArrayVectorOf"* %4, null
  br i1 %isnull5, label %delete.end7, label %delete.notnull6

delete.notnull6:                                  ; preds = %delete.end4
  %5 = bitcast %"class.xercesc_2_7::RefArrayVectorOf"* %4 to void (%"class.xercesc_2_7::RefArrayVectorOf"*)***
  %vtable = load void (%"class.xercesc_2_7::RefArrayVectorOf"*)**, void (%"class.xercesc_2_7::RefArrayVectorOf"*)*** %5, align 8, !tbaa !37
  %6 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %6, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %delete.notnull6
  %7 = bitcast void (%"class.xercesc_2_7::RefArrayVectorOf"*)** %vtable to i8*
  %8 = tail call i1 @llvm.type.test(i8* %7, metadata !"_ZTSN11xercesc_2_716RefArrayVectorOfItEE")
  tail call void @llvm.assume(i1 %8)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %delete.notnull6
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::RefArrayVectorOf"*)*, void (%"class.xercesc_2_7::RefArrayVectorOf"*)** %vtable, i64 1
  %9 = load void (%"class.xercesc_2_7::RefArrayVectorOf"*)*, void (%"class.xercesc_2_7::RefArrayVectorOf"*)** %vfn, align 8
  tail call void %9(%"class.xercesc_2_7::RefArrayVectorOf"* noundef nonnull %4) #17
  br label %delete.end7

delete.end7:                                      ; preds = %whpr.continue, %delete.end4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEC2EPS1_MS1_FvvE(%"class.xercesc_2_7::JanitorMemFunCall"* noundef %this, %"class.xercesc_2_7::FieldValueMap"* noundef %object, i64 %toCall.coerce0, i64 %toCall.coerce1) unnamed_addr #5 comdat align 2 {
entry:
  %fObject = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", %"class.xercesc_2_7::JanitorMemFunCall"* %this, i64 0, i32 0, !intel-tbaa !39
  store %"class.xercesc_2_7::FieldValueMap"* %object, %"class.xercesc_2_7::FieldValueMap"** %fObject, align 8, !tbaa !39
  %fToCall = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", %"class.xercesc_2_7::JanitorMemFunCall"* %this, i64 0, i32 1, !intel-tbaa !42
  %fToCall.repack = getelementptr inbounds { i64, i64 }, { i64, i64 }* %fToCall, i64 0, i32 0
  store i64 %toCall.coerce0, i64* %fToCall.repack, align 8, !tbaa !42
  %fToCall.repack3 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %fToCall, i64 0, i32 1
  store i64 %toCall.coerce1, i64* %fToCall.repack3, align 8, !tbaa !42
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i32 @_ZNK11xercesc_2_715BaseRefVectorOfItE4sizeEv(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %this) local_unnamed_addr #4 comdat align 2 {
entry:
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 2, !intel-tbaa !43
  %0 = load i32, i32* %fCurCount, align 4, !tbaa !43
  ret i32 %0
}

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local noundef i8* @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 noundef %0, %"class.xercesc_2_7::MemoryManager"* noundef %1) local_unnamed_addr #6

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEC2ERKS3_(%"class.xercesc_2_7::ValueVectorOf"* noundef %this, %"class.xercesc_2_7::ValueVectorOf"* noundef nonnull align 8 dereferenceable(32) %toCopy) unnamed_addr #2 comdat align 2 {
entry:
  %fCallDestructor = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 0, !intel-tbaa !48
  %fCallDestructor2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %toCopy, i64 0, i32 0, !intel-tbaa !48
  %0 = load i8, i8* %fCallDestructor2, align 8, !tbaa !48, !range !51
  store i8 %0, i8* %fCallDestructor, align 8, !tbaa !48
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 1, !intel-tbaa !52
  %fCurCount3 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %toCopy, i64 0, i32 1, !intel-tbaa !52
  %1 = load i32, i32* %fCurCount3, align 4, !tbaa !52
  store i32 %1, i32* %fCurCount, align 4, !tbaa !52
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 2, !intel-tbaa !53
  %fMaxCount4 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %toCopy, i64 0, i32 2, !intel-tbaa !53
  %2 = load i32, i32* %fMaxCount4, align 8, !tbaa !53
  store i32 %2, i32* %fMaxCount, align 8, !tbaa !53
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 3, !intel-tbaa !54
  store %"class.xercesc_2_7::IC_Field"** null, %"class.xercesc_2_7::IC_Field"*** %fElemList, align 8, !tbaa !54
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 4, !intel-tbaa !55
  %fMemoryManager5 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %toCopy, i64 0, i32 4, !intel-tbaa !55
  %3 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager5, align 8, !tbaa !55
  store %"class.xercesc_2_7::MemoryManager"* %3, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !55
  %conv = zext i32 %2 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %4 = bitcast %"class.xercesc_2_7::MemoryManager"* %3 to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %4, align 8, !tbaa !37
  %5 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %5, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %6 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %7 = tail call i1 @llvm.type.test(i8* %6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %7)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %8 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = tail call noundef i8* %8(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %3, i64 noundef %mul)
  %9 = bitcast i8* %call to %"class.xercesc_2_7::IC_Field"**
  store %"class.xercesc_2_7::IC_Field"** %9, %"class.xercesc_2_7::IC_Field"*** %fElemList, align 8, !tbaa !54
  %10 = load i32, i32* %fMaxCount, align 8, !tbaa !53
  %conv11 = zext i32 %10 to i64
  %mul12 = shl nuw nsw i64 %conv11, 3
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul12, i1 false)
  %11 = load i32, i32* %fCurCount, align 4, !tbaa !52
  %cmp27.not = icmp eq i32 %11, 0
  br i1 %cmp27.not, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %whpr.continue
  %fElemList14 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %toCopy, i64 0, i32 3, !intel-tbaa !54
  %12 = load %"class.xercesc_2_7::IC_Field"**, %"class.xercesc_2_7::IC_Field"*** %fElemList14, align 8, !tbaa !54
  %13 = load %"class.xercesc_2_7::IC_Field"**, %"class.xercesc_2_7::IC_Field"*** %fElemList, align 8, !tbaa !54
  %wide.trip.count = zext i32 %11 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %whpr.continue
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds %"class.xercesc_2_7::IC_Field"*, %"class.xercesc_2_7::IC_Field"** %12, i64 %indvars.iv
  %14 = load %"class.xercesc_2_7::IC_Field"*, %"class.xercesc_2_7::IC_Field"** %arrayidx, align 8, !tbaa !56
  %arrayidx17 = getelementptr inbounds %"class.xercesc_2_7::IC_Field"*, %"class.xercesc_2_7::IC_Field"** %13, i64 %indvars.iv
  store %"class.xercesc_2_7::IC_Field"* %14, %"class.xercesc_2_7::IC_Field"** %arrayidx17, align 8, !tbaa !56
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !58
}

; Function Attrs: nounwind
declare dso_local void @_ZN11xercesc_2_77XMemorydlEPvPNS_13MemoryManagerE(i8* noundef %0, %"class.xercesc_2_7::MemoryManager"* noundef %1) local_unnamed_addr #7

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEC2ERKS3_(%"class.xercesc_2_7::ValueVectorOf.0"* noundef %this, %"class.xercesc_2_7::ValueVectorOf.0"* noundef nonnull align 8 dereferenceable(32) %toCopy) unnamed_addr #2 comdat align 2 {
entry:
  %fCallDestructor = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 0, !intel-tbaa !59
  %fCallDestructor2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %toCopy, i64 0, i32 0, !intel-tbaa !59
  %0 = load i8, i8* %fCallDestructor2, align 8, !tbaa !59, !range !51
  store i8 %0, i8* %fCallDestructor, align 8, !tbaa !59
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 1, !intel-tbaa !62
  %fCurCount3 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %toCopy, i64 0, i32 1, !intel-tbaa !62
  %1 = load i32, i32* %fCurCount3, align 4, !tbaa !62
  store i32 %1, i32* %fCurCount, align 4, !tbaa !62
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 2, !intel-tbaa !63
  %fMaxCount4 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %toCopy, i64 0, i32 2, !intel-tbaa !63
  %2 = load i32, i32* %fMaxCount4, align 8, !tbaa !63
  store i32 %2, i32* %fMaxCount, align 8, !tbaa !63
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 3, !intel-tbaa !64
  store %"class.xercesc_2_7::DatatypeValidator"** null, %"class.xercesc_2_7::DatatypeValidator"*** %fElemList, align 8, !tbaa !64
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 4, !intel-tbaa !65
  %fMemoryManager5 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %toCopy, i64 0, i32 4, !intel-tbaa !65
  %3 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager5, align 8, !tbaa !65
  store %"class.xercesc_2_7::MemoryManager"* %3, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !65
  %conv = zext i32 %2 to i64
  %mul = shl nuw nsw i64 %conv, 3
  %4 = bitcast %"class.xercesc_2_7::MemoryManager"* %3 to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %4, align 8, !tbaa !37
  %5 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %5, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %6 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %7 = tail call i1 @llvm.type.test(i8* %6, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %7)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %8 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = tail call noundef i8* %8(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %3, i64 noundef %mul)
  %9 = bitcast i8* %call to %"class.xercesc_2_7::DatatypeValidator"**
  store %"class.xercesc_2_7::DatatypeValidator"** %9, %"class.xercesc_2_7::DatatypeValidator"*** %fElemList, align 8, !tbaa !64
  %10 = load i32, i32* %fMaxCount, align 8, !tbaa !63
  %conv11 = zext i32 %10 to i64
  %mul12 = shl nuw nsw i64 %conv11, 3
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul12, i1 false)
  %11 = load i32, i32* %fCurCount, align 4, !tbaa !62
  %cmp27.not = icmp eq i32 %11, 0
  br i1 %cmp27.not, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %whpr.continue
  %fElemList14 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %toCopy, i64 0, i32 3, !intel-tbaa !64
  %12 = load %"class.xercesc_2_7::DatatypeValidator"**, %"class.xercesc_2_7::DatatypeValidator"*** %fElemList14, align 8, !tbaa !64
  %13 = load %"class.xercesc_2_7::DatatypeValidator"**, %"class.xercesc_2_7::DatatypeValidator"*** %fElemList, align 8, !tbaa !64
  %wide.trip.count = zext i32 %11 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %whpr.continue
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds %"class.xercesc_2_7::DatatypeValidator"*, %"class.xercesc_2_7::DatatypeValidator"** %12, i64 %indvars.iv
  %14 = load %"class.xercesc_2_7::DatatypeValidator"*, %"class.xercesc_2_7::DatatypeValidator"** %arrayidx, align 8, !tbaa !66
  %arrayidx17 = getelementptr inbounds %"class.xercesc_2_7::DatatypeValidator"*, %"class.xercesc_2_7::DatatypeValidator"** %13, i64 %indvars.iv
  store %"class.xercesc_2_7::DatatypeValidator"* %14, %"class.xercesc_2_7::DatatypeValidator"** %arrayidx17, align 8, !tbaa !66
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !68
}

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef i32 @_ZNK11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEE11curCapacityEv(%"class.xercesc_2_7::ValueVectorOf"* noundef %this) local_unnamed_addr #4 comdat align 2 {
entry:
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 2, !intel-tbaa !53
  %0 = load i32, i32* %fMaxCount, align 8, !tbaa !53
  ret i32 %0
}

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_716RefArrayVectorOfItEC2EjbPNS_13MemoryManagerE(%"class.xercesc_2_7::RefArrayVectorOf"* noundef %this, i32 noundef %maxElems, i1 noundef zeroext %adoptElems, %"class.xercesc_2_7::MemoryManager"* noundef %manager) unnamed_addr #2 comdat align 2 {
entry:
  %0 = getelementptr %"class.xercesc_2_7::RefArrayVectorOf", %"class.xercesc_2_7::RefArrayVectorOf"* %this, i64 0, i32 0
  tail call void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %0, i32 noundef %maxElems, i1 noundef zeroext %adoptElems, %"class.xercesc_2_7::MemoryManager"* noundef %manager)
  %1 = getelementptr %"class.xercesc_2_7::RefArrayVectorOf", %"class.xercesc_2_7::RefArrayVectorOf"* %this, i64 0, i32 0, i32 0
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_715BaseRefVectorOfItE10addElementEPt(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %this, i16* noundef %toAdd) local_unnamed_addr #8 comdat align 2 {
entry:
  tail call void @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %this, i32 noundef 1)
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 4, !intel-tbaa !69
  %0 = load i16**, i16*** %fElemList, align 8, !tbaa !69
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 2, !intel-tbaa !43
  %1 = load i32, i32* %fCurCount, align 4, !tbaa !43
  %idxprom = zext i32 %1 to i64
  %arrayidx = getelementptr inbounds i16*, i16** %0, i64 %idxprom
  store i16* %toAdd, i16** %arrayidx, align 8, !tbaa !70
  %inc = add i32 %1, 1
  store i32 %inc, i32* %fCurCount, align 4, !tbaa !43
  ret void
}

; Function Attrs: inlinehint mustprogress uwtable
define linkonce_odr dso_local noundef i16* @_ZN11xercesc_2_79XMLString9replicateEPKtPNS_13MemoryManagerE(i16* noundef %toRep, %"class.xercesc_2_7::MemoryManager"* noundef %manager) local_unnamed_addr #9 comdat align 2 {
entry:
  %tobool.not = icmp eq i16* %toRep, null
  br i1 %tobool.not, label %if.end, label %lor.lhs.false.i

lor.lhs.false.i:                                  ; preds = %entry
  %0 = load i16, i16* %toRep, align 2, !tbaa !72
  %cmp1.i = icmp eq i16 %0, 0
  br i1 %cmp1.i, label %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit, label %while.cond.i

while.cond.i:                                     ; preds = %lor.lhs.false.i, %while.cond.i
  %src.pn.i = phi i16* [ %pszTmp.0.i, %while.cond.i ], [ %toRep, %lor.lhs.false.i ]
  %pszTmp.0.i = getelementptr inbounds i16, i16* %src.pn.i, i64 1
  %1 = load i16, i16* %pszTmp.0.i, align 2, !tbaa !72
  %tobool.not.i = icmp eq i16 %1, 0
  br i1 %tobool.not.i, label %while.end.i, label %while.cond.i, !llvm.loop !74

while.end.i:                                      ; preds = %while.cond.i
  %sub.ptr.lhs.cast.i = ptrtoint i16* %pszTmp.0.i to i64
  %sub.ptr.rhs.cast.i = ptrtoint i16* %toRep to i64
  %sub.ptr.sub.i = sub i64 2, %sub.ptr.rhs.cast.i
  %phi.cast = add i64 %sub.ptr.sub.i, %sub.ptr.lhs.cast.i
  %phi.bo11 = and i64 %phi.cast, 8589934590
  br label %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit

_ZN11xercesc_2_79XMLString9stringLenEPKt.exit:    ; preds = %lor.lhs.false.i, %while.end.i
  %retval.0.i = phi i64 [ %phi.bo11, %while.end.i ], [ 2, %lor.lhs.false.i ]
  %2 = bitcast %"class.xercesc_2_7::MemoryManager"* %manager to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %2, align 8, !tbaa !37
  %3 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %3, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit
  %4 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %5 = tail call i1 @llvm.type.test(i8* %4, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %5)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %_ZN11xercesc_2_79XMLString9stringLenEPKt.exit
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %6 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call1 = tail call noundef i8* %6(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %manager, i64 noundef %retval.0.i)
  %7 = bitcast i8* %call1 to i16*
  %8 = bitcast i16* %toRep to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %call1, i8* nonnull align 2 %8, i64 %retval.0.i, i1 false)
  br label %if.end

if.end:                                           ; preds = %whpr.continue, %entry
  %ret.0 = phi i16* [ %7, %whpr.continue ], [ null, %entry ]
  ret i16* %ret.0
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local noundef i16* @_ZN11xercesc_2_715BaseRefVectorOfItE9elementAtEj(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %this, i32 noundef %getAt) local_unnamed_addr #8 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 2, !intel-tbaa !43
  %0 = load i32, i32* %fCurCount, align 4, !tbaa !43
  %cmp.not = icmp ugt i32 %0, %getAt
  br i1 %cmp.not, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %exception = tail call i8* @__cxa_allocate_exception(i64 48) #17
  %1 = bitcast i8* %exception to %"class.xercesc_2_7::ArrayIndexOutOfBoundsException"*
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 5, !intel-tbaa !75
  %2 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !75
  invoke void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(%"class.xercesc_2_7::ArrayIndexOutOfBoundsException"* noundef %1, i8* noundef getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i64 0, i64 0), i32 noundef 249, i32 noundef 116, %"class.xercesc_2_7::MemoryManager"* noundef %2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %if.then
  tail call void @__cxa_throw(i8* %exception, i8* bitcast ({ i8*, i8*, i8* }* @_ZTIN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE to i8*), i8* bitcast (void (%"class.xercesc_2_7::XMLException"*)* @_ZN11xercesc_2_712XMLExceptionD2Ev to i8*)) #29
  unreachable

lpad:                                             ; preds = %if.then
  %3 = landingpad { i8*, i32 }
          cleanup
  tail call void @__cxa_free_exception(i8* %exception) #17
  resume { i8*, i32 } %3

if.end:                                           ; preds = %entry
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 4, !intel-tbaa !69
  %4 = load i16**, i16*** %fElemList, align 8, !tbaa !69
  %idxprom = zext i32 %getAt to i64
  %arrayidx = getelementptr inbounds i16*, i16** %4, i64 %idxprom
  %5 = load i16*, i16** %arrayidx, align 8, !tbaa !70
  ret i16* %5
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1) #3

; Function Attrs: nofree nosync nounwind readnone
declare i32 @llvm.eh.typeid.for(i8* %0) #10

; Function Attrs: nofree
declare dso_local i8* @__cxa_begin_catch(i8* %0) local_unnamed_addr #11

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEE7releaseEv(%"class.xercesc_2_7::JanitorMemFunCall"* noundef %this) local_unnamed_addr #12 comdat align 2 {
entry:
  %fObject = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", %"class.xercesc_2_7::JanitorMemFunCall"* %this, i64 0, i32 0, !intel-tbaa !39
  store %"class.xercesc_2_7::FieldValueMap"* null, %"class.xercesc_2_7::FieldValueMap"** %fObject, align 8, !tbaa !39
  %fToCall = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", %"class.xercesc_2_7::JanitorMemFunCall"* %this, i64 0, i32 1, !intel-tbaa !42
  %fToCall.repack = getelementptr inbounds { i64, i64 }, { i64, i64 }* %fToCall, i64 0, i32 0
  store i64 0, i64* %fToCall.repack, align 8, !tbaa !42
  %fToCall.repack2 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %fToCall, i64 0, i32 1
  store i64 0, i64* %fToCall.repack2, align 8, !tbaa !42
  ret void
}

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #11

; Function Attrs: nofree
declare dso_local void @__cxa_end_catch() local_unnamed_addr #11

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8* %0) local_unnamed_addr #13 comdat {
  %2 = tail call i8* @__cxa_begin_catch(i8* %0) #17
  tail call void @_ZSt9terminatev() #30
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @_ZSt9terminatev() local_unnamed_addr #14

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEED2Ev(%"class.xercesc_2_7::JanitorMemFunCall"* noundef %this) unnamed_addr #15 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %fObject = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", %"class.xercesc_2_7::JanitorMemFunCall"* %this, i64 0, i32 0, !intel-tbaa !39
  %0 = load %"class.xercesc_2_7::FieldValueMap"*, %"class.xercesc_2_7::FieldValueMap"** %fObject, align 8, !tbaa !39
  %cmp.not = icmp eq %"class.xercesc_2_7::FieldValueMap"* %0, null
  br i1 %cmp.not, label %if.end, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %fToCall = getelementptr inbounds %"class.xercesc_2_7::JanitorMemFunCall", %"class.xercesc_2_7::JanitorMemFunCall"* %this, i64 0, i32 1, !intel-tbaa !42
  %.elt = getelementptr inbounds { i64, i64 }, { i64, i64 }* %fToCall, i64 0, i32 0
  %.unpack = load i64, i64* %.elt, align 8, !tbaa !42
  %cmp.ptr.not = icmp eq i64 %.unpack, 0
  br i1 %cmp.ptr.not, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %.elt4 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %fToCall, i64 0, i32 1
  %.unpack5 = load i64, i64* %.elt4, align 8, !tbaa !42
  %1 = bitcast %"class.xercesc_2_7::FieldValueMap"* %0 to i8*
  %2 = getelementptr inbounds i8, i8* %1, i64 %.unpack5
  %this.adjusted = bitcast i8* %2 to %"class.xercesc_2_7::FieldValueMap"*
  %3 = and i64 %.unpack, 1
  %memptr.isvirtual.not = icmp eq i64 %3, 0
  br i1 %memptr.isvirtual.not, label %memptr.nonvirtual, label %memptr.virtual

memptr.virtual:                                   ; preds = %if.then
  %4 = bitcast i8* %2 to i8**
  %vtable = load i8*, i8** %4, align 8, !tbaa !37
  %5 = add i64 %.unpack, -1
  %6 = getelementptr i8, i8* %vtable, i64 %5, !nosanitize !76
  %7 = bitcast i8* %6 to void (%"class.xercesc_2_7::FieldValueMap"*)**, !nosanitize !76
  %memptr.virtualfn = load void (%"class.xercesc_2_7::FieldValueMap"*)*, void (%"class.xercesc_2_7::FieldValueMap"*)** %7, align 8, !nosanitize !76
  br label %memptr.end

memptr.nonvirtual:                                ; preds = %if.then
  %memptr.nonvirtualfn = inttoptr i64 %.unpack to void (%"class.xercesc_2_7::FieldValueMap"*)*
  br label %memptr.end

memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  %8 = phi void (%"class.xercesc_2_7::FieldValueMap"*)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  invoke void %8(%"class.xercesc_2_7::FieldValueMap"* noundef nonnull %this.adjusted)
          to label %if.end unwind label %terminate.lpad

if.end:                                           ; preds = %memptr.end, %land.lhs.true, %entry
  ret void

terminate.lpad:                                   ; preds = %memptr.end
  %9 = landingpad { i8*, i32 }
          catch i8* null
  %10 = extractvalue { i8*, i32 } %9, 0
  tail call void @__clang_call_terminate(i8* %10) #30
  unreachable
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEED2Ev(%"class.xercesc_2_7::ValueVectorOf"* noundef %this) unnamed_addr #16 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 4, !intel-tbaa !55
  %0 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !55
  %fElemList2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf", %"class.xercesc_2_7::ValueVectorOf"* %this, i64 0, i32 3, !intel-tbaa !54
  %1 = bitcast %"class.xercesc_2_7::IC_Field"*** %fElemList2 to i8**
  %2 = load i8*, i8** %1, align 8, !tbaa !54
  %3 = bitcast %"class.xercesc_2_7::MemoryManager"* %0 to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %3, align 8, !tbaa !37
  %4 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %5 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable to i8*
  %6 = tail call i1 @llvm.type.test(i8* %5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable, i64 3
  %7 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn, align 8
  invoke void %7(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %0, i8* noundef %2)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %whpr.continue
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %8 = landingpad { i8*, i32 }
          catch i8* null
  %9 = extractvalue { i8*, i32 } %8, 0
  tail call void @__clang_call_terminate(i8* %9) #30
  unreachable
}

; Function Attrs: nounwind
declare dso_local void @_ZN11xercesc_2_77XMemorydlEPv(i8* noundef %0) local_unnamed_addr #7

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEED2Ev(%"class.xercesc_2_7::ValueVectorOf.0"* noundef %this) unnamed_addr #16 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 4, !intel-tbaa !65
  %0 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !65
  %fElemList2 = getelementptr inbounds %"class.xercesc_2_7::ValueVectorOf.0", %"class.xercesc_2_7::ValueVectorOf.0"* %this, i64 0, i32 3, !intel-tbaa !64
  %1 = bitcast %"class.xercesc_2_7::DatatypeValidator"*** %fElemList2 to i8**
  %2 = load i8*, i8** %1, align 8, !tbaa !64
  %3 = bitcast %"class.xercesc_2_7::MemoryManager"* %0 to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %3, align 8, !tbaa !37
  %4 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %5 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable to i8*
  %6 = tail call i1 @llvm.type.test(i8* %5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable, i64 3
  %7 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn, align 8
  invoke void %7(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %0, i8* noundef %2)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %whpr.continue
  ret void

terminate.lpad:                                   ; preds = %whpr.continue
  %8 = landingpad { i8*, i32 }
          catch i8* null
  %9 = extractvalue { i8*, i32 } %8, 0
  tail call void @__clang_call_terminate(i8* %9) #30
  unreachable
}

; Function Attrs: nounwind
declare i1 @llvm.intel.wholeprogramsafe() #17

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(i8* %0, metadata %1) #18

; Function Attrs: inaccessiblememonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef %0) #19

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #20

; Function Attrs: nofree
declare dso_local noalias i8* @__cxa_allocate_exception(i64 %0) local_unnamed_addr #11

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_730ArrayIndexOutOfBoundsExceptionC2EPKcjNS_10XMLExcepts5CodesEPNS_13MemoryManagerE(%"class.xercesc_2_7::ArrayIndexOutOfBoundsException"* noundef %this, i8* noundef %srcFile, i32 noundef %srcLine, i32 noundef %toThrow, %"class.xercesc_2_7::MemoryManager"* noundef %memoryManager) unnamed_addr #21 comdat align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %0 = getelementptr %"class.xercesc_2_7::ArrayIndexOutOfBoundsException", %"class.xercesc_2_7::ArrayIndexOutOfBoundsException"* %this, i64 0, i32 0
  tail call void @_ZN11xercesc_2_712XMLExceptionC2EPKcjPNS_13MemoryManagerE(%"class.xercesc_2_7::XMLException"* noundef %0, i8* noundef %srcFile, i32 noundef %srcLine, %"class.xercesc_2_7::MemoryManager"* noundef %memoryManager)
  %1 = getelementptr %"class.xercesc_2_7::ArrayIndexOutOfBoundsException", %"class.xercesc_2_7::ArrayIndexOutOfBoundsException"* %this, i64 0, i32 0, i32 0
  invoke void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(%"class.xercesc_2_7::XMLException"* noundef %0, i32 noundef %toThrow)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret void

lpad:                                             ; preds = %entry
  %2 = landingpad { i8*, i32 }
          cleanup
  tail call void @_ZN11xercesc_2_712XMLExceptionD2Ev(%"class.xercesc_2_7::XMLException"* noundef %0) #17
  resume { i8*, i32 } %2
}

; Function Attrs: nofree
declare dso_local void @__cxa_free_exception(i8* %0) local_unnamed_addr #11

; Function Attrs: nounwind
declare dso_local void @_ZN11xercesc_2_712XMLExceptionD2Ev(%"class.xercesc_2_7::XMLException"* noundef %0) unnamed_addr #22

; Function Attrs: nofree noreturn
declare dso_local void @__cxa_throw(i8* %0, i8* %1, i8* %2) local_unnamed_addr #23

declare dso_local void @_ZN11xercesc_2_712XMLExceptionC2EPKcjPNS_13MemoryManagerE(%"class.xercesc_2_7::XMLException"* noundef %0, i8* noundef %1, i32 noundef %2, %"class.xercesc_2_7::MemoryManager"* noundef %3) unnamed_addr #24

declare dso_local void @_ZN11xercesc_2_712XMLException14loadExceptTextENS_10XMLExcepts5CodesE(%"class.xercesc_2_7::XMLException"* noundef %0, i32 noundef %1) local_unnamed_addr #6

declare dso_local void @_ZN11xercesc_2_712XMLExceptionC2ERKS0_(%"class.xercesc_2_7::XMLException"* noundef %0, %"class.xercesc_2_7::XMLException"* noundef nonnull align 8 dereferenceable(48) %1) unnamed_addr #24

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"class.xercesc_2_7::IC_Field"** @"llvm.intel.fakeload.p0p0s_class.xercesc_2_7::IC_Fields"(%"class.xercesc_2_7::IC_Field"** %0, metadata %1) #26

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_715BaseRefVectorOfItEC2EjbPNS_13MemoryManagerE(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %this, i32 noundef %maxElems, i1 noundef zeroext %adoptElems, %"class.xercesc_2_7::MemoryManager"* noundef %manager) unnamed_addr #2 comdat align 2 {
entry:
  %frombool = zext i1 %adoptElems to i8
  %0 = getelementptr %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 0
  %fAdoptedElems = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 1, !intel-tbaa !82
  store i8 %frombool, i8* %fAdoptedElems, align 8, !tbaa !82
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 2, !intel-tbaa !43
  store i32 0, i32* %fCurCount, align 4, !tbaa !43
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 3, !intel-tbaa !83
  store i32 %maxElems, i32* %fMaxCount, align 8, !tbaa !83
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 4, !intel-tbaa !69
  store i16** null, i16*** %fElemList, align 8, !tbaa !69
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 5, !intel-tbaa !75
  store %"class.xercesc_2_7::MemoryManager"* %manager, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !75
  %conv = zext i32 %maxElems to i64
  %mul = shl nuw nsw i64 %conv, 3
  %1 = bitcast %"class.xercesc_2_7::MemoryManager"* %manager to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %1, align 8, !tbaa !37
  %2 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %2, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %entry
  %3 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %4 = tail call i1 @llvm.type.test(i8* %3, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %4)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %entry
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %5 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = tail call noundef i8* %5(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %manager, i64 noundef %mul)
  %6 = bitcast i8* %call to i16**
  store i16** %6, i16*** %fElemList, align 8, !tbaa !69
  %cmp11.not = icmp eq i32 %maxElems, 0
  br i1 %cmp11.not, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %whpr.continue
  call void @llvm.memset.p0i8.i64(i8* align 8 %call, i8 0, i64 %mul, i1 false)
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body.preheader, %whpr.continue
  ret void
}

; Function Attrs: mustprogress uwtable
define linkonce_odr dso_local void @_ZN11xercesc_2_715BaseRefVectorOfItE19ensureExtraCapacityEj(%"class.xercesc_2_7::BaseRefVectorOf"* noundef %this, i32 noundef %length) local_unnamed_addr #8 comdat align 2 {
entry:
  %fCurCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 2, !intel-tbaa !43
  %0 = load i32, i32* %fCurCount, align 4, !tbaa !43
  %add = add i32 %0, %length
  %fMaxCount = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 3, !intel-tbaa !83
  %1 = load i32, i32* %fMaxCount, align 8, !tbaa !83
  %cmp.not = icmp ugt i32 %add, %1
  br i1 %cmp.not, label %if.end, label %cleanup

if.end:                                           ; preds = %entry
  %div = lshr i32 %1, 1
  %add4 = add i32 %div, %1
  %cmp5 = icmp ult i32 %add, %add4
  %spec.select = select i1 %cmp5, i32 %add4, i32 %add
  %fMemoryManager = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 5, !intel-tbaa !75
  %2 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !75
  %conv = zext i32 %spec.select to i64
  %mul = shl nuw nsw i64 %conv, 3
  %3 = bitcast %"class.xercesc_2_7::MemoryManager"* %2 to i8* (%"class.xercesc_2_7::MemoryManager"*, i64)***
  %vtable = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)**, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*** %3, align 8, !tbaa !37
  %4 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %4, label %whpr.wrap, label %whpr.continue

whpr.wrap:                                        ; preds = %if.end
  %5 = bitcast i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable to i8*
  %6 = tail call i1 @llvm.type.test(i8* %5, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %6)
  br label %whpr.continue

whpr.continue:                                    ; preds = %whpr.wrap, %if.end
  %vfn = getelementptr inbounds i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vtable, i64 2
  %7 = load i8* (%"class.xercesc_2_7::MemoryManager"*, i64)*, i8* (%"class.xercesc_2_7::MemoryManager"*, i64)** %vfn, align 8
  %call = tail call noundef i8* %7(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %2, i64 noundef %mul)
  %8 = bitcast i8* %call to i16**
  %9 = load i32, i32* %fCurCount, align 4, !tbaa !43
  %cmp1347.not = icmp eq i32 %9, 0
  br i1 %cmp1347.not, label %for.cond16.preheader, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %whpr.continue
  %fElemList = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 4, !intel-tbaa !69
  %10 = load i16**, i16*** %fElemList, align 8, !tbaa !69
  %wide.trip.count = zext i32 %9 to i64
  br label %for.body

for.cond16.preheader:                             ; preds = %for.body, %whpr.continue
  %cmp1749 = icmp ult i32 %9, %spec.select
  br i1 %cmp1749, label %for.body18.preheader, label %for.end23

for.body18.preheader:                             ; preds = %for.cond16.preheader
  %11 = zext i32 %9 to i64
  %12 = shl nuw nsw i64 %11, 3
  %scevgep = getelementptr i8, i8* %call, i64 %12
  %13 = xor i32 %9, -1
  %14 = add i32 %spec.select, %13
  %15 = zext i32 %14 to i64
  %16 = shl nuw nsw i64 %15, 3
  %17 = add nuw nsw i64 %16, 8
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(1) %scevgep, i8 0, i64 %17, i1 false)
  br label %for.end23

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i16*, i16** %10, i64 %indvars.iv
  %18 = load i16*, i16** %arrayidx, align 8, !tbaa !70
  %arrayidx15 = getelementptr inbounds i16*, i16** %8, i64 %indvars.iv
  store i16* %18, i16** %arrayidx15, align 8, !tbaa !70
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond16.preheader, label %for.body, !llvm.loop !92

for.end23:                                        ; preds = %for.body18.preheader, %for.cond16.preheader
  %19 = load %"class.xercesc_2_7::MemoryManager"*, %"class.xercesc_2_7::MemoryManager"** %fMemoryManager, align 8, !tbaa !75
  %fElemList25 = getelementptr inbounds %"class.xercesc_2_7::BaseRefVectorOf", %"class.xercesc_2_7::BaseRefVectorOf"* %this, i64 0, i32 4, !intel-tbaa !69
  %20 = bitcast i16*** %fElemList25 to i8**
  %21 = load i8*, i8** %20, align 8, !tbaa !69
  %22 = bitcast %"class.xercesc_2_7::MemoryManager"* %19 to void (%"class.xercesc_2_7::MemoryManager"*, i8*)***
  %vtable26 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)**, void (%"class.xercesc_2_7::MemoryManager"*, i8*)*** %22, align 8, !tbaa !37
  %23 = tail call i1 @llvm.intel.wholeprogramsafe()
  br i1 %23, label %whpr.wrap27, label %whpr.continue28

whpr.wrap27:                                      ; preds = %for.end23
  %24 = bitcast void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable26 to i8*
  %25 = tail call i1 @llvm.type.test(i8* %24, metadata !"_ZTSN11xercesc_2_713MemoryManagerE")
  tail call void @llvm.assume(i1 %25)
  br label %whpr.continue28

whpr.continue28:                                  ; preds = %whpr.wrap27, %for.end23
  %vfn29 = getelementptr inbounds void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vtable26, i64 3
  %26 = load void (%"class.xercesc_2_7::MemoryManager"*, i8*)*, void (%"class.xercesc_2_7::MemoryManager"*, i8*)** %vfn29, align 8
  tail call void %26(%"class.xercesc_2_7::MemoryManager"* noundef nonnull %19, i8* noundef %21)
  store i16** %8, i16*** %fElemList25, align 8, !tbaa !69
  store i32 %spec.select, i32* %fMaxCount, align 8, !tbaa !83
  br label %cleanup

cleanup:                                          ; preds = %entry, %whpr.continue28
  ret void
}

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #27

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.umax.i32(i32 %0, i32 %1) #28

attributes #0 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind uwtable willreturn writeonly "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #4 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #8 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #9 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { nofree nosync nounwind readnone }
attributes #11 = { nofree }
attributes #12 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #13 = { noinline noreturn nounwind "pre_loopopt" }
attributes #14 = { nofree noreturn nounwind }
attributes #15 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #16 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "noinline-dtrans" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #17 = { nounwind }
attributes #18 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }
attributes #19 = { inaccessiblememonly mustprogress nofree nosync nounwind willreturn }
attributes #20 = { argmemonly mustprogress nofree nounwind willreturn }
attributes #21 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #22 = { nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-destructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #23 = { nofree noreturn }
attributes #24 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "intel-mempool-constructor" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #25 = { uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #26 = { nofree nosync nounwind readnone speculatable }
attributes #27 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #28 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #29 = { noreturn }
attributes #30 = { noreturn nounwind }
attributes #31 = { builtin nounwind }

!llvm.module.flags = !{!18, !19, !20, !21, !22}
!llvm.ident = !{!23}

!0 = !{i64 16, !"_ZTSN11xercesc_2_712XMLExceptionE"}
!1 = !{i64 32, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPKtvE.virtual"}
!2 = !{i64 40, !"_ZTSMN11xercesc_2_712XMLExceptionEKFPS0_vE.virtual"}
!3 = !{i64 16, !"_ZTSN11xercesc_2_730ArrayIndexOutOfBoundsExceptionE"}
!4 = !{i64 32, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPKtvE.virtual"}
!5 = !{i64 40, !"_ZTSMN11xercesc_2_730ArrayIndexOutOfBoundsExceptionEKFPNS_12XMLExceptionEvE.virtual"}
!6 = !{i64 16, !"_ZTSN11xercesc_2_715BaseRefVectorOfItEE"}
!7 = !{i64 32, !"_ZTSMN11xercesc_2_715BaseRefVectorOfItEEFvPtjE.virtual"}
!8 = !{i64 40, !"_ZTSMN11xercesc_2_715BaseRefVectorOfItEEFvvE.virtual"}
!9 = !{i64 48, !"_ZTSMN11xercesc_2_715BaseRefVectorOfItEEFvjE.virtual"}
!10 = !{i64 56, !"_ZTSMN11xercesc_2_715BaseRefVectorOfItEEFvvE.virtual"}
!11 = !{i64 64, !"_ZTSMN11xercesc_2_715BaseRefVectorOfItEEFvvE.virtual"}
!12 = !{i64 16, !"_ZTSN11xercesc_2_716RefArrayVectorOfItEE"}
!13 = !{i64 32, !"_ZTSMN11xercesc_2_716RefArrayVectorOfItEEFvPtjE.virtual"}
!14 = !{i64 40, !"_ZTSMN11xercesc_2_716RefArrayVectorOfItEEFvvE.virtual"}
!15 = !{i64 48, !"_ZTSMN11xercesc_2_716RefArrayVectorOfItEEFvjE.virtual"}
!16 = !{i64 56, !"_ZTSMN11xercesc_2_716RefArrayVectorOfItEEFvvE.virtual"}
!17 = !{i64 64, !"_ZTSMN11xercesc_2_716RefArrayVectorOfItEEFvvE.virtual"}
!18 = !{i32 1, !"wchar_size", i32 4}
!19 = !{i32 1, !"Virtual Function Elim", i32 0}
!20 = !{i32 7, !"uwtable", i32 1}
!21 = !{i32 1, !"ThinLTO", i32 0}
!22 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!23 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!24 = !{!25, !26, i64 0}
!25 = !{!"struct@_ZTSN11xercesc_2_713FieldValueMapE", !26, i64 0, !29, i64 8, !30, i64 16, !31, i64 24}
!26 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !27, i64 0}
!27 = !{!"omnipotent char", !28, i64 0}
!28 = !{!"Simple C++ TBAA"}
!29 = !{!"pointer@_ZTSPN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !27, i64 0}
!30 = !{!"pointer@_ZTSPN11xercesc_2_716RefArrayVectorOfItEE", !27, i64 0}
!31 = !{!"pointer@_ZTSPN11xercesc_2_713MemoryManagerE", !27, i64 0}
!32 = !{!25, !29, i64 8}
!33 = !{!25, !30, i64 16}
!34 = !{!25, !31, i64 24}
!35 = distinct !{!35, !36}
!36 = !{!"llvm.loop.mustprogress"}
!37 = !{!38, !38, i64 0}
!38 = !{!"vtable pointer", !28, i64 0}
!39 = !{!40, !41, i64 0}
!40 = !{!"struct@_ZTSN11xercesc_2_717JanitorMemFunCallINS_13FieldValueMapEEE", !41, i64 0, !27, i64 8}
!41 = !{!"pointer@_ZTSPN11xercesc_2_713FieldValueMapE", !27, i64 0}
!42 = !{!40, !27, i64 8}
!43 = !{!44, !46, i64 12}
!44 = !{!"struct@_ZTSN11xercesc_2_715BaseRefVectorOfItEE", !45, i64 8, !46, i64 12, !46, i64 16, !47, i64 24, !31, i64 32}
!45 = !{!"bool", !27, i64 0}
!46 = !{!"int", !27, i64 0}
!47 = !{!"pointer@_ZTSPPt", !27, i64 0}
!48 = !{!49, !45, i64 0}
!49 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_8IC_FieldEEE", !45, i64 0, !46, i64 4, !46, i64 8, !50, i64 16, !31, i64 24}
!50 = !{!"pointer@_ZTSPPN11xercesc_2_78IC_FieldE", !27, i64 0}
!51 = !{i8 0, i8 2}
!52 = !{!49, !46, i64 4}
!53 = !{!49, !46, i64 8}
!54 = !{!49, !50, i64 16}
!55 = !{!49, !31, i64 24}
!56 = !{!57, !57, i64 0}
!57 = !{!"pointer@_ZTSPN11xercesc_2_78IC_FieldE", !27, i64 0}
!58 = distinct !{!58, !36}
!59 = !{!60, !45, i64 0}
!60 = !{!"struct@_ZTSN11xercesc_2_713ValueVectorOfIPNS_17DatatypeValidatorEEE", !45, i64 0, !46, i64 4, !46, i64 8, !61, i64 16, !31, i64 24}
!61 = !{!"pointer@_ZTSPPN11xercesc_2_717DatatypeValidatorE", !27, i64 0}
!62 = !{!60, !46, i64 4}
!63 = !{!60, !46, i64 8}
!64 = !{!60, !61, i64 16}
!65 = !{!60, !31, i64 24}
!66 = !{!67, !67, i64 0}
!67 = !{!"pointer@_ZTSPN11xercesc_2_717DatatypeValidatorE", !27, i64 0}
!68 = distinct !{!68, !36}
!69 = !{!44, !47, i64 24}
!70 = !{!71, !71, i64 0}
!71 = !{!"pointer@_ZTSPt", !27, i64 0}
!72 = !{!73, !73, i64 0}
!73 = !{!"short", !27, i64 0}
!74 = distinct !{!74, !36}
!75 = !{!44, !31, i64 32}
!76 = !{}
!77 = distinct !{!77, !36}
!78 = !{!79, !31, i64 40}
!79 = !{!"struct@_ZTSN11xercesc_2_712XMLExceptionE", !80, i64 8, !81, i64 16, !46, i64 24, !71, i64 32, !31, i64 40}
!80 = !{!"_ZTSN11xercesc_2_710XMLExcepts5CodesE", !27, i64 0}
!81 = !{!"pointer@_ZTSPc", !27, i64 0}
!82 = !{!44, !45, i64 8}
!83 = !{!44, !46, i64 16}
!84 = distinct !{!84, !36}
!85 = distinct !{!85, !36, !86}
!86 = !{!"llvm.loop.unswitch.partial.disable"}
!87 = distinct !{!87, !36}
!88 = distinct !{!88, !36}
!89 = distinct !{!89, !36, !86}
!90 = distinct !{!90, !36}
!91 = distinct !{!91, !36}
!92 = distinct !{!92, !36}
; end INTEL_FEATURE_SW_ADVANCED
