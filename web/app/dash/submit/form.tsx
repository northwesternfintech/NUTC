"use client";
import React, { useRef } from "react";
import {
  CheckIcon,
  PaperClipIcon,
  ChevronUpDownIcon,
} from "@heroicons/react/24/solid";
import { Fragment, useState } from "react";
import Swal from "sweetalert2";
import {
  Label,
  Listbox,
  ListboxButton,
  ListboxOption,
  ListboxOptions,
  Transition,
} from "@headlessui/react";
import { SubmitHandler, useForm } from "react-hook-form";
import { useRouter } from "next/navigation";

const CASES = [
  { id: 1, name: "HFT" },
  { id: 2, name: "Crypto Trading" },
];

const LANGUAGES = [
  { id: 1, name: "Python" },
  { id: 2, name: "C++" },
];

function classNames(...classes: any) {
  return classes.filter(Boolean).join(" ");
}

const CASE_DOCUMENT_URL =
  "https://docs.google.com/document/d/1FfWrKIXGO7oPKTTTwyprH3kM8WrnIuZmp9kcH4lo6CA/";

export default function SubmissionForm(props: { user: any }) {
  const router = useRouter();
  const [isDragOver, setDragOver] = useState(false);
  const fileSubmitRef = useRef<HTMLInputElement>(null);

  type Inputs = {
    name: string;
    case: string;
    language: string;
    description: string;
    algoFileS3Key: string;
  };

  const { handleSubmit, register, watch, setValue, resetField } =
    useForm<Inputs>({
      defaultValues: {
        name: "",
        case: "HFT",
        language: "Python",
        description: "",
        algoFileS3Key: "",
      },
    });

  const onSubmit: SubmitHandler<Inputs> = async data => {
    // Swal.fire({ title: "Submissions have closed", icon: 'warning' });
    // return;
    const responsePromise = fetch("/api/protected/db/user/createAlgo", {
      method: "POST",
      body: JSON.stringify(data),
    });
    Swal.fire({
      title: "Algorithm submitted. Waiting for initial results...",
      text: "This may take up to 30 seconds.",
      icon: "info",
      allowOutsideClick: false,
      allowEscapeKey: false,
      allowEnterKey: false,
    });
    Swal.showLoading();

    const response = await responsePromise;
    if (!response.ok) {
      Swal.fire({ title: "Error", icon: "error", text: "Server error. Please contact the nuft team in the piazza" });
      return;
    }
    Swal.close();
    var { success, message } = await response.json();
    message = message.replace(/\\n/g, "<br />");
    if (!success) {
      Swal.fire({
        title: "Compilation/Linting Error",
        icon: "error",
        html: message,
        width: 800,
        showConfirmButton: true,
      });
    } else {
      Swal.fire({
        title: "Linting complete!",
        text: "View results in the dashboard.",
        icon: "success",
        timer: 2000,
        timerProgressBar: true,
        willClose: () => {
          window.location.href = "submissions/" + data.algoFileS3Key;
        },
      });
    }

  };

  const [caseValue, languageValue, algoFileS3Key] = watch([
    "case",
    "language",
    "algoFileS3Key",
  ]);

  const handleAlgoChange = async (file: File) => {
    const fileExtension = file.name.split(".").pop()?.toLowerCase();

    if (
      (languageValue === "Python" && fileExtension !== "py") ||
      (languageValue === "C++" &&
        fileExtension !== "h" &&
        fileExtension !== "hpp")
    ) {
      Swal.fire({
        title: `Please upload a ${languageValue} file`,
        icon: "error",
        toast: true,
        position: "top-end",
        showConfirmButton: false,
        timer: 2000,
        timerProgressBar: true,
        didOpen: toast => {
          toast.addEventListener("mouseenter", Swal.stopTimer);
          toast.addEventListener("mouseleave", Swal.resumeTimer);
        },
      });
      return;
    }

    const resp = await fetch("/api/protected/db/user/uploadAlgoFile", {
      method: "POST",
    });
    const { url, key } = await resp.json();

    const uploadResp = await fetch(url, {
      method: "PUT",
      headers: {
        "Content-Type": "text/x-python",
      },
      body: file,
    });

    if (!uploadResp.ok) {
      Swal.fire({ icon: "error", title: "Error uploading algorithm" });
    } else {
      setValue("algoFileS3Key", key);
    }
  };

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
    handleAlgoChange(files[0]);
  };

  return (
    <div className="mx-auto max-w-7xl px-4 pb-24 pt-12 sm:px-6 sm:pb-32 sm:pt-16 lg:px-8">
      <div className="mx-auto max-w-2xl">
        <div className="space-y-12">
          <div>
            <h2 className="text-base font-semibold leading-7 text-white">
              Submission Information
            </h2>
            <p className="mt-1 text-sm leading-6 text-gray-400">
              This information will help to identify your submission, as well as
              give our judges a better understanding of your project.
            </p>
          </div>

          <form
            className="flex gap-12 flex-col"
            onSubmit={handleSubmit(onSubmit)}>
            <div className="flex flex-col gap-2">
              <label
                htmlFor="name"
                className="block text-sm font-medium leading-6 text-white">
                Algorithm Name
              </label>
              <div className="rounded-md bg-white/5 ring-1 ring-inset ring-white/10 focus-within:ring-2 focus-within:ring-inset focus-within:ring-indigo-500">
                <input
                  type="text"
                  id="name"
                  className="flex-1 border-0 bg-transparent py-1.5 text-white focus:ring-0 sm:text-sm sm:leading-6"
                  {...register("name")}
                />
              </div>
            </div>

            <Listbox value={caseValue} onChange={v => setValue("case", v)}>
              {({ open }) => (
                <div className="flex flex-col gap-2">
                  <div className="flex flex-cols items-center gap-x-1">
                    <Label className="block text-sm font-medium leading-6 text-white">
                      Case:
                    </Label>
                    <a href={CASE_DOCUMENT_URL} target="_blank">
                      <QuestionMarkSVG className="w-4 h-4 opacity-90" />
                    </a>
                  </div>

                  <div className="relative ring-white">
                    <ListboxButton className="w-full cursor-default rounded-md bg-white/5 py-1.5 pl-3 pr-10 text-left text-white shadow-sm ring-1 ring-inset ring-gray-300 focus:outline-none focus:ring-2 focus:ring-indigo-600 sm:text-sm sm:leading-6">
                      <span className="block truncate">{caseValue}</span>
                      <span className="pointer-events-none absolute inset-y-0 right-0 flex items-center pr-2">
                        <ChevronUpDownIcon
                          className="h-5 w-5 text-gray-400"
                          aria-hidden="true"
                        />
                      </span>
                    </ListboxButton>

                    <Transition
                      show={open}
                      as={Fragment}
                      leave="transition ease-in duration-100"
                      leaveFrom="opacity-100"
                      leaveTo="opacity-0">
                      <ListboxOptions className="absolute z-10 mt-1 max-h-60 w-full overflow-auto rounded-md bg-gray-900 py-1 text-base shadow-lg ring-1 ring-black ring-opacity-5 focus:outline-none sm:text-sm">
                        {CASES.map(caseOption => (
                          <ListboxOption
                            key={caseOption.id}
                            value={caseOption.name}
                            className={({ focus }) =>
                              classNames(
                                focus
                                  ? "bg-indigo-600 text-white"
                                  : "text-white",
                                "relative cursor-default select-none py-2 pl-3 pr-9",
                              )
                            }>
                            {({ selected, focus }) => (
                              <>
                                <span
                                  className={classNames(
                                    selected ? "font-semibold" : "font-normal",
                                    "block truncate",
                                  )}>
                                  {caseOption.name}
                                </span>

                                {selected ? (
                                  <span
                                    className={classNames(
                                      focus ? "text-white" : "text-indigo-600",
                                      "absolute inset-y-0 right-0 flex items-center pr-4",
                                    )}>
                                    <CheckIcon
                                      className="h-5 w-5"
                                      aria-hidden="true"
                                    />
                                  </span>
                                ) : null}
                              </>
                            )}
                          </ListboxOption>
                        ))}
                      </ListboxOptions>
                    </Transition>
                  </div>
                </div>
              )}
            </Listbox>

            <Listbox
              value={languageValue}
              onChange={v => {
                // clear the file input
                if (fileSubmitRef.current) {
                  fileSubmitRef.current.value = "";
                }

                setValue("algoFileS3Key", "");
                setValue("language", v);
              }}>
              {({ open }) => (
                <div className="flex flex-col gap-2">
                  <div className="flex flex-cols items-center gap-x-1">
                    <Label className="block text-sm font-medium leading-6 text-white">
                      Language:
                    </Label>
                    <a href={CASE_DOCUMENT_URL} target="_blank">
                      <QuestionMarkSVG className="w-4 h-4 opacity-90" />
                    </a>
                  </div>

                  <div className="relative ring-white">
                    <ListboxButton className="w-full cursor-default rounded-md bg-white/5 py-1.5 pl-3 pr-10 text-left text-white shadow-sm ring-1 ring-inset ring-gray-300 focus:outline-none focus:ring-2 focus:ring-indigo-600 sm:text-sm sm:leading-6">
                      <span className="block truncate">{languageValue}</span>
                      <span className="pointer-events-none absolute inset-y-0 right-0 flex items-center pr-2">
                        <ChevronUpDownIcon
                          className="h-5 w-5 text-gray-400"
                          aria-hidden="true"
                        />
                      </span>
                    </ListboxButton>

                    <Transition
                      show={open}
                      as={Fragment}
                      leave="transition ease-in duration-100"
                      leaveFrom="opacity-100"
                      leaveTo="opacity-0">
                      <ListboxOptions className="absolute z-10 mt-1 max-h-60 w-full overflow-auto rounded-md bg-gray-900 py-1 text-base shadow-lg ring-1 ring-black ring-opacity-5 focus:outline-none sm:text-sm">
                        {LANGUAGES.map(languageOption => (
                          <ListboxOption
                            key={languageOption.id}
                            value={languageOption.name}
                            className={({ focus }) =>
                              classNames(
                                focus
                                  ? "bg-indigo-600 text-white"
                                  : "text-white",
                                "relative cursor-default select-none py-2 pl-3 pr-9",
                              )
                            }>
                            {({ selected, focus }) => (
                              <>
                                <span
                                  className={classNames(
                                    selected ? "font-semibold" : "font-normal",
                                    "block truncate",
                                  )}>
                                  {languageOption.name}
                                </span>

                                {selected ? (
                                  <span
                                    className={classNames(
                                      focus ? "text-white" : "text-indigo-600",
                                      "absolute inset-y-0 right-0 flex items-center pr-4",
                                    )}>
                                    <CheckIcon
                                      className="h-5 w-5"
                                      aria-hidden="true"
                                    />
                                  </span>
                                ) : null}
                              </>
                            )}
                          </ListboxOption>
                        ))}
                      </ListboxOptions>
                    </Transition>
                  </div>
                </div>
              )}
            </Listbox>

            <div className="flex flex-col gap-2">
              <label
                htmlFor="description"
                className="block text-sm font-medium leading-6 text-white">
                Description
              </label>
              <div>
                <textarea
                  id="description"
                  rows={3}
                  className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                  {...register("description")}
                />
                <p className="mt-3 text-sm leading-6 text-gray-400">
                  Explain how your algorithm works.
                </p>
              </div>
            </div>

            <div className="col-span-full">
              <label
                htmlFor="algo-upload"
                className="block text-sm font-medium leading-6 text-white">
                Algorithm Upload
              </label>
              <div
                className={
                  algoFileS3Key
                    ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                    : isDragOver
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-indigo-500 px-6 py-10"
                      : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"
                }
                onDragOver={handleDragOver}
                onDragLeave={handleDragLeave}
                onDrop={handleDrop}>
                <div className="text-center">
                  {algoFileS3Key ? (
                    <CheckIcon
                      className="mx-auto h-12 w-12 text-green-500"
                      aria-hidden="true"
                    />
                  ) : (
                    <PaperClipIcon
                      className="mx-auto h-12 w-12 text-gray-500"
                      aria-hidden="true"
                    />
                  )}

                  <div className="mt-4 flex text-sm leading-6 text-gray-400">
                    <label
                      htmlFor="file-upload"
                      className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500">
                      <span>Upload a file</span>
                      <input
                        ref={fileSubmitRef}
                        id="file-upload"
                        name="file-upload"
                        onChange={e => {
                          if (e.target.files?.[0])
                            handleAlgoChange(e.target.files[0]);
                        }}
                        type="file"
                        className="sr-only"
                      />
                    </label>
                    <p className="pl-1">or drag and drop</p>
                  </div>
                  <p className="text-xs leading-5 text-gray-400">
                    {languageValue === "Python"
                      ? ".py up to 100KB"
                      : ".h up to 100KB"}
                  </p>
                </div>
              </div>
            </div>

            <div className="border-t border-white/10 pt-6 -mt-4">
              <button className="max-w-min rounded-md bg-indigo-500 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-400 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-500">
                Submit
              </button>
            </div>
          </form>
        </div>
      </div>
    </div>
  );
}

const QuestionMarkSVG = ({ className }: { className?: string }) => {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      fill="none"
      viewBox="0 0 24 24"
      strokeWidth={1.5}
      stroke="currentColor"
      className={className}>
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        d="M9.879 7.519c1.171-1.025 3.071-1.025 4.242 0 1.172 1.025 1.172 2.687 0 3.712-.203.179-.43.326-.67.442-.745.361-1.45.999-1.45 1.827v.75M21 12a9 9 0 1 1-18 0 9 9 0 0 1 18 0Zm-9 5.25h.008v.008H12v-.008Z"
      />
    </svg>
  );
};
