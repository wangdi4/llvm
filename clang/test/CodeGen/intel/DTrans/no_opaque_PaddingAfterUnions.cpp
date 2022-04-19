// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR

// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows-pc -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

typedef double UV_VECT[2];
typedef float COLOUR[5];

struct Blend_Map_Entry
{
  float value;
  unsigned char Same;
  union
  {
    COLOUR Colour;
    UV_VECT Point_Slope;
  } Vals;
};

Blend_Map_Entry *Create_BMapEntries(Blend_Map_Entry* BME){ return BME; }

// CHECK: %{{[\"]?}}struct.{{.+}}.Blend_Map_Entry{{[\"]?}} = type { float, i8, %{{[\"]?}}union.{{.+}}.anon{{[\"]?}} }
// CHECK: %{{[\"]?}}union.{{.+}}.anon{{[\"]?}} = type { [2 x double], [8 x i8] }

// PTR: define {{.+}}"intel_dtrans_func_index"="1" %{{[\"]?}}struct.{{.+}}.Blend_Map_Entry{{[\"]?}}* {{.+}}(%{{[\"]?}}struct.{{.+}}.Blend_Map_Entry{{[\"]?}}* noundef "intel_dtrans_func_index"="2" {{.+}}!intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]
// OPQ: define {{.+}}"intel_dtrans_func_index"="1" ptr {{.+}}(ptr noundef "intel_dtrans_func_index"="2" {{.+}}!intel.dtrans.func.type ![[FUNC_MD:[0-9]+]]

// CHECK: !intel.dtrans.types = !{![[BME:[0-9]+]], ![[UNION:[0-9]+]]}
// CHECK: ![[BME]] = !{!"S", %{{[\"]?}}struct.{{.+}}.Blend_Map_Entry{{[\"]?}} zeroinitializer, i32 3, ![[FLOAT:[0-9]+]], ![[CHAR:[0-9]+]], ![[UNION_REF:[0-9]+]]}
// CHECK: ![[FLOAT]] = !{float 0.{{.+}}, i32 0}
// CHECK: ![[CHAR]] = !{i8 0, i32 0}
// CHECK: ![[UNION_REF]] = !{%{{[\"]?}}union.{{.+}}.anon{{[\"]?}} zeroinitializer, i32 0}

// CHECK: ![[UNION]] = !{!"S", %{{[\"]?}}union.{{.+}}.anon{{[\"]?}} zeroinitializer, i32 2, ![[TWO_DOUBLES:[0-9]+]], ![[PADDING:[0-9]+]]}
// CHECK: ![[TWO_DOUBLES]] = !{!"A", i32 2, ![[DOUBLE:[0-9]+]]}
// CHECK: ![[DOUBLE]] = !{double 0{{.+}}, i32 0}
// CHECK: ![[PADDING]] = !{!"A", i32 8, ![[CHAR]]}

// CHECK: ![[FUNC_MD]] = distinct !{![[BME_PTR:[0-9]+]], ![[BME_PTR]]}
// CHECK: ![[BME_PTR]] = !{%{{[\"]?}}struct.{{.+}}.Blend_Map_Entry{{[\"]?}} zeroinitializer, i32 1}
