"use client";
import { useEffect } from "react";
import { useFirebase } from "@/app/firebase/context";
import { child, get, ref } from "firebase/database";
import { UserInfoType, useUserInfo } from "@/app/login/auth/context";
import { onAuthStateChanged } from "firebase/auth";
import { getAuth } from "firebase/auth";
import AlgorithmType from "@/app/dash/algoType";

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
        const entries = Object.entries(dbUser.algos || {});
        const sortedEntries = entries.sort((a, b) => {
          const dateA: any = new Date(a[1].uploadDate);
          const dateB: any = new Date(b[1].uploadDate);
          return dateB - dateA;
        });
        var map: Map<string, AlgorithmType> = new Map<string, AlgorithmType>();
        sortedEntries.forEach((entry) => {
          map.set(entry[0], entry[1]);
        });

        const newUser: UserInfoType = {
          uid: uid,
          username: dbUser.username || "",
          isFilledFromDB: true,
          about: dbUser.about || "",
          resumeURL: dbUser.resumeURL || "",
          firstName: dbUser.firstName || "",
          lastName: dbUser.lastName || "",
          email: dbUser.email || authUser.email || "",
          school: dbUser.school || "",
          hasCompletedReg: true,
          isApprovedApplicant: dbUser.isApprovedApplicant || false,
          isRejectedApplicant: dbUser.isRejectedApplicant || false,
          algos: map,
        };
        setUser(newUser);
      }
    });
  }, []);
  return <></>;
}
