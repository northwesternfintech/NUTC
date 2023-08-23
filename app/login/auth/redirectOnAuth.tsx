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
        if (user?.isApprovedApplicant) {
          router.push("/dash");
        } else if (user?.isRejectedApplicant) {
          router.push("/app-rejected");
        } else {
          router.push("/app-submitted");
        }
      } else {
        router.push("/registration");
      }
    } else {
      router.push("/login");
    }
  }, [user]);

  return <></>;
}
