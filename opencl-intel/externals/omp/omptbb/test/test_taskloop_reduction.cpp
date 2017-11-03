#include <iostream>
#include <atomic>
#include <stdexcept>

void enforce(bool b, const std::string& msg = std::string()) {
    if (!b)
        throw std::runtime_error(msg);
}

int main() {
    const int N1 = 100, N2 = 20;
    int k1 = 0, k2 = 0, k3 = N1 / 2;
    int x = 1;
    std::atomic<int> iters;
    iters = 0;
#pragma omp taskloop shared(iters) reduction(+:k1,k2) reduction(-:k3)
    for (int i = 0; i < N1; ++i) {
        k1 += 1;
        --k3;
        if (i == N1 / 2) {
#pragma omp taskloop shared(iters) reduction(*:x)
            for (int j = 0; j < N2; ++j) {
                x *= 2;
                ++iters;
            }
            k2 += x;
        } else
            k2 += 2;
        ++iters;
    }
    enforce(k1 == N1);
    enforce(k2 == (N1-1)*2 + (1 << N2));
    enforce(k3 == -N1 / 2);
    enforce(iters == N1+N2);
    std::cout << "Done" << std::endl;
    return 0;
}
