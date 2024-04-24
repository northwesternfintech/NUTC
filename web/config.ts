import { isLocalhost } from "@/app/firebase/config";

export const config: any = {
  linterBaseUrl: "https://nutc-linter-4oeeau4rxa-uc.a.run.app",
};

export function sandboxEndpoint(uid: string, algo_id: string): string {
  if (isLocalhost()) {
    return `http://127.0.0.1:18080/sandbox/${uid}/${algo_id}/SANDBOX`;
  } else {
    return "none";
    // return `${config.linterBaseUrl}/?uid=${uid}&algo_id=${algo_id}`;
  }
}

export function linterEndpoint(uid: string, algo_id: string): string {
  if (isLocalhost()) {
    return `http://127.0.0.1:18081/?uid=${uid}&algo_id=${algo_id}`;
  } else {
    return `${config.linterBaseUrl}/?uid=${uid}&algo_id=${algo_id}`;
  }
}
