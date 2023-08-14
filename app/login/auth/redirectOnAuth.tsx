"use client";
import { useEffect } from "react";
import { useRouter } from "next/navigation";
import { useUserInfo } from "@/app/login/auth/context";

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
