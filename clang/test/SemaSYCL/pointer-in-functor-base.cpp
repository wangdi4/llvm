// RUN: %clang_cc1 -fsycl-is-device -ast-dump %s | FileCheck %s

// Make sure we handle the wrapping of a pointer correctly in a base class of
// the functor, and don't crash anymore.
template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(const Func &kernelFunc) {
  kernelFunc();
}

struct Base {
  int *i;
};

struct Functor : Base {
  void operator()() const {}
  int k;
};

void use() {
  Functor F;
  kernel<class KernelName>(F);
}

// CHECK: FunctionDecl {{.*}}KernelName{{.*}} 'void (__wrapper_class, int)'
// CHECK-NEXT: ParmVarDecl {{.*}} used _arg_i '__wrapper_class'
// CHECK-NEXT: ParmVarDecl {{.*}} used _arg_k 'int'
// CHECK-NEXT: CompoundStmt
// CHECK-NEXT: DeclStmt
// CHECK-NEXT: VarDecl {{.*}} Functor 'Functor'
// CHECK-NEXT: InitListExpr {{.*}} 'Functor'
// CHECK-NEXT: InitListExpr {{.*}} 'Base'
// CHECK-NEXT: ImplicitCastExpr {{.*}} 'int *' <AddressSpaceConversion>
// CHECK-NEXT: ImplicitCastExpr {{.*}} '__global int *' <LValueToRValue>
// CHECK-NEXT: MemberExpr {{.*}} '__global int *' lvalue .
// CHECK-NEXT: DeclRefExpr {{.*}} '__wrapper_class' lvalue ParmVar {{.*}} '_arg_i' '__wrapper_class'
// CHECK-NEXT: ImplicitCastExpr {{.*}} 'int' <LValueToRValue>
// CHECK-NEXT: DeclRefExpr {{.*}} 'int' lvalue ParmVar {{.*}} '_arg_k' 'int'


