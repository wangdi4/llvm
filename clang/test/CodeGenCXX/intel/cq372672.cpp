// RUN: %clang_cc1 -fms-compatibility -fintel-compatibility -triple=i686-windows-msvc %s -emit-llvm -o - | FileCheck %s
// REQUIRES: llvm-backend

int main(void)
{
  double  _Complex c1, c2;
  c1 = c1 * c1;
// CHECK: [[RES1:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES2:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES3:%.+]] = fsub double [[RES1]], [[RES2]]
// CHECK: [[RES4:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES5:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[RES6:%.+]] = fadd double [[RES4]], [[RES5]]
  c2 = c2 / c2;
// CHECK: [[T0:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T1:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T2:%.+]] = fadd double [[T0]], [[T1]]
// CHECK: [[T3:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T4:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T5:%.+]] = fadd double [[T3]], [[T4]]
// CHECK: [[T6:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T7:%.+]] = {{fmul double %.+, %.+}}
// CHECK: [[T8:%.+]] = fsub double [[T6]], [[T7]]
// CHECK: [[T9:%.+]] = fdiv double [[T2]], [[T5]]
// CHECK: [[T10:%.+]] = fdiv double [[T8]], [[T5]]
}

// CQ#377455: More complex case, that led to another fail

extern double sqrt(double arg);

struct quantum_reg_node_struct
{
    float _Complex amplitude;
};

typedef struct quantum_reg_node_struct quantum_reg_node;

struct quantum_reg_struct
{
    quantum_reg_node *node;
};

typedef struct quantum_reg_struct quantum_reg;

void quantum_bmeasure_bitpreserve(quantum_reg *reg)
{
    quantum_reg out;
    out.node[0].amplitude = reg->node[0].amplitude * 1 / (float) sqrt(0);
// CHECK: [[RES2_1:%.+]] = call double @"\01?sqrt@@YANN@Z"(double 0.000000e+00)
// CHECK: [[RES2_2:%.+]] = fptrunc double [[RES2_1]] to float
// CHECK: {{.+}} = fdiv float {{%.+}}, [[RES2_2]]
// CHECK: {{.+}} = fdiv float {{%.+}}, [[RES2_2]]
}

