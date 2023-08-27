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

  const { setUser, user } = useUserInfo();

  useEffect(() => {
    onAuthStateChanged(auth, async (authUser) => {
      if (user) return;
      if (!authUser) return;

      const uid = authUser.uid;
      const snapshot = await get(child(ref(database), `users/${uid}`));
      localStorage.setItem("isLoggedIn", "true");
      if (!snapshot.exists()) {
        //create new user
        const newUser: UserInfoType = {
          uid: uid,
          isFilledFromDB: true,
          username: "",
          about: "",
          photoURL: authUser.photoURL || "",
          resumeURL: "",
          firstName: "",
          lastName: "",
          email: authUser.email || "",
          school: "",
          hasCompletedReg: false,
          isApprovedApplicant: false,
          isRejectedApplicant: false,
        };
        setUser(newUser);
      } else {
        const dbUser: UserInfoType = snapshot.val();
        const newUser: UserInfoType = {
          uid: uid,
          username: dbUser.username || "",
          isFilledFromDB: true,
          about: dbUser.about || "",
          photoURL: dbUser.photoURL || authUser.photoURL || "",
          resumeURL: dbUser.resumeURL || "",
          firstName: dbUser.firstName || "",
          lastName: dbUser.lastName || "",
          email: dbUser.email || authUser.email || "",
          school: dbUser.school || "",
          hasCompletedReg: true,
          isApprovedApplicant: dbUser.isApprovedApplicant || false,
          isRejectedApplicant: dbUser.isRejectedApplicant || false,
          algos: dbUser.algos || undefined,
        };
        setUser(newUser);
      }
    });
  }, []);
  return <></>;
}
