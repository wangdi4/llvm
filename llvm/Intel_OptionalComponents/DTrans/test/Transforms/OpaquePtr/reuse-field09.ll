; This test try to find the ptr and ptrofptr's global variables and validate isValidPtr function.
; This test also verify the global variable is OK as the printf/fopen/strcpy/memset declaration's argument in isValidPtr function.
; REQUIRES: asserts
; RUN: opt -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed < %s -passes=dtrans-reusefieldop -debug-only=dtrans-reusefieldop -disable-output 2>&1 | FileCheck %s

; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 3:3 4:3 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, ptr %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, ptr %0, align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i64, i64 }
%struct.ptr = type { [200 x i8], i32, i64, i32, ptr, ptr }
%struct.ptr2ptr = type { i32, ptr }
%"struct..?AU_iobuf@@._iobuf" = type { ptr }

@net = internal global %struct.ptr zeroinitializer, align 8
@node = internal global %struct.ptr2ptr zeroinitializer, align 8

@.str.0 = private unnamed_addr constant [31 x i8] c"network %s: not enough memory\0A\00", align 1
@.str.1 = internal unnamed_addr constant [2 x i8] c"r\00", align 1

define void @foo(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %i = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 0
  %0 = load i32, ptr %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  store i64 %conv, ptr %f, align 8
  %o = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  store i64 %conv, ptr %o, align 8
  ret void
}

define void @init_net_and_node(%struct.ptr* "intel_dtrans_func_index"="1" %net0, i64 %idx) !intel.dtrans.func.type !15 {
entry:
  %ptr = load %struct.test*, ptr getelementptr inbounds  (%struct.ptr, ptr @net, i64 0, i32 4), align 8
  %ptridx = getelementptr inbounds %struct.test, ptr %ptr, i64 %idx
  %basic = getelementptr %struct.ptr2ptr, %struct.ptr2ptr* @node, i64 0, i32 1
  %ptrofptr = load %struct.test**, ptr %basic, align 8
  %ptrofptridx = getelementptr %struct.test*, ptr %ptrofptr, i64 %idx
  store %struct.test* %ptridx, ptr %ptrofptridx, align 8
  %f = getelementptr inbounds %struct.test, ptr %ptridx, i64 0, i32 3
  store i64 %idx, ptr %f, align 8
  %print = tail call i32 (ptr, ...) @printf(ptr noundef nonnull @.str.0, ptr noundef @net)
  %fileopen = tail call ptr @fopen(ptr noundef @net, ptr noundef nonnull @.str.1)
  %str = tail call ptr @strcpy(ptr noundef nonnull @net, ptr noundef nonnull dereferenceable(1) @.str.1)
  ret void
}

define i64 @cal_0(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  %a = load i64, ptr %f, align 8
  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  %b = load i64, ptr %g, align 8
  %c = sub i64 %a, %b
  ret i64 %c
}

define i64 @cal_1(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  %a = load i64, ptr %f, align 8
  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  %b = load i64, ptr %g, align 8
  %c = sub i64 %a, %b
  ret i64 %c
}

define i64 @main() {
entry:
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(240) @net, i8 0, i64 240, i1 false)
  %call = tail call noalias i8* @calloc(i64 10, i64 40)
  store i32 10, ptr %call, align 8
  tail call void @foo(ptr %call)
  tail call void @init_net_and_node(%struct.ptr* @net, i64 101)
  %res0 = tail call i64 @cal_0(ptr %call)
  %res1 = tail call i64 @cal_1(ptr %call)
  %res = add i64 %res0, %res1
  ret i64 %res
}

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare ptr @llvm.ptr.annotation.p0(ptr %0, ptr %1, ptr %2, i32 %3, ptr %4)

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare !intel.dtrans.func.type !9 dso_local void @free(ptr "intel_dtrans_func_index"="1") #1

; Function Attrs: nounwind
declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !9 dso_local noundef i32 @printf(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, ...) local_unnamed_addr

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !21 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @fopen(ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %0, ptr nocapture noundef readonly "intel_dtrans_func_index"="3" %1)

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !22 dso_local "intel_dtrans_func_index"="1" ptr @strcpy(ptr noalias noundef returned writeonly "intel_dtrans_func_index"="2" %0, ptr noalias nocapture noundef readonly "intel_dtrans_func_index"="3" %1)

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{%struct.test zeroinitializer, i32 2}  ; %struct.test**
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 5, !1, !2, !1, !2, !2} ; { i32, i64, i32, i64, i64 }
!12 = !{!"S", %struct.ptr zeroinitializer, i32 6, !16, !1, !2, !1, !5, !5} ; { [200 x i8], i32, i64, i32, %struct.test*, %struct.test* }
!13 = !{!"S", %struct.ptr2ptr zeroinitializer, i32 2, !1, !7} ; { i32, %struct.test** }
!14 = !{%struct.ptr zeroinitializer, i32 1}  ; %struct.test*
!15 = distinct !{!14}
!16 = !{!"A", i32 200, !17}
!17 = !{i8 0, i32 0}
!18 = !{!"S", %"struct..?AU_iobuf@@._iobuf" zeroinitializer, i32 1, !19}
!19 = !{i8 0, i32 1}
!20 = !{%"struct..?AU_iobuf@@._iobuf" zeroinitializer, i32 1}
!21 = distinct !{!20, !19, !19}
!22 = distinct !{!8, !8, !8}
!intel.dtrans.types = !{!11, !12, !13, !18 }
