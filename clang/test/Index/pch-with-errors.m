#ifndef HEADER
#define HEADER

@interface I(cat)
-(void)meth;
@end

@interface I2
-(void)meth;
@end

struct FFF1
extern I2 *somevar1;

enum FFF2
extern I2 *somevar2;

#else

void foo(I2 *i) {
  [i meth];
}

#endif

// RUN: c-index-test -write-pch %t.h.pch %s -Xclang -detailed-preprocessing-record
// INTEL RUN: c-index-test -test-load-source local %s -include-pch %t.h.pch -Xclang -detailed-preprocessing-record | FileCheck -check-prefix=CHECK-PARSE %s
// INTEL RUN: c-index-test -index-file %s -include-pch %t.h.pch -Xclang -detailed-preprocessing-record | FileCheck -check-prefix=CHECK-INDEX %s

// CHECK-PARSE: pch-with-errors.m:{{.*}} FunctionDecl=foo
// CHECK-PARSE: pch-with-errors.m:{{.*}} ObjCMessageExpr=meth

// CHECK-INDEX: [indexDeclaration]: kind: function | name: foo
// CHECK-INDEX: [indexEntityReference]: kind: objc-instance-method | name: meth
