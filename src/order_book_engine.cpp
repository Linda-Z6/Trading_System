#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

struct LevelData {
  int64_t total_size = 0;                 // sum of sizes at this price
  int     order_count = 0;                // number of active orders at this price
  std::unordered_set<uint64_t> order_ids; // resting order ids here
};

struct OrderData {
  char    side;    // 'B' (bid) or 'A' (ask)
  int64_t price;   // 1 unit = 1e-9
  int64_t size;    // remaining qty
};

struct BookStats {
  int64_t total_size = 0;
  int     total_count = 0;
};


// -------------------------------
// Order Book Class (header-only)
// -------------------------------
class OrderBook {
    private:
    // price → level (map for price ordering)
    std::map<int64_t, LevelData> bids;
    std::map<int64_t, LevelData> asks;

    // order_id → order details
    std::unordered_map<uint64_t, OrderData> orders;

    // stats
    BookStats bid_stats;
    BookStats ask_stats;
    
public:
    // Add a new order
    void add(uint64_t order_id, char side, int64_t price, int64_t size) {
        OrderData od{side, price, size};
        orders[order_id] = od;

        auto& book = (side == 'B') ? bids : asks;
        auto& stats = (side == 'B') ? bid_stats : ask_stats;
        LevelData& lvl = book[price];

        lvl.total_size += size;
        lvl.order_count += 1;
        lvl.order_ids.insert(order_id);

        stats.total_size += size;
        stats.total_count += 1;
    }

    // Cancel an order (full cancel)
    void cancel(uint64_t order_id) {
        auto it = orders.find(order_id);
        if (it == orders.end()) return;

        OrderData od = it->second;
        auto& book = (od.side == 'B') ? bids : asks;
        auto& stats = (od.side == 'B') ? bid_stats : ask_stats;

        auto& lvl = book[od.price];
        lvl.total_size -= od.size;
        lvl.order_count -= 1;
        lvl.order_ids.erase(order_id);

        stats.total_size -= od.size;
        stats.total_count -= 1;

        if (lvl.order_count == 0) {
            book.erase(od.price);
        }
        orders.erase(it);
    }

    // Modify order size (increase/decrease)
    void modify(uint64_t order_id, int64_t new_size) {
        auto it = orders.find(order_id);
        if (it == orders.end()) return;

        OrderData& od = it->second;
        auto& book = (od.side == 'B') ? bids : asks;
        auto& stats = (od.side == 'B') ? bid_stats : ask_stats;
        auto& lvl = book[od.price];

        int64_t delta = new_size - od.size;
        od.size = new_size;

        lvl.total_size += delta;
        stats.total_size += delta;

        if (od.size <= 0) {
            cancel(order_id);
        }
    }

    // Fill (partial or full execution)
    void fill(uint64_t order_id, int64_t fill_size) {
        auto it = orders.find(order_id);
        if (it == orders.end()) return;

        OrderData& od = it->second;
        auto& book = (od.side == 'B') ? bids : asks;
        auto& stats = (od.side == 'B') ? bid_stats : ask_stats;
        auto& lvl = book[od.price];

        int64_t take = (fill_size >= od.size) ? od.size : fill_size;

        od.size -= take;
        lvl.total_size -= take;
        stats.total_size -= take;

        if (od.size <= 0) {
            lvl.order_count -= 1;
            lvl.order_ids.erase(order_id);
            stats.total_count -= 1;
            orders.erase(it);
            if (lvl.order_count == 0) book.erase(od.price);
        }
    }

    // Clear the entire book for one side or both
    void clear(char side = 'N') {
        if (side == 'B' || side == 'N') {
            bids.clear();
            bid_stats = {};
        }
        if (side == 'A' || side == 'N') {
            asks.clear();
            ask_stats = {};
        }
        if (side == 'N') {
            orders.clear();
        } else {
            for (auto it = orders.begin(); it != orders.end();) {
                if (it->second.side == side) {
                    it = orders.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    // Debug print top of book
    void print_top() const {
        if (!bids.empty()) {
            auto best_bid = bids.rbegin(); // highest price
            std::cout << "Best Bid: " << best_bid->first / 1e9
                      << " x " << best_bid->second.total_size << "\n";
        }
        if (!asks.empty()) {
            auto best_ask = asks.begin(); // lowest price
            std::cout << "Best Ask: " << best_ask->first / 1e9
                      << " x " << best_ask->second.total_size << "\n";
        }
    }

};