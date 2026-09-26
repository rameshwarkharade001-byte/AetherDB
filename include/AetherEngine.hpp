#ifndef AETHER_ENGINE_HPP
#define AETHER_ENGINE_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <mutex>
#include <fstream>
#include <sstream>
#include <optional>
#include <algorithm>

class AetherEngine {
private:
    struct CacheNode {
        std::string key;
        std::string value;
        CacheNode(std::string k, std::string v) : key(std::move(k)), value(std::move(v)) {}
    };

    size_t capacity_;
    std::list<CacheNode> lru_list_;
    std::unordered_map<std::string, std::list<CacheNode>::iterator> index_map_;
    
    mutable std::shared_mutex rw_lock_;
    std::ofstream wal_writer_;
    std::string wal_path_;

    bool isValidToken(const std::string& str) const {
        return !str.empty() && str.find('\n') == std::string::npos && str.find('\r') == std::string::npos;
    }

    void writeToLog(const std::string& command, const std::string& key, const std::string& value = "") {
        if (!wal_writer_.is_open()) return;
        wal_writer_ << command << " " << key;
        if (!value.empty()) wal_writer_ << " " << value;
        wal_writer_ << "\n";
        wal_writer_.flush();
    }

    void putInternal(const std::string& key, const std::string& value) {
        auto it = index_map_.find(key);
        if (it != index_map_.end()) {
            it->second->value = value;
            lru_list_.splice(lru_list_.begin(), lru_list_, it->second);
            return;
        }

        if (index_map_.size() >= capacity_) {
            const auto& lru_item = lru_list_.back();
            index_map_.erase(lru_item.key);
            lru_list_.pop_back();
        }

        lru_list_.emplace_front(key, value);
        index_map_[key] = lru_list_.begin();
    }

    bool delInternal(const std::string& key) {
        auto it = index_map_.find(key);
        if (it == index_map_.end()) return false;
        lru_list_.erase(it->second);
        index_map_.erase(it);
        return true;
    }

    void replayWAL() {
        std::ifstream wal_reader(wal_path_);
        if (!wal_reader.is_open()) return;

        std::string line;
        while (std::getline(wal_reader, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string cmd, key, val;
            if (iss >> cmd >> key) {
                if (cmd == "SET" && iss >> val) {
                    putInternal(key, val);
                } else if (cmd == "DEL") {
                    delInternal(key);
                }
            }
        }
    }

public:
    explicit AetherEngine(size_t capacity, const std::string& wal_file = "aether_wal.log")
        : capacity_(capacity), wal_path_(wal_file) {
        if (capacity_ == 0) capacity_ = 1;
        replayWAL();
        wal_writer_.open(wal_path_, std::ios::app);
    }

    ~AetherEngine() {
        if (wal_writer_.is_open()) wal_writer_.close();
    }

    AetherEngine(const AetherEngine&) = delete;
    AetherEngine& operator=(const AetherEngine&) = delete;

    bool put(const std::string& key, const std::string& value) {
        if (!isValidToken(key) || !isValidToken(value)) return false;
        std::unique_lock<std::shared_mutex> lock(rw_lock_);
        writeToLog("SET", key, value);
        putInternal(key, value);
        return true;
    }

    std::optional<std::string> get(const std::string& key) {
        if (!isValidToken(key)) return std::nullopt;
        std::shared_lock<std::shared_mutex> lock(rw_lock_);
        auto it = index_map_.find(key);
        if (it == index_map_.end()) return std::nullopt;
        return it->second->value;
    }

    bool del(const std::string& key) {
        if (!isValidToken(key)) return false;
        std::unique_lock<std::shared_mutex> lock(rw_lock_);
        if (delInternal(key)) {
            writeToLog("DEL", key);
            return true;
        }
        return false;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(rw_lock_);
        return index_map_.size();
    }
};

#endif
