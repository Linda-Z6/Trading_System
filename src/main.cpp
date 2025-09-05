#include <databento/dbn.hpp>
#include <databento/historical.hpp>
#include <databento/symbol_map.hpp>
#include <databento/historical.hpp>
#include <databento/mbo_msg.hpp>
#include <iostream>

using namespace databento;

// int main() {
//   auto client =
//       Historical::Builder().SetKey("db-MgqK9Dbf5RtHSscTrjqSNeewkamW6").Build();
//   TsSymbolMap symbol_map;
//   auto decode_symbols = [&symbol_map](const Metadata& metadata) {
//     symbol_map = metadata.CreateSymbolMap();
//   };
//   auto print_trades = [&symbol_map](const Record& record) {
//     const auto& trade_msg = record.Get<TradeMsg>();
//     std::cout << "Received trade for "
//               << symbol_map.At(trade_msg) << ": " << trade_msg
//               << '\n';
//     return KeepGoing::Continue;
//   };
//   client.TimeseriesGetRange(
//       "GLBX.MDP3", {"2022-06-10T14:30", "2022-06-10T14:40"},
//       {"ESM2", "NQZ2"}, Schema::Trades, SType::RawSymbol,
//       SType::InstrumentId, {}, decode_symbols, print_trades);
// }


#include "config.h"
#include "feed_handler.cpp"   // (for quick testing; better to use feed_handler.h)

int main() {
    try {
        FeedHandler engine(DATABENTO_API_KEY);
        engine.fetch_mbo("GLBX.MDP3", "ESH3",
                         "2022-10-28T20:30:00",
                         "2022-10-28T20:30:10");
        engine.print_mbo_sample();
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
