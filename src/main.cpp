#include "AetherEngine.hpp"
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>

void stressWorker(AetherEngine& db, int thread_id, int total_ops) {
    for (int i = 0; i < total_ops; ++i) {
        std::string key = "th_" + std::to_string(thread_id) + "_k_" + std::to_string(i % 500);
        std::string val = "payload_" + std::to_string(i);

        db.put(key, val);
        auto res = db.get(key);
        if (i % 200 == 0) {
            db.del(key);
        }
    }
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "   AetherDB: Multi-Threaded High-Throughput Benchmark  \n";
    std::cout << "=======================================================\n";

    const size_t CACHE_SIZE = 10000;
    const int THREAD_COUNT = 8;
    const int OPS_PER_THREAD = 12500;
    const std::string BENCH_LOG = "bench_wal.log";

    if (std::filesystem::exists(BENCH_LOG)) std::filesystem::remove(BENCH_LOG);

    AetherEngine db(CACHE_SIZE, BENCH_LOG);

    std::cout << "[INFO] Spawning " << THREAD_COUNT << " concurrent worker threads...\n";
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> pool;
    for (int t = 0; t < THREAD_COUNT; ++t) {
        pool.emplace_back(stressWorker, std::ref(db), t, OPS_PER_THREAD);
    }

    for (auto& th : pool) {
        th.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    double total_ops = THREAD_COUNT * OPS_PER_THREAD;
    double throughput = (total_ops / (elapsed.count() / 1000.0));

    std::cout << "[SUCCESS] Completed " << total_ops << " operations in " << elapsed.count() << " ms\n";
    std::cout << "[METRIC] Throughput: " << static_cast<uint64_t>(throughput) << " ops/second\n";

    std::filesystem::remove(BENCH_LOG);
    return 0;
}
