"use client";
import { Disclosure } from "@headlessui/react";
import { MinusSmallIcon, PlusSmallIcon } from "@heroicons/react/24/outline";

const faqs = [
  {
    question: "What is the Northwestern Trading Competition?",
    answer:
      "NUTC is a competition to design a trading algorithm that maximizes a metric, usually PnL (net profit and loss) against other algorithms in the competition. In other words, you want your algorithm to buy and sell securities in a way that maximizes the amount of money you end up with.",
  },
  {
    question: "How can I get started?",
    answer:
      "Download the template, read the instructions carefully, and modify the Strategy class. Feel free to add new functions to the class, but DO NOT change the signatures of the provided functions.",
  },
  {
    question: "What are the functions in the algorithm template?",
    answer:
      "There are four main functions that allow you to interact with the exchange. place_market_order allows you to place orders for the exchange at a given price/quantity, and you can call this in any function (including __init__). on_orderbook_update is called when a new order is placed by another algorithm (BUY or SELL). on_trade_update is called when two orders match (one BUY, one SELL). This could be your order or two other orders. on_account_update is called when one of *your* orders matches with another order. Together, all of these functions are sufficient for you to maintain a complete copy of the local orderbook - this is highly recommended.",
  },
  {
    question: "What does a zero-quantity order mean?",
    answer:
      "All liqudity at this price level was fulfilled. This helps you to update your order book - you can now safely remove that price level.",
  },
  {
    question: "How does place_market_order work?",
    answer:
      "You can place an order (BUY or SELL) at a given price/quantity for a given stock ticker. Importantly, it returns True if the order was placed, or False if it was not placed (due to you placing more than 30 orders in a minute). You may want to handle the case where you aren't able to place an order due to the rate limit.",
  },
  { question: "How much starting capital do I have?", answer: "100,000" },
  {
    question: "What libraries can I use?",
    answer:
      "numpy, pandas, scipy, polars, and scikit-learn are currently available, all at latest. More libraries may be made available upon request.",
  },
  {
    question: "How can I do well in the competition?",
    answer:
      "Read up on how trading algorithms work. Oftentimes, the winner is not the most advanced algorithm - it's the most clever. Use the sandbox to confirm your algorithm's behavior.",
  },
  {
    question: "How do I know my code will work?",
    answer:
      "When you submit an algorithm on this website, it will be tested automatically. If any function fails to run, you can click on the submission to view the error.",
  },
  {
    question: "How many algorithms can I submit?",
    answer:
      "As many as you want, but the last algorithm that passes linting will be the one to run in the contest.",
  },
  {
    question: "How does order matching work?",
    answer:
      "Matches are done via Price-Time priority. In other words, when a BUY order is submitted, if the SELL order with lowest asking price <= the BUY price, they will match (and vice versa for incoming SELL orders).",
  },
  {
    question: "When is the deadline to submit algorithms?",
    answer: "11:59pm on 5/3.",
  },
  {
    question: "How are algorithms evaluated?",
    answer:
      "By the amount of capital they hold at the end of the competition (incl. value of held stocks, which are automatically liquidated at a fair market price, with market impact.)",
  },
  {
    question: "How does the market start?",
    answer:
      "There are many liquidity providing bots on the exchange who will automatically trade with one another. Your algorithm should react to these trades - placing trades during initialization is heavily advised against.",
  },
  {
    question: "What if I have other questions or have issues with the website?",
    answer: "Please reach out in the piazza",
  },
];

export default function FAQs() {
  return (
    <div className="bg-gray-900">
      <div className="mx-auto max-w-7xl px-6 lg:px-8 pt-6 sm:pt-12 lg:pt-20 pb-12">
        <div className="mx-auto max-w-4xl divide-y divide-white/10">
          <h2 className="text-2xl font-bold leading-10 tracking-tight text-white">
            Frequently asked questions
          </h2>
          <dl className="mt-10 space-y-6 divide-y divide-white/10">
            {faqs.map((faq) => (
              <Disclosure as="div" key={faq.question} className="pt-6">
                {({ open }) => (
                  <>
                    <dt>
                      <Disclosure.Button className="flex w-full items-start justify-between text-left text-white">
                        <span className="text-base font-semibold leading-7">
                          {faq.question}
                        </span>
                        <span className="ml-6 flex h-7 items-center">
                          {open ? (
                            <MinusSmallIcon
                              className="h-6 w-6"
                              aria-hidden="true"
                            />
                          ) : (
                            <PlusSmallIcon
                              className="h-6 w-6"
                              aria-hidden="true"
                            />
                          )}
                        </span>
                      </Disclosure.Button>
                    </dt>
                    <Disclosure.Panel as="dd" className="mt-2 pr-12">
                      <p className="text-base leading-7 text-gray-300">
                        {faq.answer}
                      </p>
                    </Disclosure.Panel>
                  </>
                )}
              </Disclosure>
            ))}
          </dl>
        </div>
      </div>
    </div>
  );
}
