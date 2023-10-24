import Submission from "./submit/page";

export  interface AlgorithmType {
  lintResults: string;
  uploadDate: string;
  name: string;
  description: string;
  lintFailureMessage?: string;
  lintSuccessMessage?: string;
}
export  interface SubmissionFile {
  downloadURL: string;
  fileIdKey: string;
  fileName: string;
}
export type FirebaseUploadType = AlgorithmType & SubmissionFile[];