"use client";
import { Disclosure } from "@headlessui/react";
import { MinusSmallIcon, PlusSmallIcon } from "@heroicons/react/24/outline";

const faqs = [
  {
    question: "How can I get started?",
    answer:
      "Download the template, read the instructions carefully, and modify the Strategy class. Feel free to add new functions to the class, but DO NOT change the signatures of the provided functions.",
  },
  {
    question: "What are the functions in the algorithm template?",
    answer:
      "There are four main functions that allow you to interact with the exchange. place_market_order allows you to place orders for the exchange at a given price/quantity, and you can call this in any function (including __init__). on_orderbook_update is called when a new order is placed by another algorithm (BUY or SELL). on_trade_update is called when two orders match (one BUY, one SELL). This could be your order or two other orders. on_account_update is called when one of *your* orders matches with another order.",
  },
  {
    question: "What does a zero-quantity order mean?",
    answer:
      "An order at the given price point was filled. This helps you to update your order book.",
  },
  {
    question: "How does place_market_order work?",
    answer:
      "You can place an order (BUY or SELL) at a given price/quantity for a given stock ticker. Importantly, it returns True if the order was placed, or False if it was not placed (due to you placing more than 30 orders in a minute). You may want to handle the case where you aren't able to place an order.",
  },
  { question: "What's my starting capital?", answer: "100,000" },
  {
    question: "What libraries can I use?",
    answer: "numpy, pandas, scipy, polars, and scikit-learn",
  },
  {
    question: "How can I do well in the competition?",
    answer:
      "Read up on how trading algorithms work. Oftentimes, the winner is not the most advanced algorithm - it's the most clever.",
  },
  {
    question: "How do I know my code will work?",
    answer:
      "When you submit an algorithm on this website, it will be tested automatically. If any function fails to run, you can click on the submission to view the error.",
  },
  {
    question: "How many algorithms can I submit?",
    answer:
      "As many as you want, but the last algorithm that passes linting will be the one to run in the contest",
  },
  {
    question: "How does order matching work?",
    answer:
      "When a BUY order is submitted, if the SELL order with lowest asking price <= the BUY price, they will match (and vice versa for incoming SELL orders).",
  },
  {
    question: "When is the deadline to submit algorithms?",
    answer: "11:59pm on 11/12.",
  },
  {
    question: "How are algorithms evaluated?",
    answer:
      "By the amount of capital they hold at the end of the competition (incl. value of held stocks)",
  },
  {
    question:
      "How does buying and selling at the beginning of the contest work if nobody holds stocks?",
    answer:
      "We provide simulated SELL orders at the beginning of the contest. You are not competing against these simulated sellers, they exist soleley to provide initial liquidity.",
  },
  {
    question: "What if I have other questions or have issues with the website?",
    answer:
      'Please reach out to finrlcontest@gmail.com with the subject "Submission Platform Question".',
  },
];

export default function FAQs() {
  return (
    <div className="bg-gray-800">
      <div className="mx-auto max-w-7xl px-6 lg:px-8 pt-6 sm:pt-12 lg:pt-20 pb-12">
        <div className="mx-auto max-w-4xl divide-y divide-white/10">
          <h2 className="text-2xl font-bold leading-10 tracking-tight text-white">
            Frequently asked questions for Task II Real Time Order Execution
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
                          {open
                            ? (
                              <MinusSmallIcon
                                className="h-6 w-6"
                                aria-hidden="true"
                              />
                            )
                            : (
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
