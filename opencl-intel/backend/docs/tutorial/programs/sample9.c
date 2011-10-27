
long func(long n, long *A, long *B) {

    int sum = 0;
    long i=0;
    do {
            sum += B[i++];
    } while (i < n);

    return sum;

}
