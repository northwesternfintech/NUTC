"use client";
import { PhotoIcon, CheckIcon } from "@heroicons/react/24/solid";
import { Profile } from "@prisma/client";
import Link from "next/link";
import Swal from "sweetalert2";
import { useRef, useState } from "react";
import { useRouter } from "next/navigation";

export default function RegistrationForm(props: { user: any }) {
  const router = useRouter();

  // Uid will be ignored. We use it here because we don't want to make it an optional field
  const defaultProfile: Profile = {
    uid: props.user.sub,
    firstName: "",
    lastName: "",
    about: "N/A",
    school: "",
    year: "2029"
  };

  const [profile, setProfile] = useState(defaultProfile);

  const handleInputChange = (e: any) => {
    const { name, value } = e.target;
    setProfile((prevState: any) => ({
      ...prevState,
      [name]: value,
    }));
  };

  // TODO: make own component
  const [isDragOver, setDragOver] = useState(false);
  const dropRef: any = useRef();

  const handleDragOver = (e: any) => {
    e.preventDefault();
    setDragOver(true);
  };

  const handleDragLeave = () => {
    setDragOver(false);
  };


  const alertUnfilledField = (fieldName: string) => {
    Swal.fire({
      title: "Please fill out all fields",
      icon: "warning",
      text: "Missing field: " + fieldName,
      toast: true,
      position: "top-end",
      showConfirmButton: false,
      timer: 4000,
      timerProgressBar: true,
      didOpen: toast => {
        toast.addEventListener("mouseenter", Swal.stopTimer);
        toast.addEventListener("mouseleave", Swal.resumeTimer);
      },
    });
  };

  const confirmEntriesAdded = () => {
    for (const [key, value] of Object.entries(profile)) {
      if (!value) {
        alertUnfilledField(key);
        return false;
      }
    }
    return true;
  };

  // TODO: sanitize better
  const createNewUserHandler = async () => {
    // Swal.fire({ icon: 'info', title: 'Applications closed for Fall 2024.' });
    // return;
    if (!confirmEntriesAdded()) return;

    const response = await fetch("/api/protected/db/user/createUser", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(profile),
    });

    if (!response.ok) {
      Swal.fire({
        title: "Error",
        icon: "error",
        text: "An error occurred",
        toast: true,
        position: "top-end",
        showConfirmButton: false,
        timer: 4000,
        timerProgressBar: true,
        didOpen: toast => {
          toast.addEventListener("mouseenter", Swal.stopTimer);
          toast.addEventListener("mouseleave", Swal.resumeTimer);
        },
      });
      const errMsg = await response.text();
      alert(errMsg);
      return false;
    }

    router.push("/dash");

    return true;
  };

  return (
    <div>
      <div className="space-y-12">
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
                className="block text-sm font-medium leading-6 text-white">
                First name
              </label>
              <div className="mt-2">
                <input
                  type="text"
                  name="firstName"
                  id="firstName"
                  autoComplete="given-name"
                  defaultValue={profile.firstName}
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                />
              </div>
            </div>

            <div className="sm:col-span-3">
              <label
                htmlFor="lastName"
                className="block text-sm font-medium leading-6 text-white">
                Last name
              </label>
              <div className="mt-2">
                <input
                  type="text"
                  name="lastName"
                  id="lastName"
                  autoComplete="family-name"
                  defaultValue={profile.lastName}
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                />
              </div>
            </div>

            <div className="sm:col-span-4">
              <label
                htmlFor="email"
                className="block text-sm font-medium leading-6 text-white">
                Email address
              </label>
              <div className="mt-2">
                <input
                  id="email"
                  name="email"
                  type="email"
                  value={props?.user?.email}
                  readOnly={true}
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm pointer-events-none ring-1 ring-inset ring-white/10 sm:text-sm sm:leading-6"
                />
              </div>
            </div>

            <div className="sm:col-span-3">
              <label
                htmlFor="school"
                className="block text-sm font-medium leading-6 text-white">
                School
              </label>
              <div className="mt-2">
                <input
                  type="text"
                  id="school"
                  name="school"
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6" />
              </div>
            </div>

            <div className="sm:col-span-2">
              <label
                htmlFor="year"
                className="block text-sm font-medium leading-6 text-white">
                Graduation Year
              </label>
              <div className="mt-2">
                <select
                  id="year"
                  name="year"
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6">
                  <option>2025</option>
                  <option>2026</option>
                  <option>2027</option>
                  <option>2028</option>
                  <option>2029</option>
                </select>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div className="mt-6 flex items-center justify-end gap-x-6">
        <Link
          href="/"
          type="button"
          className="text-sm font-semibold leading-6 text-white">
          Cancel
        </Link>
        <button
          type="submit"
          onClick={createNewUserHandler}
          className="rounded-md bg-indigo-500 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-400 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-500">
          Finish Registration
        </button>
      </div>
    </div >
  );
}
