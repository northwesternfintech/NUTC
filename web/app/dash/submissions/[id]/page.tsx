"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect } from "react";
import { useRouter } from "next/navigation";
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
        <h1>{line}</h1>
        {index < LINES.length - 1 && <br />}
      </React.Fragment>
    ));
  }
  

  const stringToRender = userInfo?.user?.algos?.get(params.id)?.lintFailureMessage ||
  userInfo?.user?.algos?.get(params.id)?.lintSuccessMessage || "";
  if(stringToRender === "") {
    return <></>
  } else {
    return (
      <>
        {formatNewLines(stringToRender)}
      </>
    );
  }
}
