import { isLocalhost } from "@/app/firebase/config";

export function apiEndpoint(): string {
  if (isLocalhost()) {
    return "http://localhost:26389/";
  } else {
    return "https://directapi.nutc.io/";
  }
}
