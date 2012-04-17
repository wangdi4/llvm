void
RunBenchmark(ResultDatabase&, OptionParser&);

template <class T>
__declspec(target(mic))
void radixoffset(T*, T*, const size_t, const unsigned int);

template <class T>
__declspec(target(mic))
void rearrange(T**,T**,T**,T**,T**,const size_t);

template <class T>
__declspec(target(mic))
void scanArray(T* , T* , const size_t);


template <class T>
__declspec(target(mic))
extern void sortKernel(T* , T* , T*, T*, const size_t);


template <class T>
bool verifyResult(T* , T*, const size_t );

template <class T>
void RunTest(string , ResultDatabase &, OptionParser &);

//#define BLOCK 512
#define BITS 32

