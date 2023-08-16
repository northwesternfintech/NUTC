"use client"
import { useUserInfo } from "@/app/login/auth/context";

export default function EmailText() {
  const userInfo = useUserInfo();
  return (
    <div className="sm:col-span-4">
      <label
        htmlFor="email"
        className="block text-sm font-medium leading-6 text-white"
      >
        Email address
      </label>
      <div className="mt-2">
        <input
          id="email"
          name="email"
          type="email"
          defaultValue={userInfo.user?.email || ""}
          readOnly={userInfo.user?.email !== undefined}
          className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 sm:text-sm sm:leading-6"
        />
      </div>
    </div>
  );
};
