"use client";
import { PhotoIcon, CheckIcon } from "@heroicons/react/24/solid";
import { Profile } from "@prisma/client";
import Link from "next/link";
import Swal from "sweetalert2";
import { useRef, useState } from "react";
import { useRouter } from "next/navigation";

export default function RegistrationForm(props: { user: any }) {
  const [resumeUploaded, setResumeUploaded] = useState(false);

  const router = useRouter();
  const handleResumeChange = async (selectedFile: any) => {
    if (!selectedFile) {
      return;
    }
    if (selectedFile.type !== "application/pdf") {
      Swal.fire({
        title: "Please upload a PDF",
        icon: "warning",
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
      return;
    }

    const resp = await fetch("/api/protected/db/user/uploadResume");
    const { url } = await resp.json();

    const uploadResp = await fetch(url, {
      method: "PUT",
      headers: {
        "Content-Type": "application/pdf",
      },
      body: selectedFile,
    });

    if (!uploadResp.ok) {
      Swal.fire({ icon: "error", title: "Error uploading resume" });
    } else {
      setResumeUploaded(true);
    }
  };

  // Uid will be ignored. We use it here because we don't want to make it an optional field
  const defaultProfile: Profile = {
    uid: props.user.sub,
    firstName: "",
    lastName: "",
    about: "",
    school: "Northwestern",
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

  const handleDrop = (e: any) => {
    e.preventDefault();
    setDragOver(false);

    const files = e.dataTransfer.files;
    handleResumeChange(files[0]);
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
      }
    });
  };

  const confirmEntriesAdded = () => {
    for (const [key, value] of Object.entries(profile)) {
      if (!value) {
        alertUnfilledField(key);
        return false;
      }
    }
    if (!resumeUploaded) {
      alertUnfilledField("Resume");
      return false;
    }
    return true;
  };

  // TODO: sanitize better
  const createNewUserHandler = async () => {
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
        <div className="border-y border-white/10 pb-12 pt-12">
          <h2 className="text-base font-semibold leading-7 text-white">
            Profile
          </h2>
          <p className="mt-1 text-sm leading-6 text-gray-400">
            This information will be displayed publicly so be careful what you
            share.
          </p>

          <div className="mt-10 grid grid-cols-1 gap-x-6 gap-y-8 sm:grid-cols-6">
            <div className="col-span-full">
              <label
                htmlFor="about"
                className="block text-sm font-medium leading-6 text-white">
                About
              </label>
              <div className="mt-2">
                <textarea
                  id="about"
                  name="about"
                  rows={3}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                  onChange={handleInputChange}
                />
              </div>
              <p className="mt-3 text-sm leading-6 text-gray-400">
                Write a few sentences about yourself.
              </p>
            </div>

            <div className="col-span-full">
              <label
                htmlFor="file-upload"
                className="block text-sm font-medium leading-6 text-white">
                Resume
              </label>
              <div
                className={
                  resumeUploaded
                    ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                    : isDragOver
                      ? "mt-2 flex justify-center rounded-lg border border-dashed border-indigo-500 px-6 py-10"
                      : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"
                }
                ref={dropRef}
                onDragOver={handleDragOver}
                onDragLeave={handleDragLeave}
                onDrop={handleDrop}>
                <div className="text-center">
                  {!resumeUploaded && (
                    <PhotoIcon
                      className="mx-auto h-12 w-12 text-gray-500"
                      aria-hidden="true"
                    />
                  )}
                  {resumeUploaded && (
                    <CheckIcon
                      className="mx-auto h-12 w-12 text-green-500"
                      aria-hidden="true"
                    />
                  )}

                  <div className="mt-4 flex text-sm leading-6 text-gray-400">
                    <label
                      htmlFor="file-upload"
                      className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500">
                      <span>Upload a file</span>
                      <input
                        id="file-upload"
                        name="file-upload"
                        type="file"
                        onChange={(e: any) => {
                          //@ts-ignore
                          handleResumeChange(e.target.files[0]);
                        }}
                        className="sr-only"
                      />
                    </label>
                    <p className="pl-1">or drag and drop</p>
                  </div>
                  <p className="text-xs leading-5 text-gray-400">
                    PDF up to 10MB
                  </p>
                </div>
              </div>
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
                <select
                  id="school"
                  name="school"
                  onChange={handleInputChange}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6 [&_*]:text-black">
                  <option>Northwestern</option>
                  <option>UChicago</option>
                  <option>Other</option>
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
    </div>
  );
}
