; RUN: opt < %s  -mtriple=x86_64-unknown-unknown -mattr=+avx2 -cost-model -analyze -enable-new-pm=0 \
; RUN:     | FileCheck %s

define void @foo() local_unnamed_addr #0 {
; CHECK-LABEL:  Printing analysis 'Cost Model Analysis' for function 'foo':
; CHECK-NEXT:  Cost Model: Found an estimated cost of 0 for instruction:   [[TOK:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
; CHECK-NEXT:  Cost Model: Found an estimated cost of 0 for instruction:   call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK-NEXT:  Cost Model: Found an estimated cost of 0 for instruction:   ret void
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}


; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
