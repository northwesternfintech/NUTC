"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useEffect } from "react";
import { useRouter } from 'next/navigation';

export default function RedirectOnAuth() {
  const { user } = useUserInfo();
  const router = useRouter();
  useEffect(() => {
    if (user?.uid) {
      router.push("/dash");
    }
  }, [user]);
  return (<></>);
}
