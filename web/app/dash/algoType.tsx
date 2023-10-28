export interface AlgorithmType {
  lintResults: string;
  uploadDate: string;
  name: string;
  description: string;
  lintFailureMessage?: string;
  lintSuccessMessage?: string;
  task?: Task;
}
export interface SubmissionFile {
  downloadURL: string;
  fileIdKey: string;
  fileName: string;
}
export type Task = "Task I" | "Task II";
