; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that add instruction is replaced by GEP.

%network = type { [200 x i8], [200 x i8], i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64 }
%node = type { i64, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i32, i32 }
%arc = type { i64, ptr, ptr, i32, ptr, ptr, i64, i64 }

@net = internal global %network zeroinitializer, align 8

; CHECK-LABEL: @alloc_minus_a_add_b
; CHECK: %off1 = sub i64 %b, %a
; CHECK: %idx = sdiv i64 %off1, 64
; CHECK: %ptr = getelementptr %arc, ptr %alloc_ptr, i64 %idx
; CHECK: ret ptr %ptr
define internal ptr @alloc_minus_a_add_b() #0 {
  %net_nodes = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 12)
  %net_arcs = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 14)
  %alloc_ptr = tail call ptr @realloc(ptr noundef %net_arcs, i64 1024)
  %alloc = ptrtoint ptr %alloc_ptr to i64
  %a_ptr = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 14)
  %a = ptrtoint ptr %a_ptr to i64
  %off = sub i64 %alloc, %a
  %b_addr = getelementptr inbounds %node, ptr %net_nodes, i64 0, i32 6
  %b_ptr = load ptr, ptr %b_addr
  %b = ptrtoint ptr %b_ptr to i64
  %d = add i64 %off, %b
  %ret = inttoptr i64 %d to ptr
  ret ptr %ret
}

; CHECK-LABEL: @alloc_minus_b_add_a
; CHECK: %off1 = sub i64 %a, %b
; CHECK: %idx = sdiv i64 %off1, 64
; CHECK: %ptr = getelementptr %arc, ptr %alloc_ptr, i64 %idx
; CHECK: ret ptr %ptr
define internal ptr @alloc_minus_b_add_a() #0 {
  %net_nodes = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 12)
  %net_arcs = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 14)
  %alloc_ptr = tail call ptr @realloc(ptr noundef %net_arcs, i64 1024)
  %alloc = ptrtoint ptr %alloc_ptr to i64
  %b_addr = getelementptr inbounds %node, ptr %net_nodes, i64 0, i32 6
  %b_ptr = load ptr, ptr %b_addr
  %b = ptrtoint ptr %b_ptr to i64
  %a_ptr = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 14)
  %a = ptrtoint ptr %a_ptr to i64
  %off = sub i64 %alloc, %b
  %d = add i64 %off, %a
  %ret = inttoptr i64 %d to ptr
  ret ptr %ret
}

; CHECK-LABEL: @b_minus_a_add_alloc
; CHECK: %off1 = sub i64 %b, %a
; CHECK: %idx = sdiv i64 %off1, 64
; CHECK: %ptr = getelementptr %arc, ptr %alloc_ptr, i64 %idx
; CHECK: ret ptr %ptr
define internal ptr @b_minus_a_add_alloc() #0 {
  %net_nodes = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 12)
  %net_arcs = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 14)
  %alloc_ptr = tail call ptr @realloc(ptr noundef %net_arcs, i64 1024)
  %alloc = ptrtoint ptr %alloc_ptr to i64
  %b_addr = getelementptr inbounds %node, ptr %net_nodes, i64 0, i32 6
  %b_ptr = load ptr, ptr %b_addr
  %b = ptrtoint ptr %b_ptr to i64
  %a_ptr = load ptr, ptr getelementptr inbounds (%network, ptr @net, i64 0, i32 14)
  %a = ptrtoint ptr %a_ptr to i64
  %off = sub i64 %b, %a
  %d = add i64 %off, %alloc
  %ret = inttoptr i64 %d to ptr
  ret ptr %ret
}

attributes #0 = { nofree nounwind }
attributes #1 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" }

!0 = !{!"S", %network zeroinitializer, i32 20, !1, !1, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !4, !4, !5, !5, !5, !5, !3, !3}
!1 = !{!"A", i32 200, !2}
!2 = !{i8 0, i32 0}
!3 = !{i64 0, i32 0}
!4 = !{%node zeroinitializer, i32 1}
!5 = !{%arc zeroinitializer, i32 1}
!6 = !{!"S", %node zeroinitializer, i32 14, !3, !7, !4, !4, !4, !4, !5, !5, !5, !5, !3, !3, !7, !7}
!7 = !{i32 0, i32 0}
!8 = !{!"S", %arc zeroinitializer, i32 8, !3, !4, !4, !7, !5, !5, !3, !3}

!intel.dtrans.types = !{!0, !6, !8}

declare dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @realloc(ptr allocptr nocapture noundef "intel_dtrans_func_index"="2" %0, i64 noundef %1) local_unnamed_addr #1
