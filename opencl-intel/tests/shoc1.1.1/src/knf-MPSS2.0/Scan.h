// Block size

void
RunBenchmark(ResultDatabase&, OptionParser&);

template <class T>
__declspec(target(mic))
void scanArray(T* , T* , const size_t);

template <class T>
bool scanCPU(T*, T* , T* , const size_t );

template <class T>
void RunTest(string , ResultDatabase &, OptionParser &);

