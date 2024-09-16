const faqs = [
  {
    id: 1,
    question: "What is the Northwestern Trading Competition?",
    answer:
      "A simulated trading event where participants test their trading strategies and skills in a competitive environment.",
  },
  {
    id: 2,
    question: "Who can participate in the competition?",
    answer:
      "Undergraduate students from any US school are welcome to apply. We sponsor travel on a case-by-case basis.",
  },
  {
    id: 3,
    question: "How is the competition judged?",
    answer:
      "Each case is judged based on the profit and loss (PnL) of each team's algorithm.",
  },
  {
    id: 4,
    question: "Are teams allowed?",
    answer: "Yes, we require teams of 2-4 during application.",
  },
  {
    id: 5,
    question: "When and where is the competition?",
    answer:
      "The competition will be on October 19th in the Northwestern Technological Institute.",
  },
  {
    id: 6,
    question: "What resources are provided to participants?",
    answer:
      "Participants are given access to market/case information, historical data, and an online backtester.",
  },
];

export default function FAQ() {
  return (
    <div className="bg-gray-900" id="faq">
      <div className="mx-auto max-w-7xl px-6 py-32 sm:py-48 lg:px-8">
        <h2 className="text-2xl font-bold leading-10 tracking-tight text-white text-center">
          Frequently asked questions
        </h2>
        <p className="mt-6 max-w-2xl text-base leading-7 text-gray-300 text-center mx-auto">
          Have a different question and can&apos;t find the answer you&apos;re
          looking for? Reach out to our support team by{" "}
          <a
            href="/contact"
            className="font-semibold text-indigo-400 hover:text-indigo-300">
            sending us an email
          </a>{" "}
          and we&apos;ll get back to you as soon as we can.
        </p>
        <div className="mt-20">
          <dl className="text-center space-y-16 sm:grid sm:grid-cols-2 sm:gap-x-6 sm:gap-y-16 sm:space-y-0 lg:grid-cols-3 lg:gap-x-10">
            {faqs.map(faq => (
              <div key={faq.id}>
                <dt className="text-base font-semibold leading-7 text-white">
                  {faq.question}
                </dt>
                <dd className="mt-2 text-base leading-7 text-gray-300">
                  {faq.answer}
                </dd>
              </div>
            ))}
          </dl>
        </div>
      </div>
    </div>
  );
}
