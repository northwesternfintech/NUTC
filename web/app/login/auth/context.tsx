"use client";

import AlgorithmType from "@/app/dash/algoType";
import { createContext, ReactNode, useContext, useState } from "react";

export interface UserInfoType {
  isFilledFromDB: boolean;
  uid: string;
  username: string;
  about: string;
  teamLeader: string;
  firstName: string;
  lastName: string;
  email: string;
  school: string;
  hasCompletedReg: boolean;
  isApprovedApplicant?: boolean;
  isRejectedApplicant?: boolean;
  algos?: Map<string, AlgorithmType>;
  latestAlgoId?: string;
}

interface UserType {
  user: UserInfoType | undefined;
  setUser: (user: UserInfoType) => void;
}

const UserInfoContext = createContext<UserType | undefined>(undefined);

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

export const UserInfoProvider: React.FC<UserInfoProps> = ({ children }) => {
  const [user, setUser] = useState<UserInfoType | undefined>(undefined);
  return (
    <UserInfoContext.Provider value={{ user, setUser }}>
      {children}
    </UserInfoContext.Provider>
  );
};
