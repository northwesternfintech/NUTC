import { getAlgo } from "@/api";
import { getSession } from "@auth0/nextjs-auth0";
import { ArrowDownTrayIcon } from "@heroicons/react/16/solid";
import { notFound, redirect } from "next/navigation";
import React from "react";
import { AlgoGraphs } from "./graphs";

export default async function SubmissionPage(props: {
  params: { id: string };
}) {
  const session = await getSession();
  const algo = await getAlgo(props.params.id);

  if (!session) {
    redirect("/api/auth/login");
  }

  if (!algo) {
    notFound();
  }

  const formatNewLines = (str: string) => {
    const LINES = str.split("\n");
    return LINES.map((line: string, index: number) => (
      <React.Fragment key={`line_${index}`}>
        <p>{line}</p>
        {index < LINES.length - 1 && <br />}
      </React.Fragment>
    ));
  };

  const lintFailureMessage = algo?.lintFailureMessage;
  const lintSuccessMessage = algo?.lintSuccessMessage;
  const stringToRender = lintFailureMessage || lintSuccessMessage || "";

  if (stringToRender === "") {
    return (
      <div>
        <p>Waiting on output...</p>
      </div>
    );
  } else if (stringToRender.includes("succeeded")) {
    return (
      <div className="flex flex-col items-center">
        <h1 className="text-3xl font-bold mb-2 pt-5">
          Sandbox View of {algo.name}
        </h1>
        <h3 className="text-lg font-normal mb-4 pt-2 text-gray-400">
          Ensure to review the{" "}
          <a
            className="font-bold text-indigo-300"
            href="https://docs.google.com/document/d/1FfWrKIXGO7oPKTTTwyprH3kM8WrnIuZmp9kcH4lo6CA/edit?usp=sharing"
            target="_blank">
            case packet
          </a>
          . Results may be deleted after 24 hours.
        </h3>

        <a
          type="button"
          target="_blank"
          href={`${process.env.EXTERNAL_S3_ENDPOINT}/nutc/${algo.algoFile.s3Key}`}
          className="inline-flex items-center gap-x-1.5 rounded-md bg-indigo-700 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-600 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-700">
          Download Submission
          <ArrowDownTrayIcon className="-mr-0.5 h-5 w-5" aria-hidden="true" />
        </a>

        <AlgoGraphs algo={algo} userId={session.user.sub} />
      </div>
    );
  } else {
    return formatNewLines(stringToRender);
  }
}
