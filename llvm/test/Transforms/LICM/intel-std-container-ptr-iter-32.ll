; RUN: opt < %s -bugpoint-enable-legacy-pm -tbaa   -std-container-alias  -basiccg -domtree -basic-aa -aa -std-container-opt -domtree -loops  -loop-rotate -licm  -S | FileCheck %s
; RUN: opt < %s -passes="std-container-opt,loop-rotate,loop-mssa(licm)" -S | FileCheck %s
;
; The compiler is expected to hoist out the load *ita when the SROA
; generates ptrtoint and inttoptr for the container_ptr_iter.
;
; #include <vector>
; std::vector<int> myvector;
; std::vector<int> myvector2;
; void foo ()
; {
;     std::vector<int>::iterator ita, it;
;       for (ita = myvector2.begin() ; ita != myvector2.end(); ++ita)
;         for (it = myvector.begin() ; it != myvector.end(); ++it)
;            *it = *it + *ita *(*it);
; }


target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" = type { ptr, ptr, ptr }


@myvector = global %"class.std::vector" zeroinitializer, align 4
@myvector2 = global %"class.std::vector" zeroinitializer, align 4

; Function Attrs: nounwind uwtable
define void @_Z3foov() local_unnamed_addr #3 {
entry:
  %0 = load ptr, ptr @myvector2, align 4, !tbaa !7, !noalias !8
  %1 = call ptr @llvm.intel.std.container.ptr.iter.p0(ptr %0) #1, !noalias !8
  %2 = ptrtoint ptr %1 to i32
  br label %for.cond

for.cond:                                         ; preds = %for.inc12, %entry
  %ita.sroa.0.0 = phi i32 [ %2, %entry ], [ %19, %for.inc12 ]
  %3 = load ptr, ptr getelementptr inbounds (%"class.std::vector", ptr @myvector2, i32 0, i32 0, i32 0, i32 1), align 4, !tbaa !7, !noalias !11
  %4 = call ptr @llvm.intel.std.container.ptr.iter.p0(ptr %3) #1, !noalias !11
  %5 = inttoptr i32 %ita.sroa.0.0 to ptr
  %cmp.i26 = icmp eq ptr %5, %4
  br i1 %cmp.i26, label %for.end14, label %for.body

for.body:                                         ; preds = %for.cond
  %6 = load ptr, ptr @myvector, align 4, !tbaa !7, !noalias !14
  %7 = call ptr @llvm.intel.std.container.ptr.iter.p0(ptr %6) #1, !noalias !14
  %8 = ptrtoint ptr %7 to i32
  br label %for.cond3

for.cond3:                                        ; preds = %for.body6, %for.body
  %it.sroa.0.0 = phi i32 [ %8, %for.body ], [ %17, %for.body6 ]
  %9 = load ptr, ptr getelementptr inbounds (%"class.std::vector", ptr @myvector, i32 0, i32 0, i32 0, i32 1), align 4, !tbaa !7, !noalias !17
  %10 = call ptr @llvm.intel.std.container.ptr.iter.p0(ptr %9) #1, !noalias !17
  %11 = inttoptr i32 %it.sroa.0.0 to ptr
  %cmp.i = icmp eq ptr %11, %10
  br i1 %cmp.i, label %for.inc12, label %for.body6

for.body6:                                        ; preds = %for.cond3
; CHECK:  %13 = load i32, ptr %12, align 4, !tbaa !19, !std.container.ptr.iter !15
; CHECK-NEXT: %mul = mul nsw i32 %11, %13
; CHECK-NEXT: %add = add nsw i32 %13, %mul
; CHECK-NEXT: store i32 %add, ptr %12, align 4, !tbaa !19, !std.container.ptr.iter !15
  %12 = inttoptr i32 %it.sroa.0.0 to ptr
  %13 = load i32, ptr %12, align 4, !tbaa !20
  %14 = inttoptr i32 %ita.sroa.0.0 to ptr
  %15 = load i32, ptr %14, align 4, !tbaa !20
  %mul = mul nsw i32 %15, %13
  %add = add nsw i32 %13, %mul
  store i32 %add, ptr %12, align 4, !tbaa !20
  %16 = inttoptr i32 %it.sroa.0.0 to ptr
  %incdec.ptr.i17 = getelementptr inbounds i32, ptr %16, i32 1
  %17 = ptrtoint ptr %incdec.ptr.i17 to i32
  br label %for.cond3

for.inc12:                                        ; preds = %for.cond3
  %18 = inttoptr i32 %ita.sroa.0.0 to ptr
  %incdec.ptr.i = getelementptr inbounds i32, ptr %18, i32 1
  %19 = ptrtoint ptr %incdec.ptr.i to i32
  br label %for.cond

for.end14:                                        ; preds = %for.cond
  ret void
}


; Function Attrs: nounwind readonly
declare ptr @llvm.intel.std.container.ptr.iter.p0(ptr) #6



attributes #1 = { nounwind }
attributes #6 = { nounwind readonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20444)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"struct@_ZTSSt12_Vector_baseIiSaIiEE", !3, i64 0}
!3 = !{!"struct@_ZTSNSt12_Vector_baseIiSaIiEE12_Vector_implE", !4, i64 0, !4, i64 4, !4, i64 8}
!4 = !{!"pointer@_ZTSPi", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!4, !4, i64 0}
!8 = !{!9}
!9 = distinct !{!9, !10, !"_ZNSt6vectorIiSaIiEE5beginEv: %agg.result"}
!10 = distinct !{!10, !"_ZNSt6vectorIiSaIiEE5beginEv"}
!11 = !{!12}
!12 = distinct !{!12, !13, !"_ZNSt6vectorIiSaIiEE3endEv: %agg.result"}
!13 = distinct !{!13, !"_ZNSt6vectorIiSaIiEE3endEv"}
!14 = !{!15}
!15 = distinct !{!15, !16, !"_ZNSt6vectorIiSaIiEE5beginEv: %agg.result"}
!16 = distinct !{!16, !"_ZNSt6vectorIiSaIiEE5beginEv"}
!17 = !{!18}
!18 = distinct !{!18, !19, !"_ZNSt6vectorIiSaIiEE3endEv: %agg.result"}
!19 = distinct !{!19, !"_ZNSt6vectorIiSaIiEE3endEv"}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !5, i64 0}
!22 = !{!3, !4, i64 0}
!23 = !{!3, !4, i64 4}
!24 = !{!3, !4, i64 8}

