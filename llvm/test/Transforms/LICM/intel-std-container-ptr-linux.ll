; RUN: opt < %s -enable-new-pm=0 -tbaa   -std-container-alias   -basiccg -domtree -basic-aa -aa -std-container-opt  -loops -loop-rotate -licm  -S | FileCheck %s
; RUN: opt < %s -passes="std-container-opt,loop-rotate,loop-mssa(licm)" -S | FileCheck %s
;
; The compiler is exptected to hoisted out the load a[i][k] out of the loop j. 
; The header file vector is pre-process under Linux.
; #include <vector>
; #define CONST_VECSIZE 2050
; 
; float TIME,RESULT;
; std::vector< std::vector<float> > a(CONST_VECSIZE);
; std::vector< std::vector<float> > b(CONST_VECSIZE),c(CONST_VECSIZE);
; void test_vector()
; {
;   int i, j, k;


;   for(i=0;i<CONST_VECSIZE;i++)
;     for(k=0;k<CONST_VECSIZE;k++)
;       for(j=0;j<CONST_VECSIZE;j++)
;            c[i][j] = c[i][j] + a[i][k]* b[k][j];
;
;}

target triple = "x86_64-unknown-linux-gnu"

%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<std::vector<float, std::allocator<float> >, std::allocator<std::vector<float, std::allocator<float> > > >::_Vector_impl" }
%"struct.std::_Vector_base<std::vector<float, std::allocator<float> >, std::allocator<std::vector<float, std::allocator<float> > > >::_Vector_impl" = type { %"class.std::vector.0"*, %"class.std::vector.0"*, %"class.std::vector.0"* }
%"class.std::vector.0" = type { %"struct.std::_Vector_base.1" }
%"struct.std::_Vector_base.1" = type { %"struct.std::_Vector_base<float, std::allocator<float> >::_Vector_impl" }
%"struct.std::_Vector_base<float, std::allocator<float> >::_Vector_impl" = type { float*, float*, float* }

@a = global %"class.std::vector" zeroinitializer, align 8
@b = global %"class.std::vector" zeroinitializer, align 8
@c = global %"class.std::vector" zeroinitializer, align 8

define void @_Z11test_vectorv() {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc24, %entry
  %storemerge = phi i32 [ 0, %entry ], [ %inc25, %for.inc24 ]
  %cmp = icmp slt i32 %storemerge, 2050
  br i1 %cmp, label %for.body, label %for.end26

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc21, %for.body
  %storemerge1 = phi i32 [ 0, %for.body ], [ %inc22, %for.inc21 ]
  %cmp2 = icmp slt i32 %storemerge1, 2050
  br i1 %cmp2, label %for.body3, label %for.end23

for.body3:                                        ; preds = %for.cond1
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %storemerge2 = phi i32 [ 0, %for.body3 ], [ %inc, %for.inc ]
  %cmp5 = icmp slt i32 %storemerge2, 2050
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
; CHECK: %tmp2 = load float, float* %add.ptr.i15, align 4, !tbaa !14, !std.container.ptr !7
; CHECK-NEXT: %conv15 = sext i32 %storemerge21 to i64
; CHECK-NEXT: %add.ptr.i7 = getelementptr inbounds float, float* %tmp7, i64 %conv15
; CHECK-NEXT: %tmp8 = load float, float* %add.ptr.i7, align 4, !tbaa !14, !std.container.ptr !9
; CHECK-NEXT: %mul = fmul float %tmp5, %tmp8
; CHECK-NEXT: %add = fadd float %tmp2, %mul
; CHECK-NEXT: %conv19 = sext i32 %storemerge21 to i64
; CHECK-NEXT: %add.ptr.i3 = getelementptr inbounds float, float* %tmp10, i64 %conv19
; CHECK-NEXT: store float %add, float* %add.ptr.i3, align 4, !tbaa !14, !std.container.ptr !7
  %conv = sext i32 %storemerge to i64
  %tmp = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @c, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !1
  %call.i = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp)
  %add.ptr.i = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i, i64 %conv
  %conv7 = sext i32 %storemerge2 to i64
  %_M_start.i14 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %add.ptr.i, i64 0, i32 0, i32 0, i32 0
  %tmp1 = load float*, float** %_M_start.i14, align 8, !tbaa !7
  %add.ptr.i15 = getelementptr inbounds float, float* %tmp1, i64 %conv7
  %tmp2 = load float, float* %add.ptr.i15, align 4, !tbaa !11
  %conv9 = sext i32 %storemerge to i64
  %tmp3 = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @a, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !1
  %call.i12 = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp3)
  %add.ptr.i13 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i12, i64 %conv9
  %conv11 = sext i32 %storemerge1 to i64
  %_M_start.i10 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %add.ptr.i13, i64 0, i32 0, i32 0, i32 0
  %tmp4 = load float*, float** %_M_start.i10, align 8, !tbaa !7
  %add.ptr.i11 = getelementptr inbounds float, float* %tmp4, i64 %conv11
  %tmp5 = load float, float* %add.ptr.i11, align 4, !tbaa !11
  %conv13 = sext i32 %storemerge1 to i64
  %tmp6 = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @b, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !1
  %call.i8 = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp6)
  %add.ptr.i9 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i8, i64 %conv13
  %conv15 = sext i32 %storemerge2 to i64
  %_M_start.i6 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %add.ptr.i9, i64 0, i32 0, i32 0, i32 0
  %tmp7 = load float*, float** %_M_start.i6, align 8, !tbaa !7
  %add.ptr.i7 = getelementptr inbounds float, float* %tmp7, i64 %conv15
  %tmp8 = load float, float* %add.ptr.i7, align 4, !tbaa !11
  %mul = fmul float %tmp5, %tmp8
  %add = fadd float %tmp2, %mul
  %conv17 = sext i32 %storemerge to i64
  %tmp9 = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @c, i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !1
  %call.i4 = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp9)
  %add.ptr.i5 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i4, i64 %conv17
  %conv19 = sext i32 %storemerge2 to i64
  %_M_start.i = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %add.ptr.i5, i64 0, i32 0, i32 0, i32 0
  %tmp10 = load float*, float** %_M_start.i, align 8, !tbaa !7
  %add.ptr.i3 = getelementptr inbounds float, float* %tmp10, i64 %conv19
  store float %add, float* %add.ptr.i3, align 4, !tbaa !11
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %inc = add nsw i32 %storemerge2, 1
  br label %for.cond4

for.end:                                          ; preds = %for.cond4
  br label %for.inc21

for.inc21:                                        ; preds = %for.end
  %inc22 = add nsw i32 %storemerge1, 1
  br label %for.cond1

for.end23:                                        ; preds = %for.cond1
  br label %for.inc24

for.inc24:                                        ; preds = %for.end23
  %inc25 = add nsw i32 %storemerge, 1
  br label %for.cond

for.end26:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind readonly
declare %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"*) #0

attributes #0 = { nounwind readonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 10980)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"struct@_ZTSSt12_Vector_baseISt6vectorIfSaIfEESaIS2_EE", !3, i64 0}
!3 = !{!"struct@_ZTSNSt12_Vector_baseISt6vectorIfSaIfEESaIS2_EE12_Vector_implE", !4, i64 0, !4, i64 8, !4, i64 16}
!4 = !{!"pointer@_ZTSPSt6vectorIfSaIfEE", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !10, i64 0}
!8 = !{!"struct@_ZTSSt12_Vector_baseIfSaIfEE", !9, i64 0}
!9 = !{!"struct@_ZTSNSt12_Vector_baseIfSaIfEE12_Vector_implE", !10, i64 0, !10, i64 8, !10, i64 16}
!10 = !{!"pointer@_ZTSPf", !5, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"float", !5, i64 0}
