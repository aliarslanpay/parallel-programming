#include "life.h"
#include "Utility.h"
#include "VideoOutput.h"
#include <mpi.h>
#include <iostream>
#include <random>

#define ENABLE_MEASUREMENT false

#if ENABLE_MEASUREMENT
#include <chrono>
#include <iostream>
#endif

std::minstd_rand my_randomEngine;
uint_fast32_t my_cacheValue;
uint_fast32_t my_bitMask = 0;

void my_seedGenerator(unsigned long long seed)
{
    my_randomEngine = std::minstd_rand(seed);
}

inline bool generateBit()
{
    if (!my_bitMask)
    {
        my_cacheValue = my_randomEngine();
        my_bitMask = 1;
    }
    bool value = my_cacheValue & my_bitMask;
    my_bitMask = my_bitMask << 1;
    return value;
}

void evolve(ProblemData &problemData, int rank, int size)
{
    int workload_rows = GRID_SIZE / size;
    int workload_rest = GRID_SIZE % size;

    auto &grid = *problemData.readGrid;
    auto &writeGrid = *problemData.writeGrid;

    int flag = ((rank + 1) * workload_rows);

    if (rank == size - 1)
    {
        flag += workload_rest - 1;
    }

    for (int i = rank == 0 ? 1 : rank * workload_rows; i < flag; i++)
    {
        for (int j = 1; j < GRID_SIZE - 1; j++)
        {
            int sum = grid[i - 1][j - 1] + grid[i - 1][j] + grid[i - 1][j + 1] +
                      grid[i][j - 1] + grid[i][j + 1] +
                      grid[i + 1][j - 1] + grid[i + 1][j] + grid[i + 1][j + 1];

            writeGrid[i][j] = (sum == 3 || (grid[i][j] && sum == 2));
        }
    }
}

void copy_edges(bool (&grid)[GRID_SIZE][GRID_SIZE], int rank, int size)
{
    int workload_rows = GRID_SIZE / size;
    int workload_rest = GRID_SIZE % size;

    int prev_proc = (rank - 1 + size) % size;  // circular wrapping
    int next_proc = (rank + 1) % size;

    if (rank != size - 1)
    {
        for (int i = rank == 0 ? 1 : rank * workload_rows; i < ((rank + 1) * workload_rows); i++)
        {
            grid[i][0] = grid[i][GRID_SIZE - 2];
            grid[i][GRID_SIZE - 1] = grid[i][1];
        }

        int offset;
        if (rank == 0)
            offset = 1;
        else
            offset = 0;

        MPI_Send(grid[rank * workload_rows + offset], GRID_SIZE, MPI_CXX_BOOL, prev_proc, 0, MPI_COMM_WORLD);
        MPI_Recv(grid[(rank + 1) * workload_rows], GRID_SIZE, MPI_CXX_BOOL, next_proc, 0, MPI_COMM_WORLD, nullptr);
        MPI_Send(grid[(rank + 1) * workload_rows - 1], GRID_SIZE, MPI_CXX_BOOL, next_proc, 0, MPI_COMM_WORLD);
        MPI_Recv(grid[rank * workload_rows + offset - 1], GRID_SIZE, MPI_CXX_BOOL, prev_proc, 0, MPI_COMM_WORLD, nullptr);
    }
    else
    {
        for (int i = rank * workload_rows; i < ((rank + 1) * workload_rows + workload_rest) - 1; i++)
        {
            grid[i][0] = grid[i][GRID_SIZE - 2];
            grid[i][GRID_SIZE - 1] = grid[i][1];
        }

        MPI_Recv(grid[(rank + 1) * workload_rows + workload_rest - 1], GRID_SIZE, MPI_CXX_BOOL, next_proc, 0, MPI_COMM_WORLD, nullptr);
        MPI_Send(grid[rank * workload_rows], GRID_SIZE, MPI_CXX_BOOL, prev_proc, 0, MPI_COMM_WORLD);
        MPI_Recv(grid[rank * workload_rows - 1], GRID_SIZE, MPI_CXX_BOOL, prev_proc, 0, MPI_COMM_WORLD, nullptr);
        MPI_Send(grid[(rank + 1) * workload_rows + workload_rest - 2], GRID_SIZE, MPI_CXX_BOOL, next_proc, 0, MPI_COMM_WORLD);
    }

    // Fix corners
    if (rank == 0)
    {
        MPI_Recv(&grid[0][0], GRID_SIZE, MPI_CXX_BOOL, size - 1, 0, MPI_COMM_WORLD, nullptr);
        MPI_Send(&grid[1][1], 1, MPI_CXX_BOOL, size - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&grid[0][GRID_SIZE - 1], GRID_SIZE, MPI_CXX_BOOL, size - 1, 0, MPI_COMM_WORLD, nullptr);
        MPI_Send(&grid[1][GRID_SIZE - 2], 1, MPI_CXX_BOOL, size - 1, 0, MPI_COMM_WORLD);
    }
    if (rank == size - 1)
    {
        MPI_Send(&grid[GRID_SIZE - 2][GRID_SIZE - 2], 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&grid[GRID_SIZE - 1][GRID_SIZE - 1], GRID_SIZE, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD, nullptr);
        MPI_Send(&grid[GRID_SIZE - 2][1], 1, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&grid[GRID_SIZE - 1][0], GRID_SIZE, MPI_CXX_BOOL, 0, 0, MPI_COMM_WORLD, nullptr);
    }
}

int readProblemFromInput(ProblemData &data)
{
    auto &grid = *data.readGrid;

    unsigned int seed = 0;
    std::cout << "READY" << std::endl;
    std::cin >> seed;

    std::cout << "Using seed " << seed << std::endl;
    if (seed == 0)
    {
        std::cout << "Warning: default value 0 used as seed." << std::endl;
    }

    my_seedGenerator(seed);

    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i += 1)
    {
        *(grid[0] + i) = generateBit();
    }

    return seed;
}

void generateProblem(ProblemData &data, int seed)
{
    auto &grid = *data.readGrid;
    my_seedGenerator(seed);

    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i += 1)
    {
        *(grid[0] + i) = generateBit();
    }
}

int my_countAlive(ProblemData &data, int rank, int size)
{
    int workload_rows = GRID_SIZE / size;
    int workload_rest = GRID_SIZE % size;

    int flag = ((rank + 1) * workload_rows);

    if (rank == size - 1)
    {
        flag += workload_rest - 1;
    }

    auto &grid = *data.readGrid;
    int counter = 0;
    for (int x = rank == 0 ? 1 : rank * workload_rows; x < flag; x++)
    {
        for (int y = 1; y < GRID_SIZE - 1; y++)
        {
            if (grid[x][y])
            {
                counter++;
            }
        }
    }
    return counter;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto *problemData = new ProblemData;

    int seed;
    if (rank == 0)
    {
        seed = readProblemFromInput(*problemData);
    }

#if ENABLE_MEASUREMENT
    auto start_time = std::chrono::high_resolution_clock::now();
#endif

    MPI_Bcast(&seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
    {
        generateProblem(*problemData, seed);
    }

    for (int iteration = 0; iteration < NUM_SIMULATION_STEPS; ++iteration)
    {
        {
            copy_edges(*problemData->readGrid, rank, size);

            if (iteration % SOLUTION_REPORT_INTERVAL == 0)
            {
                int res = 0;
                int local = my_countAlive(*problemData, rank, size);

                MPI_Reduce(&local, &res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

                if (rank == 0)
                {
                    std::cout << "Iteration " << iteration << ": " << res << " cells alive." << std::endl;
                }
            }

            evolve(*problemData, rank, size);
            problemData->swapGrids();
        }
    }

    int final_res = 0;
    int local = my_countAlive(*problemData, rank, size);
    MPI_Reduce(&local, &final_res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

#if ENABLE_MEASUREMENT
    auto stop_time = std::chrono::high_resolution_clock::now();
    auto time_in_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
    std::cout << std::dec << "Solve Problem time: " << time_in_milliseconds << " ms" << std::endl;
#endif

    if (rank == 0)
    {
        std::cout << "Iteration " << NUM_SIMULATION_STEPS << ": " << final_res << " cells alive." << std::endl;
        std::cout << "DONE" << std::endl;
    }

    delete problemData;
    MPI_Finalize();
    return 0;
}