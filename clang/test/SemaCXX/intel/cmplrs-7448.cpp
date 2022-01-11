// RUN: %clang_cc1 -DWIN_TEST=1 -fsyntax-only -verify %s -fms-compatibility \
// RUN: -std=c++11 -fintel-compatibility -fms-compatibility-version=19.00 \
// RUN: -fintel-compatibility-enable=MSVC2015TriviallyConstructibleBehavior
//
// RUN: %clang_cc1 -fsyntax-only -verify %s "-std=c++11"
//
// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s "-std=c++11"
// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s \
// RUN: "-std=c++11" -fms-compatibility -fms-compatibility-version=19.10
//
// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s "-std=c++11" \
// RUN: -fms-compatibility -fms-compatibility-version=19.00 \
// RUN: -fintel-compatibility-disable=MSVC2015TriviallyConstructibleBehavior
//
// RUN: %clang_cc1 -fsyntax-only -verify %s -fms-compatibility \
// RUN: -std=c++11 -fintel-compatibility -fms-compatibility-version=19.00

// expected-no-diagnostics

template<typename T, typename ...Args>
struct is_trivially_constructible {
  static const bool value = __is_trivially_constructible(T, Args...);
};
//using namespace std;

//trivially copy constructable class with destructor
class CSomeClass {
public:
	int x;

	//explicitly specify default copy constructor
	CSomeClass(const CSomeClass&) = default;

	//non-default destructor
	~CSomeClass(){}
};

#if WIN_TEST
#define PREFIX 
#else
#define PREFIX !
#endif


static_assert( PREFIX is_trivially_constructible<CSomeClass, const CSomeClass&>::value,  "not a trivially copy constructable");
static_assert( PREFIX is_trivially_constructible<CSomeClass, CSomeClass&&>::value, "not a trivially copy constructable");


