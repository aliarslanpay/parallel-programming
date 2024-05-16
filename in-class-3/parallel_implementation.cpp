#include "Utility.h"
#include <omp.h>

#define PRINT_TIME

/*
 * Predicts the classes of NUM_PREDICTIONS points using k-nn.
 */
void predict(double input_data[][2], int *predicted_class, int *each_class_num)
{
    int k_nearest_neighbors[NUM_PREDICTIONS][K];

    #pragma omp parallel for schedule(dynamic, 16)
    for(int i = 0; i < NUM_PREDICTIONS; i++)
    {
        get_k_nearest_neighbors(input_data[i], k_nearest_neighbors[i]);
        int class_prediction;
        get_class_from_neighbors(k_nearest_neighbors[i], &class_prediction);

        #pragma omp critical
        {
            predicted_class[i] = class_prediction;
            each_class_num[class_prediction]++;
        }
    }
}

int main()
{
    double input_data[NUM_PREDICTIONS][2];
    int predicted_class[NUM_PREDICTIONS];
    int each_class_num[NUM_CLASSES] = {0};
    unsigned int seed = readInput();

#ifdef PRINT_TIME
    TicToc total_time;
#endif

    generate_test(seed, input_data);
    init_class_num(each_class_num);
    predict(input_data, predicted_class, each_class_num);
    outputResult(each_class_num);
#ifdef PRINT_TIME
    std::cerr << "time used: " << total_time.toc() << "ms.\n";
#endif
}