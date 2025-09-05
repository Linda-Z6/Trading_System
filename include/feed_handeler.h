
//The functionality of the FeedHandler class
//Reads socket
// Parses message
// Updates order book
// Publishes structured event to Strategy Engine and UI / Order Book Viewer

class FeedHandler { #quick start 完成后去写
 public:
   void connect(); #databento有api去connect, connect 失败怎么处理
   void onRawMessage(const RawMsg& msg);
   void parseAndUpdate(const RawMsg& msg);

 private:
   SocketClient socket_;
   MessageParser parser_; 不一定要创建一个class 去parse，databento 提供parser api
   OrderBook book_;
};
