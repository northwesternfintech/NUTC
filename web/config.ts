import { isLocalhost } from "@/app/firebase/config";

export function emulatorEndpoint(): string {
    return "100.70.204.62";
}
export function apiEndpoint(): string {
  if (isLocalhost()) {
    return "http://100.70.204.62";
  } else {
    return "https://black.tailf7f8d.ts.net";
  }
}
