// REQUIRES: intel_feature_sw_dtrans

// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK

template <int rank_, int dim, typename Number = double>
class Tensor;

template <int dim, typename Number>
class Tensor<0, dim, Number>
{
public:
  Number value;
};
template <int rank_, int dim, typename Number>
class Tensor
{
  Tensor<rank_ - 1, dim, Number> values[(dim != 0) ? dim : 1];
};

template <int dim, typename RangeNumberType = double>
class Function {
public:
  virtual Tensor<1, dim, RangeNumberType>
  gradient() const;

  virtual void
  vector_gradient() const;
};
template <int dim, typename RangeNumberType>
void
Function<dim, RangeNumberType>::vector_gradient() const {
  gradient();
}

template class Function< 1 ,  __complex__ float >;

// VTable and type-info for the Function class.
// CHECK: @_ZTV8FunctionILi1ECfE = {{.*}}{ [4 x ptr] }{{.*}} !intel_dtrans_type ![[FUNCTION_VTBL:[0-9]+]]
// CHECK: @_ZTI8FunctionILi1ECfE = {{.*}}{ ptr, ptr }{{.*}} !intel_dtrans_type ![[FUNCTION_TYPE_INFO:[0-9]+]]

// Function::vector_gradient function.
// CHECK: define {{.*}}void @_ZNK8FunctionILi1ECfE15vector_gradientEv(ptr{{.*}} "intel_dtrans_func_index"="1" %this) {{.*}}!intel.dtrans.func.type ![[VEC_GRAD_MD:[0-9]+]]
// CHECK: alloca ptr{{.*}}!intel_dtrans_type ![[FUNCTION_PTR:[0-9]+]]
// Call to the vtable entry for gradient.
// CHECK: call <2 x float> %{{.*}}(ptr{{.*}}), !intel_dtrans_type ![[GRADIENT_FPTR:[0-9]+]]

// Function::gradient declaration
// CHECK: declare !intel.dtrans.func.type ![[GRAD_MD:[0-9]+]] <2 x float> @_ZNK8FunctionILi1ECfE8gradientEv

// CHECK: intel.dtrans.types = !{![[FUNCTION:[0-9]+]], ![[TENSOR1:[0-9]+]], ![[TENSOR0:[0-9]+]]}

// CHECK: ![[FUNCTION_VTBL]] = !{!"L", i32 1, ![[CHAR_PTR_ARRAY:[0-9]+]]}
// CHECK: ![[CHAR_PTR_ARRAY]] = !{!"A", i32 4, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[FUNCTION_TYPE_INFO]] = !{!"L", i32 2, ![[CHAR_PTR]], ![[CHAR_PTR]]}

// Function type, contains only a VTable.
// CHECK: ![[FUNCTION]] = !{!"S", %class._ZTS8FunctionILi1ECfE.Function zeroinitializer, i32 1, ![[VTBL_PTR_PTR:[0-9]+]]}
// CHECK: ![[VTBL_PTR_PTR]] = !{![[VTBL_TY:[0-9]+]], i32 2}
// CHECK: ![[VTBL_TY]] = !{!"F", i1 true, i32 0, ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}

// Tensor<1> 
// CHECK: ![[TENSOR1]] = !{!"S", %class._ZTS6TensorILi1ELi1ECfE.Tensor zeroinitializer, i32 1, ![[ARR_OF_TENSOR_ZERO:[0-9]+]]}
// CHECK: ![[ARR_OF_TENSOR_ZERO]] = !{!"A", i32 1, ![[TENSOR0_REF:[0-9]+]]}
// CHECK: ![[TENSOR0_REF]] = !{%class._ZTS6TensorILi0ELi1ECfE.Tensor zeroinitializer, i32 0}

// Tensor<0>
// CHECK: ![[TENSOR0]] = !{!"S", %class._ZTS6TensorILi0ELi1ECfE.Tensor zeroinitializer, i32 1, ![[V:[0-9]+]]}
// CHECK: ![[V]] = !{![[L:[0-9]+]], i32 0}
// CHECK: ![[L]] = !{!"L", i32 2, ![[FLOAT:[0-9]+]], ![[FLOAT]]}
// CHECK: ![[FLOAT]] = !{float 0{{.*}}, i32 0}

// CHECK: ![[VEC_GRAD_MD]] = distinct !{![[FUNCTION_PTR]]}
// CHECK: ![[FUNCTION_PTR]] = !{%class._ZTS8FunctionILi1ECfE.Function zeroinitializer, i32 1}

// Function Ptr to Gradient (from VTable)
// CHECK: ![[GRADIENT_FPTR]] = !{!"F", i1 false, i32 1, ![[VEC_FLOATS:[0-9]+]], ![[FUNCTION_PTR]]}
// CHECK: ![[VEC_FLOATS]] = !{!"V", i32 2, ![[FLOAT]]}
// CHECK: ![[GRAD_MD]] = distinct !{![[FUNCTION_PTR]]}
