; RUN: opt < %s -enable-new-pm=0 -tbaa   -std-container-alias  -basiccg -domtree -basic-aa -aa -std-container-opt -domtree -loops  -loop-rotate -licm  -S | FileCheck %s
; RUN: opt < %s -passes="std-container-opt,loop-rotate,loop-mssa(licm)" -S | FileCheck %s
;
; The compiler is expected to hoist out the load *ita. The header file vector
; is pre-processed under Linux.
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

%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" = type { i32*, i32*, i32* }

@myvector = global %"class.std::vector" zeroinitializer, align 8
@myvector2 = global %"class.std::vector" zeroinitializer, align 8

define void @_Z3foov() {
entry:
  %tmp = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector2, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !1
  %call.i.i13 = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp)
  br label %for.cond

for.cond:                                         ; preds = %for.inc19, %entry
  %ita.sroa.0.0 = phi i32* [ %call.i.i13, %entry ], [ %incdec.ptr.i, %for.inc19 ]
  %tmp1 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector2, i64 0, i32 0, i32 0, i32 1), align 8, !tbaa !1
  %call.i.i12 = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp1)
  %cmp.i11 = icmp eq i32* %ita.sroa.0.0, %call.i.i12
  br i1 %cmp.i11, label %for.end21, label %for.body

for.body:                                         ; preds = %for.cond
  %tmp2 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !1
  %call.i.i8 = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp2)
  br label %for.cond8

for.cond8:                                        ; preds = %for.body13, %for.body
  %it.sroa.0.0 = phi i32* [ %call.i.i8, %for.body ], [ %incdec.ptr.i3, %for.body13 ]
  %tmp3 = load i32*, i32** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @myvector, i64 0, i32 0, i32 0, i32 1), align 8, !tbaa !1
  %call.i.i = call i32* @llvm.intel.std.container.ptr.iter.p0i32(i32* %tmp3)
  %cmp.i = icmp eq i32* %it.sroa.0.0, %call.i.i
  br i1 %cmp.i, label %for.inc19, label %for.body13

for.body13:                                       ; preds = %for.cond8
; CHECK: %tmp4 = load i32, i32* %it.sroa.0.03, align 4, !tbaa !7, !std.container.ptr.iter !6
; CHECK-NEXT: %mul = mul nsw i32 %tmp5, %tmp4
; CHECK-NEXT: %add = add nsw i32 %tmp4, %mul
; CHECK-NEXT: store i32 %add, i32* %it.sroa.0.03, align 4, !tbaa !7, !std.container.ptr.iter !6
  %tmp4 = load i32, i32* %it.sroa.0.0, align 4, !tbaa !5
  %tmp5 = load i32, i32* %ita.sroa.0.0, align 4, !tbaa !5
  %mul = mul nsw i32 %tmp5, %tmp4
  %add = add nsw i32 %tmp4, %mul
  store i32 %add, i32* %it.sroa.0.0, align 4, !tbaa !5
  %incdec.ptr.i3 = getelementptr inbounds i32, i32* %it.sroa.0.0, i64 1
  br label %for.cond8

for.inc19:                                        ; preds = %for.cond8
  %incdec.ptr.i = getelementptr inbounds i32, i32* %ita.sroa.0.0, i64 1
  br label %for.cond

for.end21:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind readonly
declare i32* @llvm.intel.std.container.ptr.iter.p0i32(i32*) #0

attributes #0 = { nounwind readonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 10980)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"pointer@_ZTSPi", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
