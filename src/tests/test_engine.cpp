#include "AetherEngine.hpp"
#include <cassert>
#include <filesystem>

void testBasicOperations() {
    std::string test_log = "test_basic.log";
    if (std::filesystem::exists(test_log)) std::filesystem::remove(test_log);

    {
        AetherEngine engine(2, test_log);
        assert(engine.put("k1", "v1") == true);
        assert(engine.put("k2", "v2") == true);
        assert(engine.get("k1").value() == "v1");
        assert(engine.get("k2").value() == "v2");
        assert(engine.size() == 2);
    }
    std::filesystem::remove(test_log);
    std::cout << "[PASS] Basic Operations Test Passed.\n";
}

void testLRUEviction() {
    std::string test_log = "test_lru.log";
    if (std::filesystem::exists(test_log)) std::filesystem::remove(test_log);

    {
        AetherEngine engine(2, test_log);
        engine.put("k1", "v1");
        engine.put("k2", "v2");
        engine.put("k3", "v3");

        assert(engine.get("k1").has_value() == false);
        assert(engine.get("k2").value() == "v2");
        assert(engine.get("k3").value() == "v3");
    }
    std::filesystem::remove(test_log);
    std::cout << "[PASS] LRU Eviction Test Passed.\n";
}

void testCrashRecovery() {
    std::string test_log = "test_recovery.log";
    if (std::filesystem::exists(test_log)) std::filesystem::remove(test_log);

    {
        AetherEngine engine(10, test_log);
        engine.put("persist_key", "persist_value");
        engine.put("delete_key", "delete_value");
        engine.del("delete_key");
    }

    {
        AetherEngine recoveredEngine(10, test_log);
        assert(recoveredEngine.get("persist_key").value() == "persist_value");
        assert(recoveredEngine.get("delete_key").has_value() == false);
    }
    std::filesystem::remove(test_log);
    std::cout << "[PASS] Crash Recovery via WAL Test Passed.\n";
}

int main() {
    std::cout << "Starting AetherEngine Test Suite...\n";
    testBasicOperations();
    testLRUEviction();
    testCrashRecovery();
    std::cout << "All automated tests executed successfully!\n";
    return 0;
}
