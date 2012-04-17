	int i;
	output[0]=input[0];
	for (int i = 0; i <= numblocks; i++) 
		opblocksum[i] = 0.0;

#pragma omp parallel for shared(input,output) num_threads(NUM_THREADS)
    for(i=0;i<numblocks;i++) {
		int offset=i*BLOCK;
		int end=(i==numblocks-1)?(n-offset):BLOCK;
		for(int j=offset;j<offset+end;j++)
			opblocksum[i+1] += input[j];
    }
	
	opblocksum[0]=0.0;
    for(int i=1;i<=numblocks;i++)
		opblocksum[i] += opblocksum[i-1];

#pragma omp parallel for shared(output,opblocksum) num_threads(NUM_THREADS)
    for(i=0;i<numblocks;i++) {
        int offset=i*BLOCK;
        int end=(i==numblocks-1)?(n-offset):BLOCK;
        
		output[offset] = opblocksum[i] + input[offset];
		for(int j=offset+1;j<offset+end;j++)
            output[j] = output[j-1] + input[j];
    } 
