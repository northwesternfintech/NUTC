"use client";
import { useEffect } from "react";
import { useRouter } from "next/navigation";
import { useUserInfo } from "@/app/login/auth/context";

interface RedirectOnAuthProps {
  page: string;
}

export default function RedirectOnAuth({ page }: RedirectOnAuthProps) {
  const { user } = useUserInfo();
  const router = useRouter();
  useEffect(() => {
    if (localStorage.getItem("isLoggedIn") === "true" && !user) {
      return;
    }

    // if (user) {
    //   if (user?.hasCompletedReg) {
    //     if (user?.isApprovedApplicant) {
    //       if (page === "dash") {
    //         return;
    //       }
    //       router.push("/dash");
    //     } else if (user?.isRejectedApplicant) {
    //       router.push("/app-rejected");
    //     } else {
    //       router.push("/app-submitted");
    //     }
    //   } else {
    //     router.push("/registration");
    //   }
    // } else {
    //   router.push("/login");
    // }
  }, [user]);

  return <></>;
}
