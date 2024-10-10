"use client";

import { ArrowDownTrayIcon } from "@heroicons/react/16/solid";
import { Algo, AlgoFile } from "@prisma/client";
import React, { useEffect, useState } from "react";

export function AlgoGraphs({
  algo,
  userId,
  s3Endpoint,
}: {
  algo: Algo & { algoFile: AlgoFile };
  userId: string;
  s3Endpoint: string;
}) {
  const [showLogs, setShowLogs] = useState(false);
  const upTime = new Date(algo?.algoFile?.createdAt).getTime() + 1000;
  const sandboxTimeMs = 300000;
  const baseEndpoint = `${process.env.NEXT_PUBLIC_NGINX_ENDPOINT}/d-solo/cdk4teh4zl534a/nutc-dev?orgId=1&var-traderid=${algo.algoFileS3Key}&from=${upTime}&theme=dark`;
  const [url, setUrl] = useState(baseEndpoint + `&refresh=5s`);

  useEffect(() => {
    if (upTime + sandboxTimeMs > Date.now()) {
      setUrl(baseEndpoint + "&refresh=5s");
      setTimeout(() => {
        setUrl(baseEndpoint + `&to=${upTime + sandboxTimeMs}`);
        setShowLogs(true);
      }, upTime + sandboxTimeMs - Date.now());
    } else {
      setUrl(baseEndpoint + `&to=${upTime + sandboxTimeMs}`);
      setShowLogs(true);
    }
  }, [baseEndpoint, setShowLogs, upTime]);

  return (
    <>
      {showLogs ? (
        <a
          type="button"
          target="_blank"
          href={`${s3Endpoint}/nutc/${algo.logFileS3Key}`}
          className="mt-2 inline-flex items-center gap-x-1.5 rounded-md bg-indigo-700 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-600 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-700">
          Download Logs
          <ArrowDownTrayIcon className="-mr-0.5 h-5 w-5" aria-hidden="true" />
        </a>
      ) : null}

      <div className="my-8">
        <h2 className="text-xl font-semibold mb-2">Profit and Loss</h2>
        <iframe
          className="border border-gray-300"
          src={url + "&panelId=14"}
          width="950"
          height="400"
          frameBorder="0"
          title="Panel 1"
        />
      </div>

      <div className="mb-8">
        <h2 className="text-xl font-semibold mb-2">Capital</h2>
        <iframe
          className="border border-gray-300 "
          src={url + "&panelId=13"}
          width="950"
          height="400"
          frameBorder="0"
          title="Panel 2"
        />
      </div>
    </>
  );
}
