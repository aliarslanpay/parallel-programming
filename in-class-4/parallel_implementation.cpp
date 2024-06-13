#include "Utility.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "omp.h"

#define PRINT_TIME false

thread_local Integer partial_result{0};

/* the mechanism of this function
 *       b
 *     X a
 *    -----
 *
 *
 */
Integer mulInteger(const Integer &a, const Integer &b)
{
    Integer result{0};

    #pragma omp parallel
    {
        partial_result = {0};
        #pragma omp single
        {
            for (size_t i = 0; i < a.size(); i++)
            {
                #pragma omp task
                {
                    partial_result = addInteger(mulShiftedInteger(b, a[i], i), partial_result);
                }
            }
        }
        #pragma omp critical
        {
            result = addInteger(partial_result, result);
        }
    }

    return result;
}

int main()
{
    unsigned int seed = readInput();
    int problem[NUM_FACTORS];

#if PRINT_TIME
    TicToc total_time;
#endif

    generate_test(seed, problem);
    Integer a = calcProduct(problem);

    outputResult(a);
#if PRINT_TIME
    std::cerr << "time used: " << total_time.toc() << "ms.\n";
#endif
}
