
long func(long n, long *A, long *B) {

    long sum = 1;
    for(long i=0; i<n ; i++) {
        sum *= 7;
        A[i] = sum;
    }
    B[4] = 10;

    return sum;


}
