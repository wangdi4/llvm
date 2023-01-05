; RUN: opt -aa-pipeline=basic-aa -passes="require<aa>,aa-eval" -print-all-alias-modref-info  -disable-output < %s 2>&1 | FileCheck  %s

; This test case checks that the alias analysis between %alloc_site and
; %ret_ptr is marked as NoAlias since %ret_ptr won't reach the call to @bar.

; CHECK: NoAlias:	double* %alloc_site, double* %ret_ptr

declare void @bar(double* %arg)
declare noundef double* @baz()

define void @foo() {
  %alloc_site = alloca double
  %ret_ptr = call double* @baz()

  store double 0.0, double* %alloc_site
  store double 0.0, double* %ret_ptr

  call void @bar(double* %alloc_site)
  ret void
}