// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-print %s \
// RUN:  -fopenmp-late-outline | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-dump %s \
// RUN:  -fopenmp-late-outline | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -fopenmp-late-outline \
// RUN:   -emit-pch -o %t.pch %s

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-print \
// RUN:  -fopenmp-late-outline -include-pch %t.pch %s \
// RUN:  | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -ast-dump-all \
// RUN:  -fopenmp-late-outline -include-pch %t.pch %s \
// RUN:  | FileCheck %s --check-prefix=DUMP

// RUN: %clang_cc1 -verify -triple i386-pc-win32 -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -ast-dump %s \
// RUN:  | FileCheck %s --check-prefix=DUMP

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

//PRINT:template<> float foo<float>(float t)
//DUMP:FunctionDecl{{.*}}foo 'float (float)'
template <class T>
T foo(T t) {
  T v = T();
  T x = T();
  T expr = T();
  T e = T();
  T d = T();
  int r = 0;

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (expr < x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  #pragma omp atomic compare capture
  { v = x; if (expr < x) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (expr < x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  { if (expr < x) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (expr > x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  #pragma omp atomic compare capture
  { v = x; if (expr > x) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (expr > x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  { if (expr > x) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (x < expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  #pragma omp atomic compare capture
  { v = x; if (x < expr) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x < expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  { if (x < expr) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (x > expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  #pragma omp atomic compare capture
  { v = x; if (x > expr) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x > expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'expr' 'float':'float'
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  { if (x > expr) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (x == e) {
  //PRINT:            x = d;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'e' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'d' 'float':'float'
  #pragma omp atomic compare capture
  { v = x; if (x == e) { x = d; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x == e) {
  //PRINT:            x = d;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'e' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'d' 'float':'float'
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  { if (x == e) { x = d; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:      if (x == e) {
  //PRINT:          x = d;
  //PRINT:      } else {
  //PRINT:          v = x;
  //PRINT:      }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'e' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'d' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  if (x == e) { x = d; } else { v = x; }

  //PRINT:  #pragma omp atomic compare capture
  //PRINT:      {
  //PRINT:          r = x == e;
  //PRINT:          if (r) {
  //PRINT:              x = d;
  //PRINT:          }
  //PRINT:      }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'r' 'int'
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'e' 'float':'float'
  //DUMP:IfStmt
  //DUMP:DeclRefExpr {{.*}}'r' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'d' 'float':'float'
  #pragma omp atomic compare capture
  { r = x == e; if (r) { x = d; } }

  //PRINT:  #pragma omp atomic compare capture
  //PRINT:      {
  //PRINT:          r = x == e;
  //PRINT:          if (r) {
  //PRINT:              x = d;
  //PRINT:          } else {
  //PRINT:              v = x;
  //PRINT:          }
  //PRINT:      }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'r' 'int'
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'e' 'float':'float'
  //DUMP:IfStmt
  //DUMP:DeclRefExpr {{.*}}'r' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'d' 'float':'float'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'float':'float' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'float':'float'
  //DUMP:DeclRefExpr {{.*}}'x' 'float':'float'
  #pragma omp atomic compare capture
  { r = x == e; if (r) { x = d; } else { v = x; } }

  return T();
}



//PRINT:void bar() {
//DUMP:FunctionDecl {{.*}}bar 'void ()'
void bar() {
  int v = 0, x = 1, expr = 2, e = 3, d = 4, r = 5;

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (expr < x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  #pragma omp atomic compare capture
  { v = x; if (expr < x) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (expr < x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  #pragma omp atomic compare capture
  { if (expr < x) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (expr > x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  #pragma omp atomic compare capture
  { v = x; if (expr > x) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (expr > x) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  #pragma omp atomic compare capture
  { if (expr > x) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (x < expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  #pragma omp atomic compare capture
  { v = x; if (x < expr) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x < expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '<'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  #pragma omp atomic compare capture
  { if (x < expr) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (x > expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  #pragma omp atomic compare capture
  { v = x; if (x > expr) { x = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x > expr) {
  //PRINT:            x = expr;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  #pragma omp atomic compare capture
  { if (x > expr) { x = expr; } v = x; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x;
  //PRINT:        if (x == e) {
  //PRINT:            x = d;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'e' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'d' 'int'
  #pragma omp atomic compare capture
  { v = x; if (x == e) { x = d; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x == e) {
  //PRINT:            x = d;
  //PRINT:        }
  //PRINT:        v = x;
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'e' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  //DUMP:DeclRefExpr {{.*}}'d' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:DeclRefExpr {{.*}}'x' 'int'
  #pragma omp atomic compare capture
  { if (x == e) { x = d; } v = x; }

  foo(float());
}

//PRINT:class A {
//DUMP:CXXRecordDecl {{.*}}class A definition
class A {
  int v = 0;
  int x = 1, expr = 2;
  int e = 2, d = 3;
  //DUMP:CXXMethodDecl {{.*}}bar 'void (){{.*}}'
  void bar() {

    //PRINT:#pragma omp atomic capture compare
    //PRINT:    {
    //PRINT:        this->v = this->x;
    //PRINT:        if (this->x > this->expr) {
    //PRINT:            this->x = this->expr;
    //PRINT:        }
    //PRINT:    }
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCaptureClause
    //DUMP:OMPCompareClause
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->v
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:IfStmt
    //DUMP:BinaryOperator {{.*}}'bool' '>'
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->expr
    //DUMP:CompoundStmt
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->expr
    #pragma omp atomic capture compare
    { v = x; if (x > expr) { x = expr; } }

    //PRINT:#pragma omp atomic capture compare
    //PRINT:    {
    //PRINT:        if (this->x > this->expr) {
    //PRINT:            this->x = this->expr;
    //PRINT:        }
    //PRINT:        this->v = this->x;
    //PRINT:    }
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCaptureClause
    //DUMP:OMPCompareClause
    //DUMP:IfStmt
    //DUMP:BinaryOperator {{.*}}'bool' '>'
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->expr
    //DUMP:CompoundStmt
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->expr
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->v
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    #pragma omp atomic capture compare
    { if (x > expr) { x = expr; } v = x; }

    //PRINT:#pragma omp atomic capture compare
    //PRINT:    {
    //PRINT:        this->v = this->x;
    //PRINT:        if (this->x == this->e) {
    //PRINT:            this->x = this->d;
    //PRINT:        }
    //PRINT:    }
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCaptureClause
    //DUMP:OMPCompareClause
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->v
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:IfStmt
    //DUMP:BinaryOperator {{.*}}'bool' '=='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->e
    //DUMP:CompoundStmt
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->d
    #pragma omp atomic capture compare
    { v = x; if (x == e) { x = d; } }

    //PRINT:#pragma omp atomic capture compare
    //PRINT:    {
    //PRINT:        if (this->x == this->e) {
    //PRINT:            this->x = this->d;
    //PRINT:        }
    //PRINT:        this->v = this->x;
    //PRINT:    }
    //DUMP:OMPAtomicDirective
    //DUMP:OMPCaptureClause
    //DUMP:OMPCompareClause
    //DUMP:IfStmt
    //DUMP:BinaryOperator {{.*}}'bool' '=='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->e
    //DUMP:CompoundStmt
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    //DUMP:MemberExpr {{.*}}'int' lvalue ->d
    //DUMP:BinaryOperator {{.*}}'int' lvalue '='
    //DUMP:MemberExpr {{.*}}'int' lvalue ->v
    //DUMP:MemberExpr {{.*}}'int' lvalue ->x
    #pragma omp atomic capture compare
    { if (x == e) { x = d; } v = x; }
  }
};

//PRINT:void test() {
//DUMP:FunctionDecl {{.*}}test 'void ()'
void test()
{
  int v = 0;
  int x[4] = { 0,1,2,3};
  int expr = 1, e = 2, d = 3;

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x[2];
  //PRINT:        if (x[2] > expr) {
  //PRINT:            x[2] = expr;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  #pragma omp atomic compare capture
  { v = x[2]; if (x[2] > expr) { x[2] = expr; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x[2] > expr) {
  //PRINT:            x[2] = expr;
  //PRINT:        }
  //PRINT:        v = x[2];
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '>'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  //DUMP:DeclRefExpr {{.*}}'expr' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 2
  #pragma omp atomic compare capture
  { if (x[2] > expr) { x[2] = expr; } v = x[2]; }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        v = x[3];
  //PRINT:        if (x[3] == e) {
  //PRINT:            x[3] = d;
  //PRINT:        }
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  //DUMP:DeclRefExpr {{.*}}'e' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  //DUMP:DeclRefExpr {{.*}}'d' 'int'
  #pragma omp atomic compare capture
  { v = x[3]; if (x[3] == e) { x[3] = d; } }

  //PRINT:#pragma omp atomic compare capture
  //PRINT:    {
  //PRINT:        if (x[3] == e) {
  //PRINT:            x[3] = d;
  //PRINT:        }
  //PRINT:        v = x[3];
  //PRINT:    }
  //DUMP:OMPAtomicDirective
  //DUMP:OMPCompareClause
  //DUMP:OMPCaptureClause
  //DUMP:IfStmt
  //DUMP:BinaryOperator {{.*}}'bool' '=='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  //DUMP:DeclRefExpr {{.*}}'e' 'int'
  //DUMP:CompoundStmt
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  //DUMP:DeclRefExpr {{.*}}'d' 'int'
  //DUMP:BinaryOperator {{.*}}'int' lvalue '='
  //DUMP:DeclRefExpr {{.*}}'v' 'int'
  //DUMP:ArraySubscriptExpr
  //DUMP:DeclRefExpr {{.*}}'x' 'int[4]'
  //DUMP:IntegerLiteral {{.*}}'int' 3
  #pragma omp atomic compare capture
  { if (x[3] == e) { x[3] = d; } v = x[3]; }
}

#endif // HEADER
// end INTEL_COLLAB
