#include <string>
#include <deque>
#include <future>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>
#include <vector>
#include <iostream>

#include "Utility.h"

#define ENABLE_MEASUREMENT true

#if ENABLE_MEASUREMENT
#include <chrono>
#endif

struct Problem {
    Sha1Hash sha1_hash;
    int problemNum;
};

//thread safe queue
class ProblemQueue {
public:
    ProblemQueue(size_t capacity) : problem_queue(capacity) {}

    void push(Problem problem) {
        while (!problem_queue.push(problem)) {
            // wait if the queue is full
        }
    }

    bool pop(Problem& problem) {
        return problem_queue.pop(problem);
    }

    bool empty() {
        return problem_queue.empty();
    }

private:
    boost::lockfree::queue<Problem> problem_queue;
};

ProblemQueue problemQueue(10000); // Set the initial capacity of the queue

// generate numProblems sha1 hashes with leadingZerosProblem leading zero bits
// This method is intentionally compute intense so you can already start working on solving
// problems while more problems are generated
void generateProblem(int seed, int numProblems, int leadingZerosProblem) {
    srand(seed + 1);

    for (int i = 0; i < numProblems; i++) {
        std::string base = std::to_string(rand()) + std::to_string(rand());
        Sha1Hash hash = Utility::sha1(base);
        do {
            // we keep hashing ourself until we find the desired amount of leading zeros
            hash = Utility::sha1(hash);
        } while (Utility::count_leading_zero_bits(hash) < leadingZerosProblem);
        problemQueue.push(Problem{ hash, i });
    }
}

// This method repeatedly hashes itself until the required amount of leading zero bits is found
Sha1Hash findSolutionHash(Sha1Hash hash, int leadingZerosSolution) {
    do {
        // we keep hashing ourself until we find the desired amount of leading zeros
        hash = Utility::sha1(hash);
    } while (Utility::count_leading_zero_bits(hash) < leadingZerosSolution);

    return hash;
}

int main(int argc, char* argv[]) {
    int leadingZerosProblem = 8;
    int leadingZerosSolution = 11;
    int numProblems = 10000;

    Utility::parse_input(numProblems, leadingZerosProblem, leadingZerosSolution, argc, argv);
    std::vector<Sha1Hash> solutionHashes(numProblems);

    unsigned int seed = Utility::readInput();

#if ENABLE_MEASUREMENT
    auto generation_start = std::chrono::high_resolution_clock::now();
#endif

    std::future<void> problem_generator = std::async(std::launch::async, generateProblem, seed, numProblems, leadingZerosProblem);

#if ENABLE_MEASUREMENT
    auto generation_stop = std::chrono::high_resolution_clock::now();
    auto time_for_generation = std::chrono::duration_cast<std::chrono::microseconds>(generation_stop - generation_start).count();
    std::cout << std::dec << "Generate Problem time: " << time_for_generation << " microseconds" << std::endl;

    auto solve_start = std::chrono::high_resolution_clock::now();
#endif

    auto numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;

    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([&]() {
            while (true) {
                if (problemQueue.empty()) {
                    break; // Exit if queue is empty
                }
                Problem p;
                problemQueue.pop(p);
                solutionHashes[p.problemNum] = findSolutionHash(p.sha1_hash, leadingZerosSolution);
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

#if ENABLE_MEASUREMENT
    auto solve_stop = std::chrono::high_resolution_clock::now();
    auto time_for_solve = std::chrono::duration_cast<std::chrono::microseconds>(solve_stop - solve_start).count();
    std::cout << std::dec << "Solve Problem time: " << time_for_solve << " microseconds" << std::endl;
#endif


    Sha1Hash solution;
    // Guarantee initial solution hash data is zero
    memset(solution.data, 0, SHA1_SIZE);
    // This doesn't need parallelization. It's negligibly fast
    for (int i = 0; i < numProblems; i++) {
        solution = Utility::sha1(solution, solutionHashes[i]);
    }

    Utility::printHash(solution);
    printf("DONE\n");

    return 0;
}