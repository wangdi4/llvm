; RUN: opt -aa-pipeline=basic-aa -passes="require<aa>,aa-eval" -print-all-alias-modref-info  -disable-output < %s 2>&1 | FileCheck  %s

; This test case checks that the alias analysis between %alloc_site and
; %ret_ptr will be set marked as MayAlias %alloc_site escapes before the
; call to @bar.

; CHECK: MayAlias:	double* %alloc_site, double* %ret_ptr

declare void @bar(ptr %arg)
declare noundef ptr @baz()

define void @foo() {
  %alloc_site = alloca double
  call void @bar(ptr %alloc_site)
  %ret_ptr = call ptr @baz()

  store double 0.0, ptr %alloc_site
  store double 0.0, ptr %ret_ptr
  ret void
}
