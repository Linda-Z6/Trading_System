#pragma once
#include "config.h"
#include <databento/historical.hpp>
#include <databento/datetime.hpp> // UnixNanos， TimeDeltaNanos
#include <databento/enums.hpp>
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>

using namespace databento;

// using UnixNanos = std::chrono::time_point<std::chrono::system_clock,
//             std::chrono::duration<uint64_t, std::nano>>;
// using TimeDeltaNanos = std::chrono::duration<int32_t, std::nano>;
// std::string ToString(UnixNanos unix_nanos);

// --- Our own internal struct for MBO events ---
struct Mbo {
  UnixNanos ts_recv;     // ns since epoch (capture server)
  UnixNanos ts_event;    // ns since epoch (matching engine)
  TimeDeltaNanos ts_in_delta; // ns before ts_recv (send time delta)
  char     action;      // 'A','C','M','R','T','F','N'
  char     side;        // 'B','A','N'
  int64_t  price;       // price in 1e-9 units
  uint32_t size;        // quantity
  uint64_t order_id;    // venue order id
  uint8_t  flags;       // raw flag bits
  uint32_t sequence;    // venue sequence

  double price_as_double() const { return static_cast<double>(price) * 1e-9; }
};


// --- FeedHandler class ---
class FeedHandler {
private:
    Historical client;         // Databento historical client
    std::vector<Mbo> mbo_events; // buffer for MBO messages

public:
    // Constructor: initialize with API key
    explicit FeedHandler(const string& api_key)
      : client(Historical::Builder().SetKey(api_key).Build()) {}

    // Callback: store parsed MBO into our vector
    KeepGoing store_mbo_record(const databento::Record& record) {
        const auto& msg = record.Get<databento::MboMsg>();

        Mbo one_record{
            msg.ts_recv,
            msg.hd.ts_event,
            msg.ts_in_delta,
            static_cast<char>(msg.action),
            static_cast<char>(msg.side),
            msg.price,
            msg.size,
            msg.order_id,
            static_cast<uint8_t>(msg.flags),
            msg.sequence
        };

        mbo_events.push_back(one_record);
        return KeepGoing::Continue;
    }

    // Fetch MBO data for a given instrument/time range
    void fetch_mbo(const string& dataset,
                const string& symbol,
                const string& start,
                const string& end) {
        // limit = 0 → no row cap
        uint64_t limit = 0;
        TsSymbolMap symbol_map;
        auto decode_symbols = [&symbol_map](const Metadata& metadata) {
            symbol_map = metadata.CreateSymbolMap();
        };

        client.TimeseriesGetRange(
            dataset,                           // dataset name e.g. "GLBX.MDP3"
            databento::DateTimeRange<string>{start, end}, // ISO8601 start/end
            {symbol},                          // list of symbols
            Schema::Mbo,                       // request MBO schema
            SType::RawSymbol,                  // stype_in (what we pass in symbols as)
            SType::InstrumentId,               // stype_out (how SDK normalizes internally)
            limit,
            decode_symbols,
            [this](const databento::Record& rec) {
                return this->store_mbo_record(rec);
            }
        );

    }

    // Print first N=5 buffered MBO messages
    void print_mbo_sample(size_t n = 5) const {
        std::cout << "=== MBO Sample ===\n";
        for (size_t i = 0; i < std::min(n, mbo_events.size()); ++i) {
            const auto& msg = mbo_events[i];
            std::cout << " Action=" << msg.action
                      << " Side=" << msg.side
                      << " Px=" << msg.price_as_double()
                      << " Sz=" << msg.size
                      << " OrderId=" << msg.order_id
                      << " Seq=" << msg.sequence
                      << "\n";
        }
    }

    // Access buffer externally
    const vector<Mbo>& get_buffer() const { return mbo_events; }
};



int main() {
    try {
        FeedHandler engine(DATABENTO_API_KEY);
        engine.fetch_mbo("GLBX.MDP3", "ESH3",
                         "2022-10-28T20:30:00",
                         "2022-10-28T20:30:10");
        engine.print_mbo_sample();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
