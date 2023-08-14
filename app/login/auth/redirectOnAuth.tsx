"use client";
import { useEffect } from "react";
import { useRouter } from "next/navigation";
import { getAuth, onAuthStateChanged } from "firebase/auth";
import { UserInfoType, useUserInfo } from "@/app/login/auth/context";
import { useFirebase } from "@/app/firebase/context";
import { child, get, ref } from "firebase/database";

export default function RedirectOnAuth() {
  const { user } = useUserInfo();
  const router = useRouter();
  const { database } = useFirebase();
  const { setUser } = useUserInfo();
  useEffect(() => {
    if (user) {
      if (user?.hasCompletedReg) {
        router.push("/dash");
      } else {
        router.push("/registration");
      }
    }
  }, [user]);

  const auth = getAuth();
  onAuthStateChanged(auth, async (user) => {
    if (!user) return;
    const uid = user.uid;
    const snapshot = await get(child(ref(database), `users/${uid}`));
    if (!snapshot.exists()) {
      //create new user
      const newUser: UserInfoType = {
        uid: uid,
        displayName: user.displayName || "",
        photoURL: user.photoURL || "",
        email: user.email || "",
        hasCompletedReg: false,
      };
      setUser(newUser);
    } else {
      const dbUser = snapshot.val();
      const newUser: UserInfoType = {
        uid: uid,
        displayName: dbUser.displayName || "",
        photoURL: dbUser.photoURL || "",
        email: dbUser.email || user.email || "",
        hasCompletedReg: dbUser.hasCompletedReg || false,
      };
      setUser(newUser);
    }
  });

  return <></>;
}
