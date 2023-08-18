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
          username: "",
          about: "",
          photoURL: user.photoURL || "",
          resumeURL: "",
          firstName: "",
          lastName: "",
          email: user.email || "",
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
          about: dbUser.about || "",
          photoURL: dbUser.photoURL || user.photoURL || "",
          resumeURL: dbUser.resumeURL || "",
          firstName: dbUser.firstName || "",
          lastName: dbUser.lastName || "",
          email: dbUser.email || user.email || "",
          school: dbUser.school || "",
          hasCompletedReg: true,
          isApprovedApplicant: dbUser.isApprovedApplicant || false,
          isRejectedApplicant: dbUser.isRejectedApplicant || false,
        };
        setUser(newUser);
      }
    });
  }, []);
  return <></>;
}
