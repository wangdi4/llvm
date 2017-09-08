; RUN: opt < %s   -tbaa  -std-container-alias   -basiccg -domtree -basicaa -aa -std-container-opt  -loops  -loop-rotate -licm   -S | FileCheck %s

; The compiler is exptected to hoisted out the load a[i][k] out of the loop j. 
; The header file vecotor is pre-process under windows.
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
; }

target triple = "x86_64-pc-windows-msvc"

%"class.std::vector" = type { %"class.std::_Vector_alloc" }
%"class.std::_Vector_alloc" = type { %"class.std::_Vector_val" }
%"class.std::_Vector_val" = type { %"class.std::vector.0"*, %"class.std::vector.0"*, %"class.std::vector.0"* }
%"class.std::vector.0" = type { %"class.std::_Vector_alloc.1" }
%"class.std::_Vector_alloc.1" = type { %"class.std::_Vector_val.2" }
%"class.std::_Vector_val.2" = type { float*, float*, float* }

@"\01?a@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A" = global %"class.std::vector" zeroinitializer, align 8
@"\01?b@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A" = global %"class.std::vector" zeroinitializer, align 8
@"\01?c@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A" = global %"class.std::vector" zeroinitializer, align 8

define void @"\01?test_vector@@YAXXZ"() {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc24, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc25, %for.inc24 ]
  %cmp = icmp slt i32 %i.0, 2050
  br i1 %cmp, label %for.body, label %for.end26

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc21, %for.body
  %k.0 = phi i32 [ 0, %for.body ], [ %inc22, %for.inc21 ]
  %cmp2 = icmp slt i32 %k.0, 2050
  br i1 %cmp2, label %for.body3, label %for.inc24

for.body3:                                        ; preds = %for.cond1
  br label %for.cond4

for.cond4:                                        ; preds = %for.body6, %for.body3
  %j.0 = phi i32 [ 0, %for.body3 ], [ %inc, %for.body6 ]
  %cmp5 = icmp slt i32 %j.0, 2050
  br i1 %cmp5, label %for.body6, label %for.inc21

for.body6:                                        ; preds = %for.cond4
; CHECK: %tmp2 = load float, float* %add.ptr.i13, align 4, !tbaa !18, !std.container.ptr !12
; CHECK-NEXT: %add.ptr.i5 = getelementptr inbounds float, float* %tmp7, i64 %conv7
; CHECK-NEXT: %tmp8 = load float, float* %add.ptr.i5, align 4, !tbaa !18, !std.container.ptr !14
; CHECK-NEXT: %mul = fmul float %tmp5, %tmp8
; CHECK-NEXT: %add = fadd float %tmp2, %mul
; CHECK-NEXT: %add.ptr.i1 = getelementptr inbounds float, float* %tmp10, i64 %conv7
; CHECK-NEXT: store float %add, float* %add.ptr.i1, align 4, !tbaa !18, !std.container.ptr !12
  %conv = sext i32 %i.0 to i64
  %tmp = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?c@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A", i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !8
  %call.i = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp)
  %conv7 = sext i32 %j.0 to i64
  %_Myfirst.i12 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i, i64 %conv, i32 0, i32 0, i32 0
  %tmp1 = load float*, float** %_Myfirst.i12, align 8, !tbaa !13
  %add.ptr.i13 = getelementptr inbounds float, float* %tmp1, i64 %conv7
  %tmp2 = load float, float* %add.ptr.i13, align 4, !tbaa !16
  %tmp3 = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?a@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A", i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !8
  %call.i10 = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp3)
  %conv11 = sext i32 %k.0 to i64
  %_Myfirst.i8 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i10, i64 %conv, i32 0, i32 0, i32 0
  %tmp4 = load float*, float** %_Myfirst.i8, align 8, !tbaa !13
  %add.ptr.i9 = getelementptr inbounds float, float* %tmp4, i64 %conv11
  %tmp5 = load float, float* %add.ptr.i9, align 4, !tbaa !16
  %tmp6 = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?b@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A", i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !8
  %call.i6 = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp6)
  %_Myfirst.i4 = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i6, i64 %conv11, i32 0, i32 0, i32 0
  %tmp7 = load float*, float** %_Myfirst.i4, align 8, !tbaa !13
  %add.ptr.i5 = getelementptr inbounds float, float* %tmp7, i64 %conv7
  %tmp8 = load float, float* %add.ptr.i5, align 4, !tbaa !16
  %mul = fmul float %tmp5, %tmp8
  %add = fadd float %tmp2, %mul
  %tmp9 = load %"class.std::vector.0"*, %"class.std::vector.0"** getelementptr inbounds (%"class.std::vector", %"class.std::vector"* @"\01?c@@3V?$vector@V?$vector@MV?$allocator@M@std@@@std@@V?$allocator@V?$vector@MV?$allocator@M@std@@@std@@@2@@std@@A", i64 0, i32 0, i32 0, i32 0), align 8, !tbaa !8
  %call.i2 = call %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"* %tmp9)
  %_Myfirst.i = getelementptr inbounds %"class.std::vector.0", %"class.std::vector.0"* %call.i2, i64 %conv, i32 0, i32 0, i32 0
  %tmp10 = load float*, float** %_Myfirst.i, align 8, !tbaa !13
  %add.ptr.i1 = getelementptr inbounds float, float* %tmp10, i64 %conv7
  store float %add, float* %add.ptr.i1, align 4, !tbaa !16
  %inc = add nsw i32 %j.0, 1
  br label %for.cond4

for.inc21:                                        ; preds = %for.cond4
  %inc22 = add nsw i32 %k.0, 1
  br label %for.cond1

for.inc24:                                        ; preds = %for.cond1
  %inc25 = add nsw i32 %i.0, 1
  br label %for.cond

for.end26:                                        ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind readonly
declare %"class.std::vector.0"* @"llvm.intel.std.container.ptr.p0class.std::vector.0"(%"class.std::vector.0"*) #0

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
!9 = !{!"struct@?AV?$_Vector_val@U?$_Simple_types@V?$vector@MV?$allocator@M@std@@@std@@@std@@@std@@", !10, i64 0, !10, i64 8, !10, i64 16}
!10 = !{!"pointer@?APEAV?$vector@MV?$allocator@M@std@@@std@@", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
!13 = !{!14, !15, i64 0}
!14 = !{!"struct@?AV?$_Vector_val@U?$_Simple_types@M@std@@@std@@", !15, i64 0, !15, i64 8, !15, i64 16}
!15 = !{!"pointer@?APEAM", !11, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"float", !11, i64 0}
