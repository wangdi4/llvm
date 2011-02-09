// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LRB_PROGRAM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LRB_PROGRAM_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LRB_PROGRAM_EXPORTS
#define LRB_PROGRAM_API __declspec(dllexport)
#else
#define LRB_PROGRAM_API __declspec(dllimport)
#endif

extern LRB_PROGRAM_API int      lrb_program_prototypes_count;
extern LRB_PROGRAM_API char*    lrb_program_prototypes[];

LRB_PROGRAM_API void dot_product (const float* a[4], const float* b[4], float *c, int tid);
LRB_PROGRAM_API void hallo_world (const float* a[4], const float* b[4], float *c, int tid);
