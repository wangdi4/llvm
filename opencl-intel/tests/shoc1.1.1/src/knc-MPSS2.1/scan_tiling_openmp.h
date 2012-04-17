template <class T>
__declspec(target(MIC)) void  scanTiling(T *input,  T* output, const size_t n, T* opblocksum, int ThreadCount)
{
	int i;
	int lastElement = n;
	int bufsize = ideal_buffer_size;
	// The operative principle here is to do everything with one portion of the array before moving to the next
    output[0]=input[0];  // we are doing an inclusive scan
	for (int i = 0; i <= ThreadCount; i++) 
		opblocksum[i] = 0.0;

    for(int k=0;k<n;k+=ideal_buffer_size*NUM_THREADS) {
		if (k + ideal_buffer_size*NUM_THREADS > n)  // if last buffer...
			bufsize = (int) floor((n-k)/NUM_THREADS);
		lastElement = k + bufsize * NUM_THREADS - 1;

#pragma omp parallel for shared(input,output) num_threads(NUM_THREADS)
		for(int i=0;i<NUM_THREADS;i++) {
			int offset=k + omp_get_thread_num()*bufsize;
			opblocksum[i+1] = input[offset];  // this is an inclusive scan.
			for(int j=offset+1;j<offset+bufsize;j++)
				opblocksum[i+1]+=input[j];
		}

		for(int i=1;i<=ThreadCount;i++)
			opblocksum[i] += opblocksum[i-1];

#pragma omp parallel for shared(output,opblocksum) num_threads(NUM_THREADS)
		for(int i=0;i<NUM_THREADS;i++) {
			int offset=k + omp_get_thread_num()*bufsize;
        
			output[offset] = opblocksum[i] + input[offset];
			for(int j=offset+1;j<offset+bufsize;j++) {
				output[j] = output[j-1] + input[j];
#ifdef __MIC2__
				// attempting to optimize eviction.  Doesn't seem to help.
				if (j & 0xf == 0) {
					_mm_clevict (&input[j-16],0);
					_mm_clevict (&input[j-16], 1);
					_mm_clevict (&output[j-16],0);
					_mm_clevict (&output[j-16],1);
				}
#endif
			}
		} 

		// element 0 of the sums needs the last value of the previous iteration
		opblocksum[0] = output[lastElement];
	}
 
	// We handle any remnant here.
	for (i = lastElement; i < n; i++) 
		output[i]=output[i-1]+input[i];
}
