
long func(long n, long *A, long *B) {
    long sum = 0;
    long i=0;
    while (sum < n && i < n) {
        sum += A[++i];
        B[i] = i;
    }

    return sum;

}
