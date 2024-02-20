export default interface AlgorithmType {
  lintResults: string;
  uploadDate: string;
  downloadURL: string;
  fileIdKey: string;
  name: string;
  description: string;
  sandboxLogFileURL?: string;
  lintFailureMessage?: string;
  lintSuccessMessage?: string;
}
