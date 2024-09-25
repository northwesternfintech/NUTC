import { ArrowDownOnSquareIcon } from "@heroicons/react/20/solid";
import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import dark from "react-syntax-highlighter/dist/esm/styles/prism/coldark-dark";

const pythonCodeString = `def place_market_order(side: str, ticker: str, quantity: float, price: float) -> bool:
 """Place a market order - DO NOT MODIFY"""

class Strategy:
 """Template for a strategy."""

 def __init__(self) -> None:
 """Your initialization code goes here."""

 def on_trade_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
 print(f"Python Trade update: {ticker} {side} {price} {quantity}")

 def on_orderbook_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
 print(f"Python Orderbook update: {ticker} {side} {price} {quantity}")

 def on_account_update(self, ticker: str, side: str, price: float, quantity: float, capital_remaining: float) -> None:
 print(f"Python Account update: {ticker} {side} {price} {quantity} {capital_remaining}")
`;

const cppCodeString = `// Place a market order - DO NOT MODIFY
bool place_market_order(std::string const& side, std::string const& ticker, double quantity);

// Template for a strategy
class Strategy {
public:
  Strategy() {
    // Your initialization code goes here
  }

  // Called whenever two orders match. Could be one of your orders, or two other people's orders.
  void on_trade_update(std::string ticker, std::string side, double quantity, double price) { }

  // Called whenever the orderbook changes. This could be because of a trade, or because of a new order, or both.
  void on_orderbook_update(std::string ticker, std::string side, double quantity, double price) { }

  // Called whenever one of your orders is filled.
  void on_account_update(std::string ticker, std::string side, double price, double quantity, double capital_remaining) { }
}
`;

const PythonCodeBlock = () => {
  return (
    <SyntaxHighlighter language="python" style={dark}>
      {pythonCodeString}
    </SyntaxHighlighter>
  );
};

const CppCodeBlock = () => {
  return (
    <SyntaxHighlighter language="cpp" style={dark}>
      {cppCodeString}
    </SyntaxHighlighter>
  );
};

export default function Template() {
  return (
    <div className="flex flex-col items-center relative min-h-screen px-6 pt-8 sm:pt-16 md:pl-16 md:pr-16">
      <div className="mx-auto max-w-4xl md:mx-0">
        <div className="overflow-hidden rounded-xl shadow-lg bg-gray-900">
          <div className="flex bg-gray-800">
            <div className="flex-1 text-sm font-medium text-gray-300 px-4 py-2 border-b border-gray-700">
              Template.py
            </div>
          </div>
          <div className="p-6">
            <PythonCodeBlock />
          </div>
        </div>
      </div>
      <a
        href="/template.py"
        className="mt-4 inline-flex items-center justify-center gap-x-2 rounded bg-indigo-600 px-4 py-2 text-sm font-semibold text-white hover:bg-indigo-500 focus:outline-none focus-visible:ring focus-visible:ring-indigo-500 focus-visible:ring-opacity-50">
        <ArrowDownOnSquareIcon className="h-5 w-5" aria-hidden="true" />
        Download Template
      </a>
    </div>
  );
}
