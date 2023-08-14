"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect } from "react";
import { useRouter } from "next/navigation";

export default function RedirectOnAuth() {
  const { user } = useUserInfo();
  const router = useRouter();
  useEffect(() => {
    if (user) {
      if (user?.hasCompletedReg) {
        router.push("/dash");
      } else {
        router.push("/registration");
      }
    }
  }, [user]);
  return <></>;
}
