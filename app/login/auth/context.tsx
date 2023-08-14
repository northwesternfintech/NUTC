"use client";

import { createContext, ReactNode, useState, useContext } from "react";

export interface UserInfoType {
  uid: string;
  displayName: string;
  photoURL: string;
  email: string;
  emailVerified: boolean;
}

interface UserType {
  user: UserInfoType | undefined;
  setUser: (user: UserInfoType) => void;
}

const UserInfoContext = createContext<UserType | undefined>(
  undefined,
);

export const useUserInfo = () => {
  const context = useContext(UserInfoContext);
  if (!context) {
    throw new Error("useUserInfo must be used within a UserInfoProvider");
  }
  return context;
};

interface UserInfoProps {
  children: ReactNode;
}

export const UserInfoProvider: React.FC<UserInfoProps> = (
  { children },
) => {
  const [user, setUser] = useState<UserInfoType | undefined>(undefined);
  return (
    <UserInfoContext.Provider value={{ user, setUser }}>
      {children}
    </UserInfoContext.Provider>
  );
};
