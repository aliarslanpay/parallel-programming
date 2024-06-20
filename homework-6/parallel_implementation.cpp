#include "dgemm.h"
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>

#define ENABLE_MEASUREMENT true

#if ENABLE_MEASUREMENT
#include <chrono>
#endif

void dgemm(float alpha, const float *a, const float *b, float beta, float *c) {
    // Vector size for AVX
    const int vector_size = 8; // AVX processes 8 floats at a time

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            __m256 c_vec = _mm256_setzero_ps();

            int k = 0;
            for (; k <= MATRIX_SIZE - vector_size; k += vector_size) {
                // Load A[i][k:k+7]
                __m256 a_vec = _mm256_loadu_ps(a + i*MATRIX_SIZE + k);

                // Load B[k:k+7][j]
                __m256 b_vec = _mm256_loadu_ps(b + j*MATRIX_SIZE + k);

                // Compute alpha * a_vec * b_vec
                __m256 prod = _mm256_mul_ps(a_vec, b_vec);
                prod = _mm256_mul_ps(prod, _mm256_set1_ps(alpha));

                // Accumulate into c_vec
                c_vec = _mm256_add_ps(c_vec, prod);
            }

            // Horizontal add to sum the elements in the vector
            float sum = 0.0f;
            for (int i = 0; i < vector_size; ++i) {
                sum += c_vec[i];
            }

            // Handle remaining elements
            for (; k < MATRIX_SIZE; ++k) {
                sum += alpha * a[i * MATRIX_SIZE + k] * b[j * MATRIX_SIZE + k];
            }

            // Update C[i][j] with the accumulated sum
            c[i * MATRIX_SIZE + j] = beta * c[i * MATRIX_SIZE + j] + sum;
        }
    }
}

int main(int, char **) {
    float alpha, beta;

    // mem allocations
    size_t mem_size = MATRIX_SIZE * MATRIX_SIZE * sizeof(float);
    auto a = (float *) malloc(mem_size);
    auto b = (float *) malloc(mem_size);
    auto c = (float *) malloc(mem_size);

    // check if allocated
    if (nullptr == a || nullptr == b || nullptr == c) {
        printf("Memory allocation failed\n");
        if (nullptr != a) free(a);
        if (nullptr != b) free(b);
        if (nullptr != c) free(c);
        return 0;
    }

    generateProblemFromInput(alpha, a, b, beta, c);

#if ENABLE_MEASUREMENT
    auto start_time = std::chrono::high_resolution_clock::now();
#endif

    std::cerr << "Launching dgemm step." << std::endl;
    // matrix-multiplication
    dgemm(alpha, a, b, beta, c);

    outputSolution(c);

#if ENABLE_MEASUREMENT
    auto stop_time = std::chrono::high_resolution_clock::now();
    auto time_in_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
    std::cout << std::dec << "Solve Problem time: " << time_in_milliseconds << " ms" << std::endl;
#endif

    free(a);
    free(b);
    free(c);

    return EXIT_SUCCESS;
}
