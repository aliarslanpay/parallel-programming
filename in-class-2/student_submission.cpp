#include "Utility.h"
#include <thread>
#include <mutex>


#define PRINT_TIME 1

int task_id = 0;
std::thread threads[THREAD_NUM];
std::mutex mutex;

void compute_vel(unsigned int pos, double current_vel, double *target_vel)
{

    double x = std::fmod(std::pow(current_vel, pos % NUM_DATAPOINTS), MAX_X);
    /*
     * You do not have to modify or understand this code.
     * But in your free time, if you manage to find an optimization to this algorithm (maybe another algorithm?), 
     * you will get our personal appreciation.
     * This algorithm is not relevant for the exam and is not part of the course material. 
     */
    double p[NUM_DATAPOINTS][NUM_DATAPOINTS];

    for (int i = 0; i < NUM_DATAPOINTS; i++)
    {
        p[i][0] = datapoints[i][1];
    }

    for (int k = 1; k < NUM_DATAPOINTS; k++)
    {
        for (int i = 0; i < NUM_DATAPOINTS - k; i++)
        {
            p[i][k] = p[i][k - 1] + ((x - datapoints[i][0]) / (datapoints[i + k][0] - datapoints[i][0])) * (p[i + 1][k - 1] - p[i][k - 1]);
        }
    }

    *target_vel = std::fmod(p[0][NUM_DATAPOINTS - 1], MAX_VELO);
}

void update_rock_pos(unsigned int rock_id, unsigned int local_rocks_pos[][2], double local_rocks_vel[][2])
{
    unsigned int &rock_r = local_rocks_pos[rock_id][0];
    unsigned int &rock_c = local_rocks_pos[rock_id][1];
    double &rock_vr = local_rocks_vel[rock_id][0];
    double &rock_vc = local_rocks_vel[rock_id][1];
    compute_vel(rock_r, rock_vr, &rock_vr);
    compute_vel(rock_c, rock_vc, &rock_vc);
    double tmp = rock_r + rock_vr;

    rock_r = (unsigned int)((long)tmp % MAP_SIZE);

    tmp = rock_c + rock_vc;
    rock_c = (unsigned int)((long)tmp % MAP_SIZE);
}

void working_thread(unsigned int &buffer)
{

    // define rocks' pos and vel arrays for local thread
    unsigned int local_rocks_pos[ROCKS_NUM / THREAD_NUM][2];
    double local_rocks_vel[ROCKS_NUM / THREAD_NUM][2];
    unsigned int local_crashed_count = 0;

    int local_task_id;
    mutex.lock();
    local_task_id = task_id++;
    mutex.unlock();

    // start index for global array
    int rock_si = local_task_id * ROCKS_NUM / THREAD_NUM;

    // copy
    memcpy(local_rocks_pos, &rocks_pos[rock_si][0], ROCKS_NUM / THREAD_NUM * 2 * sizeof(unsigned int));
    memcpy(local_rocks_vel, &rocks_vel[rock_si][0], ROCKS_NUM / THREAD_NUM * 2 * sizeof(double));

    // This is the main work done by the thread
    for (unsigned int i = 0; i < MAP_SIZE; i++)
    {
        // computationally expensive tasks
        for (unsigned int k = 0; k < ROCKS_NUM / THREAD_NUM; k++)
        {
            update_rock_pos(k, local_rocks_pos, local_rocks_vel);
            auto &row = local_rocks_pos[k][0];
            auto &col = local_rocks_pos[k][1];
            if (row == i && col == i)
            {
                local_crashed_count++;
            }
        }
    }

    //return the value to the buffer 
    buffer = local_crashed_count;
}

int main()
{
    unsigned int seed = readInput();
#ifdef PRINT_TIME
    TicToc total_time;
#endif
    generate_test(rocks_pos, rocks_vel, datapoints, seed);
    unsigned int crashed_count = 0;
    //This is a buffer for receiving the output
    unsigned int buffer[THREAD_NUM];

    task_id = 0;
    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
    {
        threads[thread_id] = std::thread(working_thread, std::ref(buffer[thread_id]));
    }

    for (int thread_id = 0; thread_id < THREAD_NUM; thread_id++)
    {
        threads[thread_id].join();
        crashed_count += buffer[thread_id];
    }
        

    outputResult(crashed_count);
#ifdef PRINT_TIME
    std::cerr << "time used: " << total_time.toc() << "ms.\n";
#endif
}