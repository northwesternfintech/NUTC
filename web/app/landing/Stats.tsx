const stats = [
  { id: 1, name: "Participants", value: "65+" },
  { id: 2, name: "Cash Prizes", value: "$5,000+" },
  { id: 3, name: "Cases", value: "4+" },
];

export default function Stats() {
  return (
    <div className="bg-gray-900 py-32 sm:py-48">
      <div className="mx-auto max-w-7xl px-6 lg:px-8">
        <dl className="grid grid-cols-1 gap-x-8 gap-y-16 text-center lg:grid-cols-3">
          {stats.map(stat => (
            <div
              key={stat.id}
              className="mx-auto flex max-w-xs flex-col gap-y-4">
              <dt className="text-base leading-7 text-gray-400">{stat.name}</dt>
              <dd className="order-first text-3xl font-semibold tracking-tight text-white sm:text-5xl">
                {stat.value}
              </dd>
            </div>
          ))}
        </dl>
      </div>
    </div>
  );
}
