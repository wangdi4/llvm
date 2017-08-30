// RUN: %clang_cc1 -DWIN_TEST=1 -fsyntax-only -verify %s "-fms-compatibility" "-std=c++11"
// RUN: %clang_cc1 -fsyntax-only -verify %s "-std=c++11"
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


