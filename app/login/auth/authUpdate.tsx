"use client";
import { useEffect } from "react";
import { useFirebase } from "@/app/firebase/context";
import { child, get, ref } from "firebase/database";
import { UserInfoType, useUserInfo } from "@/app/login/auth/context";
import { onAuthStateChanged } from "firebase/auth";
import { getAuth } from "firebase/auth";

export default function AuthUpdate() {
  const { database } = useFirebase();
  const auth = getAuth();

  const { setUser } = useUserInfo();

  useEffect(() => {
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
          displayName: dbUser.displayName || user.displayName || "",
          photoURL: dbUser.photoURL || user.photoURL || "",
          email: dbUser.email || user.email || "",
          hasCompletedReg: dbUser.hasCompletedReg || false,
        };
        setUser(newUser);
      }
    });
  }, []);
  return <></>;
}
