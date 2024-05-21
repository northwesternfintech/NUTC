export default interface AlgorithmType {
  lintResults: string;
  uploadDate: string;
  downloadURL: string;
  fileIdKey: string;
  uploadTime: number;
  name: string;
  description: string;
  sandboxLogFileURL?: string;
  lintFailureMessage?: string;
  lintSuccessMessage?: string;
  caseType: string;
}

export interface UserInfoType {
  isFilledFromDB: boolean;
  uid: string;
  username: string;
  about: string;
  resumeURL: string;
  firstName: string;
  lastName: string;
  email: string;
  school: string;
  hasCompletedReg: boolean;
  isApprovedApplicant?: boolean;

  // Only set on the subservient user
  isPairedTo?: string;
  isInAGroup: boolean;
  isRejectedApplicant?: boolean;
  algos?: Map<string, AlgorithmType>;
  latestAlgoId?: string;
}

interface UserType {
  user: UserInfoType | undefined;
  setUser: (user: UserInfoType) => void;
}
