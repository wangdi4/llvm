// RUN: %clang_cc1 -fintel-compatibility --extended_float_types %s -emit-llvm -o - | FileCheck %s

// sizeof __complex128 type.
typedef _Complex float __attribute__((mode(TC))) __complex128;
int check_sizeof_Quad() {
  // CHECK: call{{.*}}@llvm.intel.sizeof{{.*}}32, { fp128, fp128 }* getelementptr ({ fp128, fp128 }, { fp128, fp128 }* null, i32 1))
  return sizeof(__complex128);
}

int varlen(int x)
{
  float (*y)[x][2][3];
  // Variable-length array of [2 x [3 x float]].
  // The element type is under sizeof (due to star).
  // CHECK: call{{.*}}@llvm.intel.sizeof{{.*}} {{%.+}}, [2 x [3 x float]]* getelementptr
  return sizeof(*(varlen(x-1),y));
}

template<typename T>
class TemplateClass {
public:
  void templateClassFunction() {
    int size = sizeof(__PRETTY_FUNCTION__);
  }
};
int user() {
  TemplateClass<double> t1;
  // llvm type of the __PRETTY_FUNCTION__, array of chars
  // CHECK: {{%.+}} = call{{.*}}@llvm.intel.sizeof{{.*}}[{{.*}} x i8]* getelementptr
  t1.templateClassFunction();
  return 0;
}

struct Str1 {
  int a;
  float b;
  char c;
  virtual int foo();
} x;

int struct_user() {
  Str1 var;
  // CHECK: {{%.+}} = call{{.*}}@llvm.intel.sizeof{{.*}}%struct.Str1* getelementptr (%struct.Str1, %struct.Str1* null, i32 1))
  return sizeof(var);
}

