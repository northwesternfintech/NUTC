import { ArrowDownOnSquareIcon } from "@heroicons/react/20/solid";
import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import dark from "react-syntax-highlighter/dist/esm/styles/prism/coldark-dark";

const pythonCodeString = `from enum import Enum
class Side(Enum):
    BUY = 0
    SELL = 1

class Ticker(Enum):
    ETH = 0
    BTC = 1
    LTC = 2

def place_market_order(side: Side, ticker: Ticker, quantity: float) -> bool:
    return True

def place_limit_order(side: Side, ticker: Ticker, quantity: float, price: float, ioc: bool = False) -> int:
    return 0

def cancel_order(ticker: Ticker, order_id: int) -> bool:
    return True

# You can use print() and view the logs after sandbox run has completed
class Strategy:
    def __init__(self) -> None:
        pass
    def on_trade_update(self, ticker: Ticker, side: Side, quantity: float, price: float) -> None:
        pass
    def on_orderbook_update(
        self, ticker: Ticker, side: Side, quantity: float, price: float
    ) -> None:
        pass
    def on_account_update(
        self,
        ticker: Ticker,
        side: Side,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
      pass
`;

const cppCodeString = `#include <cstdint>
#include <string>
enum class Side { buy = 0, sell = 1 };
enum class Ticker : std::uint8_t { ETH = 0, BTC = 1, LTC = 2 };
bool place_market_order(Side side, Ticker ticker, float quantity);
std::int64_t place_limit_order(Side side, Ticker ticker, float quantity,
                               float price, bool ioc = false);

bool cancel_order(Ticker ticker, std::int64_t order_id);
void println(const std::string &text);

class Strategy {
public:
  Strategy() {}
  void on_trade_update(Ticker ticker, Side side, float quantity, float price) {}
  void on_orderbook_update(Ticker ticker, Side side, float quantity,
                           float price) {}
  void on_account_update(Ticker ticker, Side side, float price, float quantity,
                         float capital_remaining) {}
};
`;

const PythonCodeBlock = () => {
  return (
    <SyntaxHighlighter language="python" style={dark} className="text-sm">
      {pythonCodeString}
    </SyntaxHighlighter>
  );
};

const CppCodeBlock = () => {
  return (
    <SyntaxHighlighter language="cpp" style={dark} className="text-sm">
      {cppCodeString}
    </SyntaxHighlighter>
  );
};

export default function Template() {
  return (
<div className="flex flex-col items-center min-h-screen px-4 pt-8 sm:pt-16 md:px-8 pb-16">
  <div className="flex flex-col gap-12 w-full max-w-4xl mx-auto"> {/* Increased gap here to gap-12 */}
    
    {/* Python Code Block */}
    <div className="w-full"> 
      <div className="overflow-hidden rounded-xl shadow-lg bg-gray-900">
        <div className="flex bg-gray-800">
          <div className="flex-1 text-sm font-medium text-gray-300 px-4 py-2 border-b border-gray-700">
            template.py - Python
          </div>
        </div>
        <div className="p-6 overflow-x-auto max-w-full max-h-screen">
          <PythonCodeBlock />
        </div>
      </div>
      <a
        href="/template.py"
        className="mt-4 inline-flex items-center justify-center gap-x-2 rounded bg-indigo-600 px-4 py-2 text-sm font-semibold text-white hover:bg-indigo-500 focus:outline-none focus-visible:ring focus-visible:ring-indigo-500 focus-visible:ring-opacity-50"
      >
        <ArrowDownOnSquareIcon className="h-5 w-5" aria-hidden="true" />
        Download Full Python Template 
      </a>
    </div>

    {/* C++ Code Block */}
    <div className="w-full mt-8"> {/* Added mt-8 to increase spacing */}
      <div className="overflow-hidden rounded-xl shadow-lg bg-gray-900">
        <div className="flex bg-gray-800">
          <div className="flex-1 text-sm font-medium text-gray-300 px-4 py-2 border-b border-gray-700">
            template.hpp - C++
          </div>
        </div>
        <div className="p-6 overflow-x-auto max-w-full max-h-screen">
          <CppCodeBlock />
        </div>
      </div>
      <a
        href="/template.hpp"
        className="mt-4 inline-flex items-center justify-center gap-x-2 rounded bg-indigo-600 px-4 py-2 text-sm font-semibold text-white hover:bg-indigo-500 focus:outline-none focus-visible:ring focus-visible:ring-indigo-500 focus-visible:ring-opacity-50"
      >
        <ArrowDownOnSquareIcon className="h-5 w-5" aria-hidden="true" />
        Download Full C++ Template
      </a>
    </div>
  </div>
</div>
  );
}
