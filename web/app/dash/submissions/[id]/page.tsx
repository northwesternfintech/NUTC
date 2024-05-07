"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { apiEndpoint } from "@/config";
import {ArrowDownTrayIcon} from "@heroicons/react/24/solid"
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
  const [hr, setHr] = useState("#");

  useEffect(() => {
  if(algoDetails?.sandbox_results) {
	  setHr(algoDetails?.sandbox_results.replaceAll("0x0st", "0x0.st")?.replaceAll("txt",".txt"));
  }
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
        <h1 className="text-3xl font-bold mb-2 pt-5">Sandbox View of {algoDetails?.name}</h1>
        <h3 className="text-lg font-normal mb-4 pt-2 text-gray-400">Ensure to review the <a className="font-bold text-indigo-300" href="https://docs.google.com/document/d/1FfWrKIXGO7oPKTTTwyprH3kM8WrnIuZmp9kcH4lo6CA/edit?usp=sharing" target="_blank">case packet</a>. Results may be deleted after 24 hours.</h3>

<div className="flex flex-row gap-x-2">
	<a
        type="button"
	target="_blank"
	href={algoDetails?.downloadURL}
        className="inline-flex items-center gap-x-1.5 rounded-md bg-indigo-700 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-600 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-700"
      >
        Download Submission
        <ArrowDownTrayIcon className="-mr-0.5 h-5 w-5" aria-hidden="true" />
      </a>
	<a
        type="button"
	target="_blank"
	href={hr}
        className="inline-flex items-center gap-x-1.5 rounded-md bg-indigo-700 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-600 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-700"
      >
        Download Logs
        <ArrowDownTrayIcon className="-mr-0.5 h-5 w-5" aria-hidden="true" />
      </a></div>
        
        <div className="my-8">
          <h2 className="text-xl font-semibold mb-2">Profit and Loss</h2>
          <iframe
            className="border border-gray-300"
            src={url + "&panelId=1"}
            width="950"
            height="400"
            frameBorder="0"
            title="Panel 1"
          ></iframe>
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
          ></iframe>
        </div>
      </div>
    );
  } else {
    return formatNewLines(stringToRender);
  }
}
