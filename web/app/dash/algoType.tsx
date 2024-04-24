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
}
