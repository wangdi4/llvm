// RUN: %clang_cc1 -triple i386-pc-win32 -fms-extensions -fintel-ms-compatibility -fsyntax-only -verify %s

// Check char types.
typedef unsigned char       UC1; // expected-note{{previous}}
typedef unsigned __int8     UC1;
typedef signed char         UC1;  // expected-warning{{redefinition}}

typedef char                SC1;  // expected-note{{previous}}
typedef signed char         SC1;  // expected-warning{{redefinition}}
typedef __int8              SC1;
// Check short types.
typedef unsigned short      US1;  // expected-note2{{previous}}
typedef __wchar_t           US1;  // expected-warning{{redefinition}}
typedef unsigned __int16    US1;
typedef short               US1;  // expected-warning{{redefinition}}

typedef short               SS1;
typedef __int16             SS1;
// Check int types.
typedef unsigned int        UI1;  // expected-note2{{previous}}
typedef unsigned long       UI1;  // expected-warning{{redefinition}}
typedef unsigned __int32    UI1;
typedef long                UI1;  // expected-warning{{redefinition}}
// Check long long types.
typedef unsigned long long  ULL;
typedef unsigned __int64    ULL;

typedef long long           LL1;
typedef __int64             LL1;

typedef unsigned long long  LL2;  // expected-note{{previous}}
typedef __int64             LL2;  // expected-warning{{redefinition}}
// Integer vs floating type - error.
typedef enum { A }          IF1;  // expected-note{{previous}}
typedef float               IF1;  // expected-error{{redefinition}}

typedef int                 IF2;  // expected-note{{previous}}
typedef float               IF2;  // expected-error{{redefinition}}

typedef long long           IF3;  // expected-note{{previous}}
typedef double              IF3;  // expected-error{{redefinition}}
// Enum vs int, enum vs enum - OK.
typedef enum { B }          IE1;  // expected-note{{previous}}
typedef int                 IE1;  // expected-warning{{redefinition}}

typedef enum { C }          IE2;  // expected-note{{previous}}
typedef enum { D, E }       IE2;  // expected-warning{{redefinition}}
// Enum vs pointer - error.
typedef enum { F }          IE3;  // expected-note{{previous}}
typedef int               * IE3;  // expected-error{{redefinition}}
// Check pointer types.
// Poiters with different levels - error.
typedef int              ** DL1;  // expected-note{{previous}}
typedef long              * DL1;  // expected-error{{redefinition}}
// void * vs char * - OK.
typedef char              * PT1;  // expected-note{{previous}}
typedef void              * PT1;  // expected-warning{{redefinition}}

typedef unsigned char       UChar;
typedef UChar             * PT2;  // expected-note{{previous}}
typedef void              * PT2;  // expected-warning{{redefinition}}

typedef char             ** PT3;  // expected-note{{previous}}
typedef void             ** PT3;  // expected-warning{{redefinition}}
// void * vs other pointer - error.
typedef int               * PT4;  // expected-note{{previous}}
typedef void              * PT4;  // expected-error{{redefinition}}
// Check pointee types.
typedef int               * PT5;  // expected-note{{previous}}
typedef long              * PT5;  // expected-warning{{redefinition}}

typedef unsigned int      * PT6;  // expected-note{{previous}}
typedef long              * PT6;  // expected-warning{{redefinition}}

typedef int               * PT7;  // expected-note{{previous}}
typedef float             * PT7;  // expected-error{{redefinition}}
// Different qualifiers - error.
typedef short               DQ1;  // expected-note{{previous}}
typedef const short         DQ1;  // expected-error{{redefinition}}

typedef int   * const       DQ2;  // expected-note{{previous}}
typedef long  *             DQ2;  // expected-error{{redefinition}}

typedef int               * DQ3;  // expected-note{{previous}}
typedef volatile int      * DQ3;  // expected-error{{redefinition}}
// Other than (enum | integer | pointer) type - error.
typedef struct S { int x; } OT1;  // expected-note{{previous}}
typedef int                 OT1;  // expected-error{{redefinition}}

typedef float               OT2;  // expected-note{{previous}}
typedef double              OT2;  // expected-error{{redefinition}}

typedef int                 DS1;  // Different scopes - OK.

int main() {
  typedef long              DS1;  // Different scopes - OK.
  {
    typedef long            DS2;  // Different scopes - OK.
  }
  typedef int               DS2;  // Different scopes - OK.
  // Both are in the same non-file scope - error.
  typedef int               LS1;  // expected-note{{previous}}
  typedef long              LS1;  // expected-error{{redefinition}}
}
