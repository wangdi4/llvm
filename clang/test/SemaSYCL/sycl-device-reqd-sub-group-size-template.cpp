// RUN: %clang_cc1 -fsycl -fsycl-is-device -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

// Test that checkes template parameter support for 'intel_reqd_sub_group_size' attribute on sycl device.

template <int SIZE>
class KernelFunctor {
public:
  //expected-error@+1{{'intel_reqd_sub_group_size' attribute requires a positive integral compile time constant expression}}
  [[cl::intel_reqd_sub_group_size(SIZE)]] void operator()() {}
};

int main() {
  //expected-note@+1{{in instantiation of template class 'KernelFunctor<-1>' requested here}}
  KernelFunctor<-1>();
  // no error expected
  KernelFunctor<10>();
}

// CHECK: ClassTemplateDecl {{.*}} {{.*}} KernelFunctor
// CHECK: ClassTemplateSpecializationDecl {{.*}} {{.*}} class KernelFunctor definition
// CHECK: CXXRecordDecl {{.*}} {{.*}} implicit class KernelFunctor
// CHECK: IntelReqdSubGroupSizeAttr {{.*}}
// CHECK: SubstNonTypeTemplateParmExpr {{.*}}
<<<<<<< HEAD
// CHECK-NEXT: NonTypeTemplateParmDecl
=======
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}}
>>>>>>> 9d1a28046d4b8ec95045fc1ecf3cafb17bc5b049
// CHECK-NEXT: IntegerLiteral{{.*}}10{{$}}
