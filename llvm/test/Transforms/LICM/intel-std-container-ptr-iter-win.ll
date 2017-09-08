; RUN: opt < %s  -tbaa  -std-container-alias  -basiccg -domtree -basicaa -aa -std-container-opt -loops  -loop-rotate -licm   -S | FileCheck %s
;
; The compiler is expected to hoist out the load *ita. The header file vector
; is pre-processed under Windows.
;
; #include <vector>
; std::vector<int> myvector;
; std::vector<int> myvector2;
; std::vector<int>::iterator ita, it;
; void foo ()
; {
;       for (ita = myvector2.begin() ; ita != myvector2.end(); ++ita)
;         for (it = myvector.begin() ; it != myvector.end(); ++it)
;            *it = *it + *ita *(*it);
; }
;


target triple = "x86_64-pc-windows-msvc"

%"class.std::vector" = type { %"class.std::_Vector_alloc" }
%"class.std::_Vector_alloc" = type { %"class.std::_Vector_val" }
%"class.std::_Vector_val" = type { i32*, i32*, i32* }

@"\01?myvector@@3V?$vector@HV?$allocator@H@std@@@std@@A" = global %"class.std::vector" zeroinitializer, align 8
@"\01?myvector2@@3V?$vector@HV?$allocator@H@std@@@std@@A" = global %"class.std::vector" zeroinitializer, align 8

define void @"\01?foo@@YAXXZ"() {
entry:
  %tmp = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?myvector2@@3V?$vector@HV?$allocator@H@std@@@std@@A", i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !8, !noalias !13
  %cal.i.i.i17 = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp), !noalias !13
  br label %for.cond

for.cond:                                         ; preds = %for.inc14, %entry
  %ita.sroa.0.0 = phi i32* [ %cal.i.i.i17, %entry ], [ %incdec.ptr.i.i, %for.inc14 ]
  %tmp1 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?myvector2@@3V?$vector@HV?$allocator@H@std@@@std@@A", i64 0, i32 0, i32 0, i32 1), align 8, !tbaa !16, !noalias !17
  %cal.i.i.i15 = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp1), !noalias !17
  %lnot.i13 = icmp eq i32* %ita.sroa.0.0, %cal.i.i.i15
  br i1 %lnot.i13, label %for.end16, label %for.body

for.body:                                         ; preds = %for.cond
  %tmp2 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?myvector@@3V?$vector@HV?$allocator@H@std@@@std@@A", i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !8, !noalias !20
  %cal.i.i.i10 = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp2), !noalias !20
  br label %for.cond5

for.cond5:                                        ; preds = %for.body8, %for.body
  %it.sroa.0.0 = phi i32* [ %cal.i.i.i10, %for.body ], [ %incdec.ptr.i.i3, %for.body8 ]
  %tmp3 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?myvector@@3V?$vector@HV?$allocator@H@std@@@std@@A", i64 0, i32 0, i32 0, i32 1), align 8, !tbaa !16, !noalias !23
  %cal.i.i.i = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp3), !noalias !23
  %lnot.i = icmp eq i32* %it.sroa.0.0, %cal.i.i.i
  br i1 %lnot.i, label %for.inc14, label %for.body8

for.body8:                                        ; preds = %for.cond5
; CHECK: %tmp4 = load i32, i32* %it.sroa.0.03, align 4, !tbaa !27, !std.container.ptr.iter !23
; CHECK-NEXT: %mul = mul nsw i32 %tmp5, %tmp4
; CHECK-NEXT: %add = add nsw i32 %tmp4, %mul
; CHECK-NEXT: store i32 %add, i32* %it.sroa.0.03, align 4, !tbaa !27, !std.container.ptr.iter !23
  %tmp4 = load i32, i32* %it.sroa.0.0, align 4, !tbaa !26
  %tmp5 = load i32, i32* %ita.sroa.0.0, align 4, !tbaa !26
  %mul = mul nsw i32 %tmp5, %tmp4
  %add = add nsw i32 %tmp4, %mul
  store i32 %add, i32* %it.sroa.0.0, align 4, !tbaa !26
  %incdec.ptr.i.i3 = getelementptr inbounds i32, i32* %it.sroa.0.0, i64 1
  br label %for.cond5

for.inc14:                                        ; preds = %for.cond5
  %incdec.ptr.i.i = getelementptr inbounds i32, i32* %ita.sroa.0.0, i64 1
  br label %for.cond

for.end16:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind readonly
declare i32* @llvm.intel.std.container.ptr.iter.p0i32(i32*) #0

attributes #0 = { nounwind readonly }

!llvm.module.flags = !{!6}
!llvm.ident = !{!7}

!llvm.linker.options = !{!1}
!1 = !{!2, !3, !4, !5}
!2 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1800\22"}
!3 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!4 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MT_StaticRelease\22"}
!5 = !{!"/DEFAULTLIB:libcpmt.lib"}
!6 = !{i32 1, !"PIC Level", i32 2}
!7 = !{!"clang version 4.0.0 (trunk 17977)"}
!8 = !{!9, !10, i64 0}
!9 = !{!"struct@?AV?$_Vector_val@U?$_Simple_types@H@std@@@std@@", !10, i64 0, !10, i64 8, !10, i64 16}
!10 = !{!"pointer@?APEAH", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
!13 = !{!14}
!14 = distinct !{!14, !15, !"\01?begin@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ: %agg.result"}
!15 = distinct !{!15, !"\01?begin@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ"}
!16 = !{!9, !10, i64 8}
!17 = !{!18}
!18 = distinct !{!18, !19, !"\01?end@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ: %agg.result"}
!19 = distinct !{!19, !"\01?end@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ"}
!20 = !{!21}
!21 = distinct !{!21, !22, !"\01?begin@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ: %agg.result"}
!22 = distinct !{!22, !"\01?begin@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ"}
!23 = !{!24}
!24 = distinct !{!24, !25, !"\01?end@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ: %agg.result"}
!25 = distinct !{!25, !"\01?end@?$vector@HV?$allocator@H@std@@@std@@QEAA?AV?$_Vector_iterator@V?$_Vector_val@U?$_Simple_types@H@std@@@std@@@2@XZ"}
!26 = !{!27, !27, i64 0}
!27 = !{!"int", !11, i64 0}
