/* protobuf config.h for MSVC.  On other platforms, this is generated
 * automatically by autoheader / autoconf / configure. */

#ifdef _MSC_VER

/* the location of <hash_map> */
#define HASH_MAP_H <hash_map>

/* the namespace of hash_map/hash_set */
// Apparently Microsoft decided to move hash_map *back* to the std namespace
// in MSVC 2010:
//   http://blogs.msdn.com/vcblog/archive/2009/05/25/stl-breaking-changes-in-visual-studio-2010-beta-1.aspx
// TODO(kenton):  Use unordered_map instead, which is available in MSVC 2010.
#if _MSC_VER < 1310 || _MSC_VER >= 1600
#define HASH_NAMESPACE std
#else
#define HASH_NAMESPACE stdext
#endif

/* the location of <hash_set> */
#define HASH_SET_H <hash_set>

/* define if the compiler has hash_map */
#define HAVE_HASH_MAP 1

/* define if the compiler has hash_set */
#define HAVE_HASH_SET 1

#else /* no _MSC_VER */

 
/* the name of <hash_map> */
#define HASH_MAP_CLASS unordered_map

/* the location of <unordered_map> or <hash_map> */
#define HASH_MAP_H <tr1/unordered_map>

/* the namespace of hash_map/hash_set */
#define HASH_NAMESPACE std::tr1

/* the name of <hash_set> */
#define HASH_SET_CLASS unordered_set

/* the location of <unordered_set> or <hash_set> */
#define HASH_SET_H <tr1/unordered_set>

#define HAVE_HASH_MAP 1
#define HAVE_HASH_SET 1


#endif
 
