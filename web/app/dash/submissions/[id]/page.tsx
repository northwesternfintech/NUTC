"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { apiEndpoint } from "@/config";
import React from "react";
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

  const algoDetails = userInfo?.user?.algos?.get(params.id);
  const lintFailureMessage = algoDetails?.lintFailureMessage;
  const lintSuccessMessage = algoDetails?.lintSuccessMessage;
  const stringToRender = lintFailureMessage || lintSuccessMessage || "";
  const upTime = (algoDetails?.uploadTime || 0) + 1000;
  const sandboxTimeMs = 300000;
  const baseEndpoint =
    apiEndpoint() +
    `/d-solo/cdk4teh4zl534a/ppl?orgId=1&var-traderid=${userInfo?.user?.uid}-${params.id}&from=${upTime}&theme=dark`;

  const [url, setUrl] = useState(baseEndpoint + `&refresh=5s`);

  useEffect(() => {
    if (upTime + sandboxTimeMs > Date.now()) {
      setUrl(baseEndpoint + "&refresh=5s");
      setTimeout(
        () => {
          setUrl(baseEndpoint + `&to=${upTime + sandboxTimeMs}`);
        },
        upTime + sandboxTimeMs - Date.now(),
      );
    } else {
      setUrl(baseEndpoint + `&to=${upTime + sandboxTimeMs}`);
    }
  }, [userInfo.user]);

  if (stringToRender === "") {
    return (
      <div>
        <p>Waiting on output...</p>
      </div>
    );
  } else if (stringToRender.includes("succeeded")) {
    return (
      <div className="flex flex-col items-center">
        <h1 className="text-3xl font-bold mb-4 pt-5">Sandbox View of {algoDetails?.name}</h1>
        
        <div className="mb-8">
          <h2 className="text-xl font-semibold mb-2">Profit and Loss</h2>
          <iframe
            className="border border-gray-300"
            src={url + "&panelId=1"}
            width="900"
            height="400"
            frameBorder="0"
            title="Panel 1"
          ></iframe>
        </div>
        
        <div>
          <h2 className="text-xl font-semibold mb-2">Capital</h2>
          <iframe
            className="border border-gray-300"
            src={url + "&panelId=2"}
            width="900"
            height="400"
            frameBorder="0"
            title="Panel 2"
          ></iframe>
        </div>
      </div>
    );
  } else {
    return formatNewLines(stringToRender);
  }
}
