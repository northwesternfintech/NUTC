"use client";

import { createContext, ReactNode, useContext } from "react";
import { app, database, functions, storage } from "@/app/firebase/config";
import { GithubAuthProvider, GoogleAuthProvider } from "firebase/auth";

interface FirebaseContextType {
  app: any;
  googleProvider: any;
  githubProvider: any;
  database: any;
  storage: any;
  functions: any;
}

const FirebaseContext = createContext<FirebaseContextType | undefined>(
  undefined,
);

export const useFirebase = () => {
  const context = useContext(FirebaseContext);
  if (!context) {
    throw new Error("useFirebase must be used within a firebaseProvider");
  }
  return context;
};

interface FirebaseProviderProps {
  children: ReactNode;
}

export const FirebaseProvider: React.FC<FirebaseProviderProps> = (
  { children },
) => {
  const googleProvider = new GoogleAuthProvider();
  const githubProvider = new GithubAuthProvider();
  googleProvider.addScope("https://www.googleapis.com/auth/userinfo.profile");
  return (
    <FirebaseContext.Provider
      value={{
        app,
        githubProvider,
        googleProvider,
        database,
        storage,
        functions,
      }}
    >
      {children}
    </FirebaseContext.Provider>
  );
};
