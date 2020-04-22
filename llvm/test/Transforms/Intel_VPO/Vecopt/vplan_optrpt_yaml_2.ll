; Test to chack that opt-report messages are printed for VPlan Vectorizer.
; Based on vplan_pointer_operand_widening test.

; RUN: opt <%s -VPlanDriver -vplan-force-vf=2 -pass-remarks-output=%t -S -disable-output ; FileCheck --input-file %t %s
; RUN: opt <%s -VPlanDriver -vplan-force-vf=1 -pass-remarks-output=%t -S -disable-output ; FileCheck --check-prefix=CHECK-NEG --allow-empty --input-file %t %s

; CHECK: Pass:{{[ ]*}}VPlan Vectorization
; CHECK: Function:{{[ ]*}}_ZGVeN16uu_testKernel
; CHECK: Remark:{{[ ]*}}Kernel was 2-way vectorized
; CHECK: Remark:{{[ ]*}}1 gathers
; CHECK: Remark:{{[ ]*}}1 scatters

; CHECK-NEG-NOT: {{.+}}

%Struct = type { <3 x i32>, i32 }

define void @_ZGVeN16uu_testKernel(%Struct *%a, i1 %flag) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %IfElseEnd ]
  %base = getelementptr %Struct, %Struct *%a, i64 %indvars.iv
  %ptr = getelementptr inbounds %Struct, %Struct * %base, i32 0, i32 0
  %ld = load <3 x i32>, <3 x i32>* %ptr
  %ld.0 = extractelement <3 x i32> %ld, i32 0
  %cmp = icmp eq i32 %ld.0, 42
  br i1 %cmp, label %block1, label %block2

block1:
  %ptr1 = getelementptr inbounds %Struct, %Struct * %base, i32 0, i32 0
  br label %IfElseEnd

block2:
  %ptr2 = getelementptr inbounds %Struct, %Struct * %base, i32 1, i32 0
  br label %IfElseEnd

IfElseEnd:
  %phi = phi <3 x i32>* [ %ptr1, %block1], [ %ptr2, %block2]
  store <3 x i32> %ld, <3 x i32>* %phi
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
