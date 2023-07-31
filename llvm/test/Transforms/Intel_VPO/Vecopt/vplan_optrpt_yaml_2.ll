; Test to chack that opt-report messages are printed for VPlan Vectorizer.
; Based on vplan_pointer_operand_widening test.

; RUN: opt <%s -passes=vplan-vec -vplan-force-vf=2 -pass-remarks-output=%t -S -disable-output ; FileCheck --input-file %t %s
; RUN: opt <%s -passes=vplan-vec -vplan-force-vf=1 -pass-remarks-output=%t -S -disable-output ; FileCheck --check-prefix=CHECK-NEG --allow-empty --input-file %t %s

; CHECK: Pass:{{[ ]*}}VPlan Vectorization
; CHECK: Function:{{[ ]*}}_ZGVeN16uu_testKernel
; CHECK: Remark:{{[ ]*}}Kernel was 2-way vectorized
; CHECK: Remark:{{[ ]*}}1 gathers
; CHECK: Remark:{{[ ]*}}1 scatters

; CHECK-NEG-NOT: {{.+}}

%Struct = type { <3 x i32>, i32 }

define void @_ZGVeN16uu_testKernel(ptr %a, i1 %flag) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %IfElseEnd ]
  %base = getelementptr %Struct, ptr %a, i64 %indvars.iv
  %ld = load <3 x i32>, ptr %base
  %ld.0 = extractelement <3 x i32> %ld, i32 0
  %cmp = icmp eq i32 %ld.0, 42
  br i1 %cmp, label %block1, label %block2

block1:
  br label %IfElseEnd

block2:
  %ptr2 = getelementptr inbounds %Struct, ptr %base, i32 1, i32 0
  br label %IfElseEnd

IfElseEnd:
  %phi = phi ptr [ %base, %block1], [ %ptr2, %block2]
  store <3 x i32> %ld, ptr %phi
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
