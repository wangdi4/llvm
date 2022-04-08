// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
struct custom_op;
struct op;

typedef struct custom_op XOP;
typedef struct op OP;

typedef union {
  const char *xop_name;
  const char *xop_desc;
  unsigned int xop_class;
  void (*xop_peep)(OP *o, OP *oldop);
  XOP *xop_ptr;
} XOPRETANY;

// PTR: define dso_local "intel_dtrans_func_index"="1" i8* @Perl_custom_op_get_field(%struct._ZTS2op.op* noundef "intel_dtrans_func_index"="2" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[PERL_FUNC_MD:[0-9]+]]
// OPQ: define dso_local "intel_dtrans_func_index"="1" ptr @Perl_custom_op_get_field(ptr noundef "intel_dtrans_func_index"="2" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[PERL_FUNC_MD:[0-9]+]]
XOPRETANY Perl_custom_op_get_field(const OP *o) {
  XOPRETANY local;
  local.xop_class = 1;
  return local;
}

typedef struct {
  long long a;
  struct op *b;
} Complex;

typedef union {
  Complex a;
} ComplexRep;

// PTR: define dso_local "intel_dtrans_func_index"="1" { i64, %struct._ZTS2op.op* } @f() {{.*}}!intel.dtrans.func.type ![[F_FUNC_MD:[0-9]+]]
// OPQ: define dso_local "intel_dtrans_func_index"="1" { i64, ptr } @f() {{.*}}!intel.dtrans.func.type ![[F_FUNC_MD:[0-9]+]]
ComplexRep f() {
  ComplexRep r = {};
  return r;
}

// CHECK: !intel.dtrans.types = !{![[XOP:[0-9]+]], ![[OP:[0-9]+]], ![[COMPLEXREP:[0-9]+]], ![[COMPLEX:[0-9]+]]}
// CHECK: ![[XOP]] = !{!"S", %union._ZTS9XOPRETANY.XOPRETANY zeroinitializer, i32 1, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[OP]] = !{!"S", %struct._ZTS2op.op zeroinitializer, i32 -1}
// CHECK: ![[COMPLEXREP]] = !{!"S", %union._ZTS10ComplexRep.ComplexRep zeroinitializer, i32 1, ![[COMPLEX_REF:[0-9]+]]}
// CHECK: ![[COMPLEX_REF]] = !{%struct._ZTS7Complex.Complex zeroinitializer, i32 0}
// CHECK: ![[COMPLEX]] = !{!"S", %struct._ZTS7Complex.Complex zeroinitializer, i32 2, ![[LONGLONG:[0-9]+]], ![[OP_PTR:[0-9]+]]}
// CHECK: ![[LONGLONG]] = !{i64 0, i32 0}
// CHECK: ![[OP_PTR]] = !{%struct._ZTS2op.op zeroinitializer, i32 1}

// Function infos:
// CHECK: ![[PERL_FUNC_MD]] = distinct !{![[CHAR_PTR]], ![[OP_PTR]]}
// CHECK: ![[F_FUNC_MD]] = distinct !{![[CPLEX_LIT:[0-9]+]]}
// CHECK: ![[CPLEX_LIT]] = !{!"L", i32 2, ![[LONGLONG]], ![[OP_PTR]]}
