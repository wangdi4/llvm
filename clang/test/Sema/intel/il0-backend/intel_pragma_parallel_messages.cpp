// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma parallel test

#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
struct Class1 {
  int as;
#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
  char b;
};

#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
struct St1 {
  float rrr;
  double e;
} a;

#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
int test(int argc) {
  int wwww;
#pragma parallel               // expected-warning {{this pragma may not be used here}}
#pragma parallel always        // expected-warning {{this pragma may not be used here}}
#pragma parallel always assert // expected-warning {{this pragma may not be used here}}
#pragma parallel fgsdswf c     // expected-error {{invalid parallel pragma}}
  float fdf;
  return (argc);
}

#pragma parallel               // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always        // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c     // expected-warning {{this pragma must immediately precede a statement}}
int gVar;
double gDbl;
int gArr[10];
double gDblArr[10][40];
St1 gSt1[10];
St1 gStSt1[10][30];
int main(int argc, char **argv) {
  int lVar;
  double lDbl;
  int lArr[10];
  double lDblArr[10][40];
  St1 lSt1[10];
  St1 lStSt1[10][30];
#pragma parallel
#pragma parallel always
#pragma parallel always assert
#pragma parallel fgsdswf      // expected-error {{invalid parallel pragma}}
#pragma parallel private(a.rrr), lastprivate(a.rrr), firstprivate(a.rrr), private(a.rrr)  // expected-error 4 {{only simple variables or array expressions are allowed in #pragma parallel}}
#pragma parallel private(a.rrr) // expected-error {{only simple variables or array expressions are allowed in #pragma parallel}}
#pragma parallel private(UnknownVar) // expected-error {{use of undeclared identifier 'UnknownVar'}}
#pragma parallel private(gArr[1]), lastprivate(gArr[1]), firstprivate(gArr[1]), private(gArr[1])  // expected-error {{variable 'gArr' in firstprivate or lastprivate clause may not be in private clause}}
#pragma parallel private(gVar, lVar), private (a, lArr, lDblArr[20])
#pragma parallel private gVar, lVar), private a, lArr, lDblArr[20] // expected-error {{invalid parallel pragma}}
#pragma parallel lastprivate(gVar, lVar), firstprivate (a, lArr, lDblArr[20])
#pragma parallel lastprivate(lSt1[3], lStSt1[40]), firstprivate (gSt1, gStSt1)
  return (test(argc));
}

void asasas(char **argv) {
  ;
}
