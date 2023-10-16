"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect } from "react";
import { useRouter } from "next/navigation";
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
  return (
    <h1>
      {userInfo?.user?.algos?.get(params.id)?.lintFailureMessage ||
        userInfo?.user?.algos?.get(params.id)?.lintSuccessMessage || ""}
    </h1>
  );
}
