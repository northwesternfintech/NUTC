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
  useEffect(() => {    if (localStorage.getItem("isLoggedIn") === "true" && !user) {
    return;
  }

    if (user) {
      console.log("user", user);
      if (user?.hasCompletedReg) {
        if (user?.isApprovedApplicant) {
          if (page === "dash") {
            return;
          }
          router.push("/dash");
        } else if (user?.isRejectedApplicant) {
          if (page === "app-rejected") {
            return;
          }
          router.push("/app-rejected");
        } else {
          if (page === "app-submitted") {
            return;
          }
          router.push("/app-submitted");
        }
      } else {
        if (page === "registration") {
          return;
        }
        router.push("/registration");
      }
    } else {
      if (page === "login") {
        return;
      }
      router.push("/login");
    }

  }, [user]);

  return <></>;
}
