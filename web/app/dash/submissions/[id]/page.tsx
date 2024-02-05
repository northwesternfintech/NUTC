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
import { ref, getBytes } from "firebase/storage";
import { useFirebase } from "@/app/firebase/context";

interface SandboxData {
  matches?: {
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
  ob_updates?: {
    timestamp: string;
    data: {
      price: number;
      quantity: number;
      security: string;
      side: number;
    };
  }[];
}

export default function Page({ params }: { params: { id: string } }) {
  const userInfo = useUserInfo();
  const router = useRouter();

  const [sandboxData, setSandboxData] = React.useState<SandboxData>();

  const { storage } = useFirebase();

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

    const sandboxLogFileURL = userInfo.user.algos!.get(
      params.id,
    )!.sandboxLogFileURL;

    if (!sandboxLogFileURL) {
      return;
    }

    const reference = ref(storage, sandboxLogFileURL);

    getBytes(reference).then(bytes => {
      const decoder = new TextDecoder("utf-8");
      const text = decoder.decode(bytes);
      setSandboxData(JSON.parse(text));
    });
  }, [userInfo, params.id, router, storage]);

  const formatNewLines = (str: string) => {
    const LINES = str.split("\n");
    return LINES.map((line: string, index: number) => (
      <React.Fragment key={`line_${index}`}>
        <p>{line}</p>
        {index < LINES.length - 1 && <br />}
      </React.Fragment>
    ));
  };

  const algoDetails = userInfo?.user?.algos?.get(params.id);
  const lintFailureMessage = algoDetails?.lintFailureMessage;
  const lintSuccessMessage = algoDetails?.lintSuccessMessage;
  const stringToRender = lintFailureMessage || lintSuccessMessage || "";

  return (
    <div>
      {stringToRender === "" ? (
        <p>Waiting on output from linter...</p>
      ) : (
        <div>Linter output: {formatNewLines(stringToRender)}</div>
      )}

      <div className="w-full h-1 border-b-2 border-gray-500 my-4" />

      {sandboxData && <SandboxDashboard sandboxData={sandboxData} />}
    </div>
  );
}

function SandboxDashboard(props: { sandboxData: SandboxData }) {
  const duration = useMemo(() => {
    if (props.sandboxData.matches == null) {
      return 0;
    }

    const first = new Date(props.sandboxData.matches[0].timestamp);
    const last = new Date(
      props.sandboxData.matches[props.sandboxData.matches.length - 1].timestamp,
    );
    return (last.getTime() - first.getTime()) / 1000;
  }, [props.sandboxData]);

  const totalVolumeTraded = useMemo(() => {
    if (props.sandboxData.matches == null) {
      return 0;
    }
    return props.sandboxData.matches.reduce(
      (acc, match) => acc + match.data.quantity,
      0,
    );
  }, [props.sandboxData]);

  return (
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
          <VolumeByTickerChart data={props.sandboxData} />
        </div>
        <div>
          <h3 className="font-semibold">Average Trade Price by Ticker</h3>
          <AverageTradePriceByTickerChart data={props.sandboxData} />
        </div>
        <div>
          <h3 className="font-semibold">Matches per Second</h3>
          <MatchesPerSecondChart data={props.sandboxData} />
        </div>
      </div>
    </div>
  );
}

function MatchesPerSecondChart(props: { data: SandboxData }) {
  const dataForChart = useMemo(() => {
    if (props.data.matches == null) {
      return [];
    }

    const matchesPerSecond = new Map();

    props.data.matches.forEach(match => {
      const date = new Date(match.timestamp);
      const second = date.toISOString().split(".")[0] + "Z"; // Remove milliseconds
      matchesPerSecond.set(second, (matchesPerSecond.get(second) || 0) + 1);
    });

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
    if (props.data.matches == null) {
      return [];
    }

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
    if (props.data.matches == null) {
      return [];
    }
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
