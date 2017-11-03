#include <iostream>
#include <mutex>
#include <atomic>

void enforce(bool b, const std::string& msg = std::string()) {
    if (!b)
        throw std::runtime_error(msg);
}

int main() {
    std::atomic<int> iters;
    int N = 10000;
    iters = 0;
#pragma omp taskloop shared(iters)
    for (int i=0; i<N; ++i) {
        ++iters;
    }
    enforce(iters == N);
    std::cout << "Done" << std::endl;
    return 0;
}
