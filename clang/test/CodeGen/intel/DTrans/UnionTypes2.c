// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
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

// CHECK: define dso_local "intel_dtrans_func_index"="1" i8* @Perl_custom_op_get_field(%struct.op* "intel_dtrans_func_index"="2" %{{.+}}) {{.*}}!intel.dtrans.func.type ![[PERL_FUNC_MD:[0-9]+]]
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

// CHECK: define dso_local "intel_dtrans_func_index"="1" { i64, %struct.op* } @f() {{.*}}!intel.dtrans.func.type ![[F_FUNC_MD:[0-9]+]]
ComplexRep f() {
  ComplexRep r = {};
  return r;
}

// CHECK: !intel.dtrans.types = !{![[XOP:[0-9]+]], ![[OP:[0-9]+]], ![[COMPLEXREP:[0-9]+]], ![[COMPLEX:[0-9]+]]}
// CHECK: ![[XOP]] = !{!"S", %union.XOPRETANY zeroinitializer, i32 1, ![[CHAR_PTR:[0-9]+]]}
// CHECK: ![[CHAR_PTR]] = !{i8 0, i32 1}
// CHECK: ![[OP]] = !{!"S", %struct.op zeroinitializer, i32 -1}
// CHECK: ![[COMPLEXREP]] = !{!"S", %union.ComplexRep zeroinitializer, i32 1, ![[COMPLEX_REF:[0-9]+]]}
// CHECK: ![[COMPLEX_REF]] = !{%struct.Complex zeroinitializer, i32 0}
// CHECK: ![[COMPLEX]] = !{!"S", %struct.Complex zeroinitializer, i32 2, ![[LONGLONG:[0-9]+]], ![[OP_PTR:[0-9]+]]}
// CHECK: ![[LONGLONG]] = !{i64 0, i32 0}
// CHECK: ![[OP_PTR]] = !{%struct.op zeroinitializer, i32 1}

// Function infos:
// CHECK: ![[PERL_FUNC_MD]] = distinct !{![[CHAR_PTR]], ![[OP_PTR]]}
// CHECK: ![[F_FUNC_MD]] = distinct !{![[CPLEX_LIT:[0-9]+]]}
// CHECK: ![[CPLEX_LIT]] = !{!"L", i32 2, ![[LONGLONG]], ![[OP_PTR]]}
