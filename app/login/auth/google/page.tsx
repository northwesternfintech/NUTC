"use client";
import { getAuth, GoogleAuthProvider, signInWithPopup } from "firebase/auth";
import { useFirebase } from "@/app/firebase/context";
import { UserInfoType, useUserInfo } from "@/app/login/auth/context";

export default function GoogleLogin() {
  const provider = useFirebase().provider;
  const { user, setUser } = useUserInfo();
  return (
    <button
      onClick={() => signinPopup(provider, user, setUser)}
      className="transitions-color duration-300 ease-in-out hover:bg-gray-400 flex w-full items-center justify-center gap-3 rounded-md bg-[#FFFFFF] px-3 py-1.5 text-gray-800 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-[#FFFFFF]"
    >
      <svg
        className="h-5 w-5"
        aria-hidden="true"
        fill="currentColor"
        viewBox="0 0 24 24"
      >
        <path d="M7 11v2.4h3.97c-.16 1.029-1.2 3.02-3.97 3.02-2.39 0-4.34-1.979-4.34-4.42 0-2.44 1.95-4.42 4.34-4.42 1.36 0 2.27.58 2.79 1.08l1.9-1.83c-1.22-1.14-2.8-1.83-4.69-1.83-3.87 0-7 3.13-7 7s3.13 7 7 7c4.04 0 6.721-2.84 6.721-6.84 0-.46-.051-.81-.111-1.16h-6.61zm0 0 17 2h-3v3h-2v-3h-3v-2h3v-3h2v3h3v2z" />
      </svg>
      <span className="text-sm font-semibold leading-6">Google</span>
    </button>
  );
}

function signinPopup(provider: any, user: UserInfoType | undefined, setUser: (user: UserInfoType) => void) {
  const auth = getAuth();
  signInWithPopup(auth, provider)
    .then((result) => {
      const credential = GoogleAuthProvider.credentialFromResult(result);
      const token = credential.accessToken;
      const user = result.user;
      const newUser: UserInfoType = {
        uid: user.uid || "",
        displayName: user.displayName || "",
        photoURL: user.photoURL || "",
        email: user.email || "",
        emailVerified: user.emailVerified || true,
      };
      setUser(newUser);
      console.log("Set user");
    }).catch((error) => {
      const errorCode = error.code;
      const errorMessage = error.message;
      const email = error.customData.email;
      const credential = GoogleAuthProvider.credentialFromError(error);
    });
}
