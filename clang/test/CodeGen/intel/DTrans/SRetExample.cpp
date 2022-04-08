// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
typedef double BigReal;
class Vector {
public:
  BigReal x, y, z;

  inline Vector(void) : x(-99999), y(-99999), z(-99999) { ; }

  inline Vector(BigReal newx, BigReal newy, BigReal newz)
      : x(newx), y(newy), z(newz) { ; }

  inline friend Vector operator+(const Vector &v1, const Vector &v2) {
    return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
  }

  inline friend Vector operator*(const BigReal &f, const Vector &v1) {
    return Vector(f * v1.x, f * v1.y, f * v1.z);
  }
};

class Data {
public:
  Vector offset(int i) const {
    return ((i % 3 - 1) * a1 + ((i / 3) % 3 - 1) * a2 + (i / 9 - 1) * a3);
  }

  Vector a1, a2, a3;
};

int test() {
  Data x;
  Vector y = x.offset(2);
  return y.x + y.x + y.z;
}

// Data constructor:
// PTR: define linkonce_odr void @_ZN4DataC1Ev(%class._ZTS4Data.Data* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[DATA_CTOR_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN4DataC1Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[DATA_CTOR_FUNC_MD:[0-9]+]]
//
// Data::offset:
// PTR: define linkonce_odr void @_ZNK4Data6offsetEi(%class._ZTS6Vector.Vector* noalias sret(%class._ZTS6Vector.Vector) align 8 "intel_dtrans_func_index"="1" %{{.+}}, %class._ZTS4Data.Data* {{[^,]*}}"intel_dtrans_func_index"="2" %{{.+}}, i32 noundef %{{.+}}) {{.*}}!intel.dtrans.func.type ![[OFFSET_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZNK4Data6offsetEi(ptr noalias sret(%class._ZTS6Vector.Vector) align 8 "intel_dtrans_func_index"="1" %{{.+}}, ptr {{[^,]*}}"intel_dtrans_func_index"="2" %{{.+}}, i32 noundef %{{.+}}) {{.*}}!intel.dtrans.func.type ![[OFFSET_FUNC_MD:[0-9]+]]
//
// Data constructor 2:
// PTR: define linkonce_odr void @_ZN4DataC2Ev(%class._ZTS4Data.Data* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[DATA_CTOR2_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN4DataC2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[DATA_CTOR2_FUNC_MD:[0-9]+]]
//
// Vector constructors 1 and 2:
// PTR: define linkonce_odr void @_ZN6VectorC1Ev(%class._ZTS6Vector.Vector* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_CTOR_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN6VectorC1Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_CTOR_FUNC_MD:[0-9]+]]
// PTR: define linkonce_odr void @_ZN6VectorC2Ev(%class._ZTS6Vector.Vector* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_CTOR2_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN6VectorC2Ev(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_CTOR2_FUNC_MD:[0-9]+]]
//
// Vector operator+, operator*:
// PTR: define linkonce_odr void @_ZplRK6VectorS1_(%class._ZTS6Vector.Vector* noalias sret(%class._ZTS6Vector.Vector) align 8 "intel_dtrans_func_index"="1" %{{.+}}, %class._ZTS6Vector.Vector* noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %{{.+}}, %class._ZTS6Vector.Vector* noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="3" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_PLUS_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZplRK6VectorS1_(ptr noalias sret(%class._ZTS6Vector.Vector) align 8 "intel_dtrans_func_index"="1" %{{.+}}, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="2" %{{.+}}, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="3" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_PLUS_FUNC_MD:[0-9]+]]
// PTR: define linkonce_odr void @_ZmlRKdRK6Vector(%class._ZTS6Vector.Vector* noalias sret(%class._ZTS6Vector.Vector) align 8 "intel_dtrans_func_index"="1" %{{.+}}, double* noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %{{.+}}, %class._ZTS6Vector.Vector* noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="3" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_MUL_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZmlRKdRK6Vector(ptr noalias sret(%class._ZTS6Vector.Vector) align 8 "intel_dtrans_func_index"="1" %{{.+}}, ptr noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %{{.+}}, ptr noundef nonnull align 8 dereferenceable(24) "intel_dtrans_func_index"="3" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_MUL_FUNC_MD:[0-9]+]]

// Vector constructor (3 doubles) 1 and 2:
// PTR: define linkonce_odr void @_ZN6VectorC1Eddd(%class._ZTS6Vector.Vector* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_DDD_CTOR_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN6VectorC1Eddd(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_DDD_CTOR_FUNC_MD:[0-9]+]]
// PTR: define linkonce_odr void @_ZN6VectorC2Eddd(%class._ZTS6Vector.Vector* {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_DDD_CTOR2_FUNC_MD:[0-9]+]]
// OPQ: define linkonce_odr void @_ZN6VectorC2Eddd(ptr {{[^,]*}}"intel_dtrans_func_index"="1" %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}, double noundef %{{.+}}) {{.*}}!intel.dtrans.func.type ![[VECTOR_DDD_CTOR2_FUNC_MD:[0-9]+]]
// CHECK: !intel.dtrans.types = !{![[DATA:[0-9]+]], ![[VECTOR:[0-9]+]]}

// CHECK: ![[DATA]] = !{!"S", %class._ZTS4Data.Data zeroinitializer, i32 3, ![[VECTOR_NP:[0-9]+]], ![[VECTOR_NP]], ![[VECTOR_NP]]}
// CHECK: ![[VECTOR_NP]] = !{%class._ZTS6Vector.Vector zeroinitializer, i32 0}
// CHECK: ![[VECTOR]] = !{!"S", %class._ZTS6Vector.Vector zeroinitializer, i32 3, ![[DOUBLE:[0-9]+]], ![[DOUBLE]], ![[DOUBLE]]}
// CHECK: ![[DOUBLE]] = !{double 0.0{{.+}}, i32 0}

// CHECK: ![[DATA_CTOR_FUNC_MD]] = distinct !{![[DATA_PTR:[0-9]+]]}
// CHECK: ![[DATA_PTR]] = !{%class._ZTS4Data.Data zeroinitializer, i32 1}
// CHECK: ![[OFFSET_FUNC_MD]] = distinct !{![[VECTOR_PTR:[0-9]+]], ![[DATA_PTR]]}
// CHECK: ![[VECTOR_PTR]] = !{%class._ZTS6Vector.Vector zeroinitializer, i32 1}
// CHECK: ![[DATA_CTOR2_FUNC_MD]] = distinct !{![[DATA_PTR]]}
// CHECK: ![[VECTOR_CTOR_FUNC_MD]] = distinct !{![[VECTOR_PTR]]}
// CHECK: ![[VECTOR_CTOR2_FUNC_MD]] = distinct !{![[VECTOR_PTR]]}
// CHECK: ![[VECTOR_PLUS_FUNC_MD]] = distinct !{![[VECTOR_PTR]], ![[VECTOR_PTR]], ![[VECTOR_PTR]]}
// CHECK: ![[VECTOR_MUL_FUNC_MD]] = distinct !{![[VECTOR_PTR]], ![[DOUBLE_PTR:[0-9]+]], ![[VECTOR_PTR]]}
// CHECK: ![[DOUBLE_PTR]] = !{double 0.0{{.+}}, i32 1}
// CHECK: ![[VECTOR_DDD_CTOR_FUNC_MD]] = distinct !{![[VECTOR_PTR]]}
// CHECK: ![[VECTOR_DDD_CTOR2_FUNC_MD]] = distinct !{![[VECTOR_PTR]]}
