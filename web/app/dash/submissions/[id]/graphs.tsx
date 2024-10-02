"use client";

import { Algo, AlgoFile } from "@prisma/client";
import React, { useEffect, useState } from "react";

export function AlgoGraphs({
  algo,
  userId,
}: {
  algo: Algo & { algoFile: AlgoFile };
  userId: string;
}) {
  const upTime = (algo?.algoFile?.createdAt?.getMilliseconds() || 0) + 1000;
  const sandboxTimeMs = 300000;
  const baseEndpoint = `${process.env.NGINX_ENDPOINT}/d-solo/cdk4teh4zl534a/ppl?orgId=1&var-traderid=${userId}-${algo.algoFileS3Key}&from=${upTime}&theme=dark`;
  const [url, setUrl] = useState(baseEndpoint + `&refresh=5s`);

  useEffect(() => {
    if (upTime + sandboxTimeMs > Date.now()) {
      setUrl(baseEndpoint + "&refresh=5s");
      setTimeout(() => {
        setUrl(baseEndpoint + `&to=${upTime + sandboxTimeMs}`);
      }, upTime + sandboxTimeMs - Date.now());
    } else {
      setUrl(baseEndpoint + `&to=${upTime + sandboxTimeMs}`);
    }
  }, [baseEndpoint, upTime]);

  return (
    <>
      <div className="my-8">
        <h2 className="text-xl font-semibold mb-2">Profit and Loss</h2>
        <iframe
          className="border border-gray-300"
          src={url + "&panelId=1"}
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
          src={url + "&panelId=2"}
          width="950"
          height="400"
          frameBorder="0"
          title="Panel 2"
        />
      </div>
    </>
  );
}
