#include <cstring>
#include <immintrin.h>

#include "Utility.h"


#define PRINT_TIME 1

/*
 * Dense matrix vector multiplication
 */
void dmv(float *mat, float *in_vec, float *out_vec, size_t mat_size)
{
    float sum = 0;
    for (size_t row = 0; row < mat_size; row++)
    {
        out_vec[row] = 0;
        __m256 partial_sum = _mm256_set1_ps(0);
        float partial_sum_array[8] = {0, 0, 0, 0, 0,0,0,0};

        for (size_t col = 0; col < mat_size; col += 8)
        {
            // Load 8 float values from the matrix row into a __m256 type
            __m256 mat_values = _mm256_loadu_ps(&mat[row * mat_size + col]);

            // Load 8 float values from the vector into a __m256 type
            __m256 vec_values = _mm256_loadu_ps(&in_vec[col]);

            // Perform element-wise product between mat_values and vec_values
            __m256 product = _mm256_mul_ps(mat_values, vec_values);

            // Add product to partial_sum
            partial_sum = _mm256_add_ps(partial_sum, product);
        }

        // Store the partial_sum into partial_sum_array
        _mm256_storeu_ps(partial_sum_array, partial_sum);

        for (int i = 0; i < 8; i++)
        {
            out_vec[row] += partial_sum_array[i];
        }
        sum += out_vec[row];
    }
    for (size_t row = 0; row < mat_size; row++)
    {
        out_vec[row] /= sum;
    }
}

int main()
{
    unsigned int seed = readInput();

    // allocate aligned memory
    float *mat = (float *)_mm_malloc(sizeof(float) * MAT_SIZE * MAT_SIZE, 32);
    float *in_vec = (float *)_mm_malloc(sizeof(float) * MAT_SIZE, 32);
    float *out_vec = (float *)_mm_malloc(sizeof(float) * MAT_SIZE, 32);

#ifdef PRINT_TIME
    TicToc total_time;
#endif
    generate_test(seed, mat, in_vec);

    for (int i = 0; i < ITER_NUM; i++)
    {
        dmv(mat, in_vec, out_vec, MAT_SIZE);
        out_vec[i] = 1.0/double(i + 1);
        memcpy(in_vec, out_vec, sizeof(float) * MAT_SIZE);
    }

    outputResult(seed, out_vec);
#ifdef PRINT_TIME
    std::cerr << "time used: " << total_time.toc() << "ms.\n";
#endif
}
