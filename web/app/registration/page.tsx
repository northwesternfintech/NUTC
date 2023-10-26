"use client";
import {
  PhotoIcon,
  CheckIcon,
  UserCircleIcon,
} from "@heroicons/react/24/solid";
import Link from "next/link";
import { useEffect, useState } from "react";
import { UserInfoType, useUserInfo } from "@/app/login/auth/context";
import { useFirebase } from "@/app/firebase/context";
import { ref, update } from "firebase/database";
import Swal from "sweetalert2";

async function writeNewUser(functions: any, database: any, user: UserInfoType) {
  //iterate over fields in user
  for (const [key, value] of Object.entries(user)) {
    if (!(key === "isFilledFromDB") && !value) {
      Swal.fire({
        title: "Please fill out all fields",
        icon: "warning",
        text: "Missing field: " + key,
        toast: true,
        position: "top-end",
        showConfirmButton: false,
        timer: 4000,
        timerProgressBar: true,
        didOpen: (toast) => {
          toast.addEventListener("mouseenter", Swal.stopTimer);
          toast.addEventListener("mouseleave", Swal.resumeTimer);
        },
      });
      return false;
    }
  }
  user.isApprovedApplicant = true; // auto accept for FinRL
  await update(ref(database, "users/" + user.uid), user);
  await functions.httpsCallable("emailApplication")();
  return true;
}

export default function Registration() {
  const { database, storage, functions } = useFirebase();
  const userInfo = useUserInfo();
  const defaultUser: UserInfoType = {
    uid: userInfo.user?.uid || "-1",
    isFilledFromDB: false,
    username: "",
    about: "",
    teamLeader: "",
    firstName: "",
    lastName: "",
    email: "",
    school: "",
    ICAIFRegistrationNumber: "",
    hasCompletedReg: true, //will be after this
  };

  const [currUser, setCurrUser] = useState(defaultUser);

  useEffect(() => {
    var currU = { ...currUser };
    if (!userInfo.user?.email) return;
    currU.email = userInfo.user?.email || "";
    currU.uid = userInfo.user?.uid || "-1";
    setCurrUser(currU);
  }, [userInfo]);

  const handleInputChange = (e: any) => {
    const { name, value } = e.target;
    //@ts-ignore
    setCurrUser((prevState) => ({
      ...prevState,
      [name]: value,
    }));
  };

  return (
    <div>
      <div className="space-y-12">
        <div className="border-y border-white/10 pb-12 pt-12">
          <h2 className="text-base font-semibold leading-7 text-white">
            Profile
          </h2>
          <p className="mt-1 text-sm leading-6 text-gray-400">
            This information will be displayed publicly so be careful what you
            share.
          </p>

          <div className="mt-10 grid grid-cols-1 gap-x-6 gap-y-8 sm:grid-cols-6">
            <div className="sm:col-span-4">
              <label
                htmlFor="username"
                className="block text-sm font-medium leading-6 text-white"
              >
                Username
              </label>
              <div className="mt-2">
                <div className="flex rounded-md bg-white/5 ring-1 ring-inset ring-white/10 focus-within:ring-2 focus-within:ring-inset focus-within:ring-indigo-500">
                  <span className="flex select-none items-center pl-3 text-gray-500 sm:text-sm">
                    finrl-contest-2023.web.app/users/
                  </span>
                  <input
                    type="text"
                    name="username"
                    id="username"
                    autoComplete="username"
                    className="flex-1 border-0 bg-transparent py-1.5 pl-1 text-white focus:ring-0 sm:text-sm sm:leading-6"
                    placeholder="username"
                    defaultValue={currUser.username}
                    onChange={handleInputChange}
                  />
                </div>
              </div>
            </div>

            <div className="col-span-full">
              <label
                htmlFor="about"
                className="block text-sm font-medium leading-6 text-white"
              >
                About
              </label>
              <div className="mt-2">
                <textarea
                  id="about"
                  name="about"
                  rows={3}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                  defaultValue={currUser.about}
                  onChange={handleInputChange}
                />
              </div>
              <p className="mt-3 text-sm leading-6 text-gray-400">
                Write a few sentences about yourself.
              </p>
            </div>
          </div>
        </div>

        <div className="border-b border-white/10 pb-12">
          <h2 className="text-base font-semibold leading-7 text-white">
            Personal Information
          </h2>
          <p className="mt-1 text-sm leading-6 text-gray-400">
            Be careful to enter correct information, or you may be inelligible
            for awards.
          </p>

          <div className="mt-10 grid grid-cols-1 gap-x-6 gap-y-8 sm:grid-cols-6">
            <div className="sm:col-span-3">
              <label
                htmlFor="firstName"
                className="block text-sm font-medium leading-6 text-white"
              >
                First name
              </label>
              <div className="mt-2">
                <input
                  type="text"
                  name="firstName"
                  id="firstName"
                  autoComplete="given-name"
                  defaultValue={currUser.firstName}
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                />
              </div>
            </div>

            <div className="sm:col-span-3">
              <label
                htmlFor="lastName"
                className="block text-sm font-medium leading-6 text-white"
              >
                Last name
              </label>
              <div className="mt-2">
                <input
                  type="text"
                  name="lastName"
                  id="lastName"
                  autoComplete="family-name"
                  defaultValue={currUser.lastName}
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                />
              </div>
            </div>

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
                  value={currUser.email}
                  readOnly={true}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 sm:text-sm sm:leading-6"
                />
              </div>
            </div>

            <div className="sm:col-span-3">
              <label
                htmlFor="team-leader"
                className="block text-sm font-medium leading-6 text-white"
              >
                Team name
              </label>
              <div className="mt-2">
                <input
                  id="teamLeader"
                  name="teamLeader"
                  type="text"
                  value={currUser.teamLeader}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 sm:text-sm sm:leading-6"
                  onChange={handleInputChange}
                />
              </div>
            </div>

            <div className="sm:col-span-3">
              <label
                htmlFor="school"
                className="block text-sm font-medium leading-6 text-white"
              >
                Institution
              </label>
              <div className="mt-2">
                <input
                  id="school"
                  name="school"
                  type="text"
                  value={currUser.school}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 sm:text-sm sm:leading-6"
                  onChange={handleInputChange}
                />
              </div>
            </div>

        {/* <div className="border-b border-white/10 pb-12">
          <h2 className="text-base font-semibold leading-7 text-white">
            Notifications
          </h2>
          <p className="mt-1 text-sm leading-6 text-gray-400">
            We&#39;ll always let you know about important updates, but you pick
            what else you want to hear about.
          </p> */}
{/* 
          <div className="mt-10 space-y-10">
            <fieldset>
              <legend className="text-sm font-semibold leading-6 text-white">
                By Email
              </legend>
              <div className="mt-6 space-y-6">
                <div className="relative flex gap-x-3">
                  <div className="flex h-6 items-center">
                    <input
                      id="test-runs"
                      name="test-runs"
                      type="checkbox"
                      className="h-4 w-4 rounded border-white/10 bg-white/5 text-indigo-600 focus:ring-indigo-600 focus:ring-offset-gray-900"
                    />
                  </div>
                  <div className="text-sm leading-6">
                    <label
                      htmlFor="test-runs"
                      className="font-medium text-white"
                    >
                      Test Runs
                    </label>
                    <p className="text-gray-400">
                      Get notified when your script passes or fails linting
                    </p>
                  </div>
                </div>
                <div className="relative flex gap-x-3">
                  <div className="flex h-6 items-center">
                    <input
                      id="results"
                      name="results"
                      type="checkbox"
                      className="h-4 w-4 rounded border-white/10 bg-white/5 text-indigo-600 focus:ring-indigo-600 focus:ring-offset-gray-900"
                    />
                  </div>
                  <div className="text-sm leading-6">
                    <label htmlFor="results" className="font-medium text-white">
                      Results
                    </label>
                    <p className="text-gray-400">
                      Get notified when results are announced
                    </p>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div> */}
      </div>

      <div className="mt-6 flex items-center justify-end gap-x-6">
        <Link
          href="/"
          type="button"
          className="text-sm font-semibold leading-6 text-white"
        >
          Cancel
        </Link>
        <button
          type="submit"
          onClick={async () => {
            if (await writeNewUser(functions, database, currUser)) {
              userInfo.setUser(currUser);
            }
          }}
          className="rounded-md bg-indigo-500 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-400 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-500"
        >
          Finish Registration
        </button>
      </div>
    </div>
  );
}
