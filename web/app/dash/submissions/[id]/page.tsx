"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect, useMemo } from "react";
import { useRouter } from "next/navigation";
import React from "react";
import {
  Bar,
  BarChart,
  CartesianGrid,
  Legend,
  Line,
  LineChart,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from "recharts";

interface SandboxData {
  matches: {
    timestamp: string;
    data: {
      buyer_id: string;
      price: number;
      quantity: number;
      seller_id: string;
      side: number;
      ticker: string;
    };
  }[];
  ob_updates: {
    timestamp: string;
    data: {
      price: number;
      quantity: number;
      security: string;
      side: number;
    };
  }[];
}

const sandboxOutput = {
  matches: [
    {
      timestamp: "2024-02-02T04:32:03.956Z",
      data: {
        buyer_id: "vzZV25SctsUZOFPpptE8XXYFP6h2",
        price: 100,
        quantity: 333,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "A",
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.956Z",
      data: {
        buyer_id: "vzZV25SctsUZOFPpptE8XXYFP6h2",
        price: 100,
        quantity: 333,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "A",
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.959Z",
      data: {
        buyer_id: "vzZV25SctsUZOFPpptE8XXYFP6h2",
        price: 200,
        quantity: 166,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "B",
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.959Z",
      data: {
        buyer_id: "vzZV25SctsUZOFPpptE8XXYFP6h2",
        price: 200,
        quantity: 166,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "B",
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.961Z",
      data: {
        buyer_id: "vzZV25SctsUZOFPpptE8XXYFP6h2",
        price: 300,
        quantity: 111,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "C",
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.961Z",
      data: {
        buyer_id: "vzZV25SctsUZOFPpptE8XXYFP6h2",
        price: 300,
        quantity: 111,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "C",
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.963Z",
      data: {
        buyer_id: "HPl7qB7JXVPr8whA5CdeHkiIJgy2",
        price: 100,
        quantity: 1,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "A",
      },
    },
    {
      timestamp: "2024-02-02T04:32:04.963Z",
      data: {
        buyer_id: "HPl7qB7JXVPr8whA5CdeHkiIJgy2",
        price: 100,
        quantity: 1,
        seller_id: "SIMULATED",
        side: 0,
        ticker: "A",
      },
    },
  ],
  ob_updates: [
    {
      timestamp: "2024-02-02T04:32:03.956Z",
      data: {
        price: 100,
        quantity: 0,
        security: "A",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.956Z",
      data: {
        price: 100,
        quantity: 667,
        security: "A",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.959Z",
      data: {
        price: 200,
        quantity: 0,
        security: "B",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.959Z",
      data: {
        price: 200,
        quantity: 1834,
        security: "B",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.961Z",
      data: {
        price: 300,
        quantity: 0,
        security: "C",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.961Z",
      data: {
        price: 300,
        quantity: 2889,
        security: "C",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.963Z",
      data: {
        price: 100,
        quantity: 0,
        security: "A",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.963Z",
      data: {
        price: 100,
        quantity: 666,
        security: "A",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.964Z",
      data: {
        price: 105,
        quantity: 1,
        security: "A",
        side: 1,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.965Z",
      data: {
        price: 99.9,
        quantity: 10,
        security: "A",
        side: 0,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.965Z",
      data: {
        price: 1,
        quantity: 100,
        security: "A",
        side: 0,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.966Z",
      data: {
        price: 199.9,
        quantity: 10,
        security: "B",
        side: 0,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.966Z",
      data: {
        price: 299.9,
        quantity: 10,
        security: "C",
        side: 0,
      },
    },
    {
      timestamp: "2024-02-02T04:32:03.967Z",
      data: {
        price: 99.9,
        quantity: 10,
        security: "A",
        side: 0,
      },
    },
  ],
} satisfies SandboxData;

function MatchesPerSecondChart(props: { data: SandboxData }) {
  const dataForChart = useMemo(() => {
    // Initialize an empty object to hold the count of matches per second
    const matchesPerSecond = new Map();

    props.data.matches.forEach(match => {
      // Convert timestamp to date and then back to a string without milliseconds for grouping
      const date = new Date(match.timestamp);
      const second = date.toISOString().split(".")[0] + "Z"; // Remove milliseconds
      // Count the occurrences
      matchesPerSecond.set(second, (matchesPerSecond.get(second) || 0) + 1);
    });

    // Convert the map to an array of objects suitable for Recharts
    const dataForChart = Array.from(matchesPerSecond, ([timestamp, count]) => ({
      timestamp,
      count,
    }));

    return dataForChart;
  }, [props.data]);

  const formatXAxis = (timestamp: string) => {
    const date = new Date(timestamp);
    return date.toISOString().split("T")[1].split(".")[0];
  };

  return (
    <ResponsiveContainer width="100%" height={300}>
      <LineChart data={dataForChart}>
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="timestamp" tickFormatter={formatXAxis} />
        <YAxis />
        <Tooltip />
        <Legend />
        <Line type="monotone" dataKey="count" stroke="#ff0000" />
      </LineChart>
    </ResponsiveContainer>
  );
}

function VolumeByTickerChart(props: { data: SandboxData }) {
  const volumeByTickerData = useMemo(() => {
    const volumeByTicker: { [key: string]: number } = {};

    props.data.matches.forEach((match: any) => {
      const { ticker } = match.data;
      const quantity = match.data.quantity;

      if (ticker in volumeByTicker) {
        volumeByTicker[ticker] += quantity;
      } else {
        volumeByTicker[ticker] = quantity;
      }
    });

    return Object.entries(volumeByTicker).map(([name, volume]) => ({
      name,
      volume,
    }));
  }, [props.data]);

  return (
    <ResponsiveContainer width="100%" height={300}>
      <BarChart data={volumeByTickerData}>
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="name" />
        <YAxis />
        <Tooltip />
        <Legend />
        <Bar dataKey="volume" fill="#8884d8" />
      </BarChart>
    </ResponsiveContainer>
  );
}

function AverageTradePriceByTickerChart(props: { data: SandboxData }) {
  const averageTradePriceByTicker = useMemo(() => {
    const sums: Record<string, number> = {};
    const quantities: Record<string, number> = {};

    props.data.matches.forEach(match => {
      const { ticker, price, quantity } = match.data;
      if (!sums[ticker]) sums[ticker] = 0;
      if (!quantities[ticker]) quantities[ticker] = 0;
      sums[ticker] += price * quantity;
      quantities[ticker] += quantity;
    });
    return Object.keys(sums).map(ticker => ({
      name: ticker,
      averagePrice: sums[ticker] / quantities[ticker],
    }));
  }, [props.data]);

  return (
    <ResponsiveContainer width="100%" height={300}>
      <BarChart data={averageTradePriceByTicker}>
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="name" />
        <YAxis />
        <Tooltip />
        <Legend />
        <Bar dataKey="averagePrice" fill="#82ca9d" />
      </BarChart>
    </ResponsiveContainer>
  );
}

export default function Page({ params }: { params: { id: string } }) {
  const userInfo = useUserInfo();
  const router = useRouter();
  useEffect(() => {
    if (!userInfo?.user) {
      return;
    }
    if (!userInfo?.user?.algos) {
      router.push("/dash");
    }
    if (!userInfo?.user?.algos?.has(params.id)) {
      router.push("/dash");
    }
  });

  const formatNewLines = (str: string) => {
    const LINES = str.split("\n");
    return LINES.map((line: string, index: number) => (
      <React.Fragment key={`line_${index}`}>
        <p>{line}</p>
        {index < LINES.length - 1 && <br />}
      </React.Fragment>
    ));
  };

  const duration = useMemo(() => {
    const first = new Date(sandboxOutput.matches[0].timestamp);
    const last = new Date(
      sandboxOutput.matches[sandboxOutput.matches.length - 1].timestamp,
    );
    return (last.getTime() - first.getTime()) / 1000;
  }, []);

  const algoDetails = userInfo?.user?.algos?.get(params.id);
  const lintFailureMessage = algoDetails?.lintFailureMessage;
  const lintSuccessMessage = algoDetails?.lintSuccessMessage;
  const stringToRender = lintFailureMessage || lintSuccessMessage || "";

  const totalVolumeTraded = sandboxOutput.matches.reduce(
    (acc, match) => acc + match.data.quantity,
    0,
  );

  return (
    <div>
      {stringToRender === "" ? (
        <p>Waiting on output from linter...</p>
      ) : (
        <div>Linter output: {formatNewLines(stringToRender)}</div>
      )}

      <div className="w-full h-1 border-b-2 border-gray-500 my-4" />

      <div className="mx-1">
        <h2 className="text-xl font-bold mb-4 text-center">
          Results from sandboxed run:
        </h2>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-8">
          <div className="bg-white shadow overflow-hidden rounded-lg p-4">
            <div className="px-4 py-5 sm:p-6">
              <dl>
                <dt className="text-sm font-medium text-gray-500 truncate">
                  Total Volume Traded
                </dt>
                <dd className="mt-1 text-3xl font-semibold text-gray-900">
                  {totalVolumeTraded}
                </dd>
              </dl>
            </div>
          </div>

          <div className="bg-white shadow overflow-hidden rounded-lg p-4">
            <div className="px-4 py-5 sm:p-6">
              <dl>
                <dt className="text-sm font-medium text-gray-500 truncate">
                  Trial time (s)
                </dt>
                <dd className="mt-1 text-3xl font-semibold text-gray-900">
                  {duration}
                </dd>
              </dl>
            </div>
          </div>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <h3 className="font-semibold">Volume Traded by Ticker</h3>
            <VolumeByTickerChart data={sandboxOutput} />
          </div>
          <div>
            <h3 className="font-semibold">Average Trade Price by Ticker</h3>
            <AverageTradePriceByTickerChart data={sandboxOutput} />
          </div>
          <div>
            <h3 className="font-semibold">Matches per Second</h3>
            <MatchesPerSecondChart data={sandboxOutput} />
          </div>
        </div>
      </div>
    </div>
  );
}
