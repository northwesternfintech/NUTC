"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect } from "react";
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
  const upTime = algoDetails?.uploadTime || 0;
  function panelUrl(panelNum: number) {
    return apiEndpoint() +
      `d-solo/cdk4teh4zl534a/ppl?orgId=1&var-traderid=${userInfo?.user?.uid}-${params.id}&from=${upTime}&theme=dark&panelId=${panelNum}&` +
      (upTime + 300000 < Date.now() ? `&to=${upTime + 300000}` : "&refresh=5s");
  }

  // TODO: set timeout for 5mins from start to change to no refresh

  if (stringToRender === "") {
    return (
      <div>
        <p>Waiting on output...</p>
      </div>
    );
  } else {
    return (
      <div className="flex flex-col items-center">
        {formatNewLines(stringToRender)}
        <iframe
          src={panelUrl(1)}
          width="900"
          height="400"
          frameBorder="0"
        >
        </iframe>
        <iframe
          src={panelUrl(2)}
          width="900"
          height="400"
          frameBorder="0"
        >
        </iframe>
      </div>
    );
  }
}
