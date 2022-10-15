; This test try to find the ptr and ptrofptr's global variables and validate isValidPtr function.
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed < %s -dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed < %s -passes=dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s

; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 3:3 4:3 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, %struct.test* %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, i64* %0, align 8
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i64, i64 }
%struct.ptr = type { i32, i64, i32, %struct.test*, %struct.test* }
%struct.ptr2ptr = type { i32, %struct.test** }

@__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"
@0 = private constant [1 x i8] zeroinitializer

@net = internal global %struct.ptr zeroinitializer, align 8
@node = internal global %struct.ptr2ptr zeroinitializer, align 8

define void @foo_0(%struct.test* %tp) {
entry:
  %i = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 0
  %0 = load i32, i32* %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  store i64 %conv, i64* %f, align 8
  %o = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  store i64 %conv, i64* %o, align 8
  ret void
}

define i64 @cal_0(%struct.test* %tp) {
entry:
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  %a = load i64, i64* %f, align 8
  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  %b = load i64, i64* %g, align 8
  %h = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
  %d = load i64, i64* %g, align 8
  %e = add i64 %a, %b
  %ret = add i64 %d, %e
  ret i64 %ret
}

define void @init_net_and_node(%struct.ptr* %net0, i64 %idx) {
entry:
  %ptr = load %struct.test*, %struct.test** getelementptr inbounds  (%struct.ptr, %struct.ptr* @net, i64 0, i32 3), align 8
  %ptridx = getelementptr inbounds %struct.test, %struct.test* %ptr, i64 %idx
  %basic = getelementptr %struct.ptr2ptr, %struct.ptr2ptr* @node, i64 0, i32 1
  %ptrofptr = load %struct.test**, %struct.test*** %basic, align 8
  %ptrofptridx = getelementptr %struct.test*, %struct.test** %ptrofptr, i64 %idx
  store %struct.test* %ptridx, %struct.test** %ptrofptridx, align 8
  %f = getelementptr inbounds %struct.test, %struct.test* %ptridx, i64 0, i32 3
  store i64 %idx, i64* %f, align 8

  ret void
}

define void @ptr_0(i64 %idx) {
entry:
  %ptr = load %struct.test*, %struct.test** getelementptr inbounds  (%struct.ptr, %struct.ptr* @net, i64 0, i32 3), align 8
  %ptridx = getelementptr inbounds %struct.test, %struct.test* %ptr, i64 %idx
  %cmp = icmp uge %struct.test* %ptr, %ptr
  %bc = bitcast %struct.test* %ptr to i8*
  tail call void @free(i8* noundef %bc)
  %f = getelementptr inbounds %struct.test, %struct.test* %ptridx, i64 0, i32 0
  %g = call i32* @llvm.ptr.annotation.p0i32(i32* %f, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0), i32 0, i8* null)
  %ptridx0 = getelementptr inbounds %struct.test, %struct.test* %ptr, i64 0, i32 0
  store i32 99, i32* %ptridx0, align 8
  ret void
}

define i64 @cal_1(%struct.test* %tp) {
entry:
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
  %a = load i64, i64* %f, align 8
  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  %b = load i64, i64* %g, align 8
  %c = sub i64 %a, %b
  ret i64 %c
}

define i64 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 40)
  %0 = bitcast i8* %call to %struct.test*
  %i = getelementptr %struct.test, %struct.test* %0, i64 0, i32 0
  store i32 10, i32* %i, align 8
  tail call void @foo_0(%struct.test* %0)
  %res_0 = tail call i64 @cal_0(%struct.test* %0)
  tail call void @init_net_and_node(%struct.ptr* @net, i64 101)
  %res_1 = tail call i64 @cal_1(%struct.test* %0)
  tail call void @ptr_0(i64 101)
  %res = add i64 %res_0, %res_1
  ret i64 %res
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64) #0

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare i32* @llvm.ptr.annotation.p0i32(i32* %0, i8* %1, i8* %2, i32 %3, i8* %4)

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare dso_local void @free(i8* nocapture noundef %0) local_unnamed_addr #1

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }
