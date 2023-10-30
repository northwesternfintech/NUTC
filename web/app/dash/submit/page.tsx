"use client";
import { CheckIcon, PaperClipIcon } from "@heroicons/react/24/solid";
import axios from "axios";
import { useRef, useState } from "react";
import AlgorithmType from "@/app/dash/algoType";
import Swal from "sweetalert2";
import { push, ref, set } from "firebase/database";
import { getDownloadURL, ref as sRef, uploadBytes } from "firebase/storage";
import { useFirebase } from "@/app/firebase/context";
import { useUserInfo } from "@/app/login/auth/context";

async function uploadFile(
  database: any,
  storage: any,
  uid: string,
  file: File,
) {
  const fileNameWithoutType = file.type.split("/")[0];
  const fileRef = push(ref(database, `users/${uid}/${fileNameWithoutType}`));
  if (!fileRef) {
    return { downloadURL: "", fileIdKey: "", fileRef: "" };
  }
  const fileIdKey: string = `${uid}/${fileRef.key}` || ""; //bad practice
  const storageRef = sRef(storage);
  const fileType = file.type.split("/")[1];
  const storageFileRef = sRef(storageRef, `${fileNameWithoutType}/${fileIdKey}.${fileType}`);
  try {
    console.log("uploading");
    const snapshot = await uploadBytes(storageFileRef, file);
    const downloadURL = await getDownloadURL(snapshot.ref); //in theory, we should be saving the ID, rather than URL. this is easier.
    return { downloadURL, fileIdKey, fileRef };
  } catch (e) {
    console.log(e);
    return { downloadURL: "", fileIdKey: "", fileRef: "" };
  }
}

async function writeNewAlgo(
  algo: AlgorithmType,
  algoRef: any,
  database: any,
  uid: string,
) {
  if (algo.downloadURL === "" || algo.description === "" || algo.name === "") {
    Swal.fire({
      title: "Please fill out all fields",
      icon: "warning",
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
  algo.lintResults = "pending";
  algo.uploadDate = new Date().toISOString();

  await set(algoRef, algo);
  // await functions.httpsCallable("emailApplication")();
  // above should be lint function
  return true;
}

export default function Submission() {
  const defaultAlgo: AlgorithmType = {
    lintResults: "pending",
    uploadDate: "",
    downloadURL: "",
    fileIdKey: "",
    name: "",
    description: "",
  };

  const handleInputChange = (e: any) => {
    const { name, value } = e.target;
    //@ts-ignore
    setAlgo((prevState) => ({
      ...prevState,
      [name]: value,
    }));
  };

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
    // handleFileChange(files[0]);
  };

  const userInfo = useUserInfo();
  const { database, storage, functions } = useFirebase();

  const handleFileChange = async (file_name: string, selectedFile: any) => {
    if (!selectedFile) {
      return;
    }
    console.log(selectedFile);
    const fileName = selectedFile.name;
    // const fileExtension = fileName.split(".").pop().toLowerCase();
    // if (fileExtension !== "py") {
    //   Swal.fire({
    //     title: "Please upload a Python file",
    //     icon: "error",
    //     toast: true,
    //     position: "top-end",
    //     showConfirmButton: false,
    //     timer: 4000,
    //     timerProgressBar: true,
    //     didOpen: (toast) => {
    //       toast.addEventListener("mouseenter", Swal.stopTimer);
    //       toast.addEventListener("mouseleave", Swal.resumeTimer);
    //     },
    //   });
    //   return;
    // }
    const downloadLink = await uploadFile(
      database,
      storage,
      userInfo?.user?.uid || "unknown",
      selectedFile,
    );
    console.log("downloadLink", downloadLink);
    if (downloadLink.downloadURL !== "") {
      setAlgo((prevState) => ({
        ...prevState,
        downloadURL: downloadLink.downloadURL,
        fileIdKey: downloadLink.fileIdKey,
      }));
      setAlgoRef(downloadLink.fileRef);
      Swal.fire({
        title: "Algorithm uploaded!",
        icon: "success",
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
    } else {
      Swal.fire({
        title: "Algorithm upload failed",
        icon: "error",
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
    }
  };

  const [algo, setAlgo] = useState(defaultAlgo);
  const [algoRef, setAlgoRef]: any = useState(null);
  return (
    <div className="mx-auto max-w-7xl px-4 pb-24 pt-12 sm:px-6 sm:pb-32 sm:pt-16 lg:px-8">
      <div className="mx-auto max-w-2xl">
        <div className="space-y-12">
          <div className="border-b border-white/10 pb-12">
            <h2 className="text-base font-semibold leading-7 text-white">
              Submission Information
            </h2>
            <p className="mt-1 text-sm leading-6 text-gray-400">
              This information will help to identify your submission, as well as
              give our judges a better understanding of your project.
            </p>

            <div className="mt-10 grid grid-cols-1 gap-x-6 gap-y-8 sm:grid-cols-6">
              <div className="sm:col-span-4">
                <label
                  htmlFor="name"
                  className="block text-sm font-medium leading-6 text-white"
                >
                  Algorithm Name
                </label>
                <div className="mt-2">
                  <div className="flex rounded-md bg-white/5 ring-1 ring-inset ring-white/10 focus-within:ring-2 focus-within:ring-inset focus-within:ring-indigo-500">
                    <input
                      type="text"
                      name="name"
                      id="name"
                      onChange={handleInputChange}
                      className="flex-1 border-0 bg-transparent py-1.5 text-white focus:ring-0 sm:text-sm sm:leading-6"
                    />
                  </div>
                </div>
              </div>

              <div className="col-span-full">
                <label
                  htmlFor="description"
                  className="block text-sm font-medium leading-6 text-white"
                >
                  Description
                </label>
                <div className="mt-2">
                  <textarea
                    id="description"
                    name="description"
                    rows={3}
                    onChange={handleInputChange}
                    className="block w-full rounded-md border-0 bg-white/5 py-1.5 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 sm:text-sm sm:leading-6"
                    defaultValue={""}
                  />
                </div>
                <p className="mt-3 text-sm leading-6 text-gray-400">
                  Explain how your algorithm works.
                </p>
              </div>

              <div className="col-span-full">
                <label
                  htmlFor="algo-upload"
                  className="block text-sm font-medium leading-6 text-white"
                >
                  Algorithm Upload
                </label>
                <div
                  className={algo.downloadURL
                    ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                    : isDragOver
                    ? "mt-2 flex justify-center rounded-lg border border-solid border-indigo-500 px-6 py-10"
                    : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"}
                  ref={dropRef}
                  onDragOver={handleDragOver}
                  onDragLeave={handleDragLeave}
                  onDrop={handleDrop}
                >
                  <div className="text-center">
                    {!algo.downloadURL && (
                      <PaperClipIcon
                        className="mx-auto h-12 w-12 text-gray-500"
                        aria-hidden="true"
                      />
                    )}
                    {algo.downloadURL && (
                      <CheckIcon
                        className="mx-auto h-12 w-12 text-green-500"
                        aria-hidden="true"
                      />
                    )}
                    <div className="mt-4 flex text-sm leading-6 text-gray-400">
                      <label
                        htmlFor="file-upload"
                        className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500"
                      >
                        <span>Upload requirements.txt</span>
                        <input
                          id="file-upload"
                          name="file-upload"
                          onChange={(e) => {
                            //@ts-ignore
                            handleFileChange("requirements.txt", e.target.files[0]);
                          }}
                          multiple
                          type="file"
                          className="sr-only"
                        />
                      </label>
                      <p className="pl-1">or drag and drop</p>
                    </div>
                    <p className="text-xs leading-5 text-gray-400">
                      .txt up to 100KB
                    </p>
                  </div>
                </div>
              </div>
            </div>
          </div>

          <button
            type="submit"
            onClick={async () => {
              //@ts-ignore
              if (
                await writeNewAlgo(
                  algo,
                  algoRef,
                  database,
                  userInfo?.user?.uid || "",
                ) //bad practice, fix later
              ) {
                const res = axios.get(
                  `https://nutc-linter-4oeeau4rxa-uc.a.run.app/?uid=${userInfo?.user?.uid}&algo_id=${algoRef.key}`,
                );
                Swal.fire({
                  title: "Algorithm submitted!",
                  icon: "success",
                  timer: 2000,
                  timerProgressBar: true,
                  willClose: () => {
                    window.location.reload();
                  },
                });
                console.log(res);
              }
            }}
            className="rounded-md bg-indigo-500 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-400 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-500"
          >
            Submit
          </button>
        </div>
      </div>
    </div>
  );
}
