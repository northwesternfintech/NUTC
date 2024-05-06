"use client";
import { useUserInfo } from "@/app/login/auth/context";
import { useFirebase } from "@/app/firebase/context";
import Swal from "sweetalert2";
import { useState } from "react";

export default function Group() {
  const [iv, setIv] = useState("");
  const handleChange = (event: any) => {
    setIv(event.target.value);
  };
  const userInfo = useUserInfo();
  const { functions } = useFirebase();
  if (!userInfo?.user?.isInAGroup) {
    return (
      <div className="bg-gray-900">
        <div className="border-b border-white/10 pb-10 mx-auto max-w-7xl px-6 lg:px-8 pt-6 sm:pt-12 lg:pt-20 pb-12">
          <div className="mx-auto max-w-4xl">
            <h2 className="text-2xl font-bold leading-10 tracking-tight text-white">
              Add a partner
            </h2>
            <p className="mt-2 text-base leading-7 text-gray-300">
              Share your group ID with your partner
            </p>
            <p className="text-red-500">
              <span className="font-semibold">IMPORTANT:</span> Do not share
              your Group ID with anyone except your group member.
            </p>
            <br />
            <p className="text-base leading-7 text-gray-300">
              Your Group ID:{" "}
              <span className="font-semibold">
                {userInfo.user?.uid ?? "You must be logged in"}
              </span>
            </p>
          </div>
        </div>

        <div className="border-b border-white/10 pb-10 mx-auto max-w-7xl px-6 lg:px-8 pt-4 sm:pt-6 lg:pt-10 pb-12">
          <div className="mx-auto max-w-4xl">
            <h2 className="text-2xl font-bold leading-10 tracking-tight text-white">
              Join a group
            </h2>
            <p className="mt-2 text-base leading-7 text-gray-300">
              Enter your partner&apos;s group ID
            </p>
            <br />
            <div className="mt-2 flex flex-row justify-left gap-4">
              <input
                type="text"
                value={iv}
                onChange={handleChange}
                id="other_uid"
                className="block w-2/5 rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                placeholder="Group ID"
              />
              <button
                type="button"
                onClick={async () => {
                  const res = await functions.httpsCallable("addUserToGroup")({
                    groupId: iv,
                  });
                  if (res.data) {
                    Swal.fire({
                      icon: "success",
                      title: "Successfully added to group",
                    });
                    location.reload();
                  } else {
                    Swal.fire({
                      icon: "error",
                      title: "Error adding to group",
                      text: "Double-check the group ID. If you continue to experience errors, contact support@nutc.io",
                    });
                  }
                }}
                className="rounded-md bg-indigo-600 px-2 py-1 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
              >
                Join Group
              </button>
            </div>
            <p className="mt-2 text-sm text-gray-500" id="email-description">
              Above section on your partner&apos;s dashboard
            </p>
          </div>
        </div>
      </div>
    );
  } else {
    return (
      <div className="bg-gray-900 px-6 py-24 sm:py-32 lg:px-8">
        <div className="mx-auto max-w-2xl text-center">
          <h2 className="text-3xl font-bold tracking-tight text-white sm:text-6xl">
            You are already in a group.
          </h2>
        </div>
      </div>
    );
  }
}
