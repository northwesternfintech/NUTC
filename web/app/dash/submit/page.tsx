"use client";
import { CheckIcon, PaperClipIcon } from "@heroicons/react/24/solid";
import axios from "axios";
import { useEffect, useRef, useState } from "react";
import type { AlgorithmType, FirebaseUploadType, SubmissionFile } from "@/app/dash/algoType";
import Swal from "sweetalert2";
import { push, ref, set, update } from "firebase/database";
import { getDownloadURL, ref as sRef, uploadBytes } from "firebase/storage";
import { useFirebase } from "@/app/firebase/context";
import { useUserInfo } from "@/app/login/auth/context";

async function uploadFile(
  database: any,
  storage: any,
  uid: string,
  file: File,
  uploadCategory: string
) {
  const fileExtension = file.name.split(".").pop();
  const fileRef = push(ref(database, `users/${uid}/algos`));
  if (!fileRef) {
    return { downloadURL: "", fileIdKey: "", fileRef: "" };
  }
  const fileIdKey: string = `${uid}/${fileRef.key}` || ""; //bad practice
  const storageRef = sRef(storage);
  const storageFileRef = sRef(
    storageRef,
    `${uploadCategory}/${fileIdKey}.${fileExtension}`
  );
  try {
    const snapshot = await uploadBytes(storageFileRef, file);
    const downloadURL = await getDownloadURL(snapshot.ref); //in theory, we should be saving the ID, rather than URL. this is easier.
    return { downloadURL, fileIdKey, fileRef };
  } catch (e) {
    console.log(e);
    return { downloadURL: "", fileIdKey: "", fileRef: "" };
  }
}

// ├── Finrl-contest
// │ ├── trained_models # Your trained PPO weights
// │ ├── test.py # File for testing your submitted PPO model
// │ ├── readme.md # File to explain the your code
// │ ├── requirements.txt # Have it if adding any new packages
// │ ├── And any additional scripts you create

async function writeNewAlgo(
  algo: AlgorithmType,
  submissionFiles: SubmissionFile[],
  algoRef: any,
  database: any,
  uid: string
) {
  let hasEmptyFile = false;
  for (let i = 0; i < 4; i++) {
    if (submissionFiles[i].downloadURL === "") {
      hasEmptyFile = true;
    }
  }
  if (
    hasEmptyFile ||
    algo.description === "" ||
    algo.name === ""
  ) {
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

  await set(algoRef, { ...algo, ...submissionFiles });
 
  return true;
}

export default function Submission() {
  const defaultAlgo: AlgorithmType = {
    lintResults: "pending",
    uploadDate: "",
    name: "",
    description: "",
  };

  const [submissionFiles, setSubmissionFiles] = useState<SubmissionFile[]>([
    {
      fileName: "requirements.txt",
      downloadURL: "",
      fileIdKey: "",
    },
    {
      fileName: "test.py",
      downloadURL: "",
      fileIdKey: "",
    },
    {
      fileName: "trained_ppo.zip",
      downloadURL: "",
      fileIdKey: "",
    },
    {
      fileName: "readme.md",
      downloadURL: "",
      fileIdKey: "",
    },
  ]);

  const handleInputChange = (e: any) => {
    const { name, value } = e.target;
    //@ts-ignore
    setAlgo((prevState) => ({
      ...prevState,
      [name]: value,
    }));
  };

  const [isDragOver, setDragOver] = useState([false, false, false, false]);
  const refs = [useRef<any>(), useRef<any>(), useRef<any>(), useRef<any>()];

  const handleDragOver = (e: any, index: number) => {
    e.preventDefault();
    setDragOver((prevState) => {
      const updatedState = [...prevState];
      updatedState[index] = true;
      return updatedState;
    });
  };

  const handleDragLeave = (index: number) => {
    setDragOver((prevState) => {
      const updatedState = [...prevState];
      updatedState[index] = false;
      return updatedState;
    });
  };

  const handleDrop = (
    e: any,
    uploadCategory: string,
    fileType: string,
    uploadIndex: number
  ) => {
    e.preventDefault();
    setDragOver((prevState) => {
      const updatedState = [...prevState];
      updatedState[uploadIndex] = false;
      return updatedState;
    });

    const files = e.dataTransfer.files;
    handleFileChange(uploadCategory, fileType, uploadIndex, files[0]);
  };

  const userInfo = useUserInfo();
  const { database, storage, functions } = useFirebase();

  const handleFileChange = async (
    uploadCategory: string,
    fileType: string,
    uploadIndex: number,
    selectedFile: any
  ) => {
    if (!selectedFile) {
      return;
    }
    const fileName = selectedFile.name;
    const fileExtension = fileName.split(".").pop().toLowerCase();
    if (fileExtension !== fileType) {
      Swal.fire({
        title: `Please upload a .${fileType} file`,
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
      return;
    }
    const downloadLink = await uploadFile(
      database,
      storage,
      userInfo?.user?.uid || "unknown",
      selectedFile,
      uploadCategory
    );
    if (downloadLink.downloadURL !== "") {
      setSubmissionFiles((prevState) => {
        const updatedState = [...prevState];
        updatedState[uploadIndex].downloadURL = downloadLink.downloadURL;
        updatedState[uploadIndex].fileIdKey = downloadLink.fileIdKey;
        return updatedState;
      });

      setAlgoRef(downloadLink.fileRef);
      Swal.fire({
        title: `${fileName} uploaded!`,
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
                  Submission Name
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
                  Explain how your submission works.
                </p>
              </div>

              <div className="col-span-full">
                <label
                  htmlFor="algo-upload"
                  className="block text-sm font-medium leading-6 text-white"
                >
                  Submission Upload
                </label>

                <div
                  className={
                    submissionFiles[0].downloadURL
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                      : isDragOver[0]
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-indigo-500 px-6 py-10"
                      : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"
                  }
                  ref={refs[0]}
                  onDragOver={(e) => handleDragOver(e, 0)}
                  onDragLeave={() => handleDragLeave(0)}
                  onDrop={(e) => handleDrop(e, "requirements_txt", "txt", 0)}
                >
                  <div className="text-center">
                    {!submissionFiles[0].downloadURL && (
                      <PaperClipIcon
                        className="mx-auto h-12 w-12 text-gray-500"
                        aria-hidden="true"
                      />
                    )}
                    {submissionFiles[0].downloadURL && (
                      <CheckIcon
                        className="mx-auto h-12 w-12 text-green-500"
                        aria-hidden="true"
                      />
                    )}
                    <div className="mt-4 flex text-sm leading-6 text-gray-400">
                      <label
                        htmlFor="file-upload-requirements"
                        className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500"
                      >
                        <span>Upload requirements.txt</span>
                        <input
                          id="file-upload-requirements"
                          name="file-upload-requirements"
                          onChange={(e) => {
                            handleFileChange(
                              "requirements_txt",
                              "txt",
                              0,
                              //@ts-ignore
                              e.target.files![0]
                            );
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

                <div
                  className={
                    submissionFiles[1].downloadURL
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                      : isDragOver[1]
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-indigo-500 px-6 py-10"
                      : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"
                  }
                  ref={refs[1]}
                  onDragOver={(e) => handleDragOver(e, 1)}
                  onDragLeave={() => handleDragLeave(1)}
                  onDrop={(e) => handleDrop(e, "test_py", "py", 1)}
                >
                  <div className="text-center">
                    {!submissionFiles[1].downloadURL && (
                      <PaperClipIcon
                        className="mx-auto h-12 w-12 text-gray-500"
                        aria-hidden="true"
                      />
                    )}
                    {submissionFiles[1].downloadURL && (
                      <CheckIcon
                        className="mx-auto h-12 w-12 text-green-500"
                        aria-hidden="true"
                      />
                    )}
                    <div className="mt-4 flex text-sm leading-6 text-gray-400">
                      <label
                        htmlFor="file-upload-test"
                        className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500"
                      >
                        <span>Upload test.py</span>
                        <input
                          id="file-upload-test"
                          name="file-upload-test"
                          onChange={(e) => {
                            handleFileChange(
                              "test_py",
                              "py",
                              1,
                              //@ts-ignore
                              e.target.files[0]
                            );
                          }}
                          type="file"
                          className="sr-only"
                        />
                      </label>
                      <p className="pl-1">or drag and drop</p>
                    </div>
                    <p className="text-xs leading-5 text-gray-400">
                      .py up to 100KB
                    </p>
                  </div>
                </div>

                <div
                  className={
                    submissionFiles[2].downloadURL
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                      : isDragOver[2]
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-indigo-500 px-6 py-10"
                      : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"
                  }
                  ref={refs[2]}
                  onDragOver={(e) => handleDragOver(e, 2)}
                  onDragLeave={() => handleDragLeave(2)}
                  onDrop={(e) => handleDrop(e, "trained_ppo_zip", "zip", 2)}
                >
                  <div className="text-center">
                    {!submissionFiles[2].downloadURL && (
                      <PaperClipIcon
                        className="mx-auto h-12 w-12 text-gray-500"
                        aria-hidden="true"
                      />
                    )}
                    {submissionFiles[2].downloadURL && (
                      <CheckIcon
                        className="mx-auto h-12 w-12 text-green-500"
                        aria-hidden="true"
                      />
                    )}
                    <div className="mt-4 flex text-sm leading-6 text-gray-400">
                      <label
                        htmlFor="file-upload-trained_model"
                        className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500"
                      >
                        <span>Upload trained model</span>
                        <input
                          id="file-upload-trained_model"
                          name="file-upload-trained_model"
                          onChange={(e) => {
                            handleFileChange(
                              "trained_ppo_zip",
                              "zip",
                              2,
                              //@ts-ignore
                              e.target.files[0]
                            );
                          }}
                          type="file"
                          className="sr-only"
                        />
                      </label>
                      <p className="pl-1">or drag and drop</p>
                    </div>
                    <p className="text-xs leading-5 text-gray-400">
                      .zip up to 100KB
                    </p>
                  </div>
                </div>


                <div
                  className={
                    submissionFiles[3].downloadURL
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-green-400 px-6 py-10"
                      : isDragOver[3]
                      ? "mt-2 flex justify-center rounded-lg border border-solid border-indigo-500 px-6 py-10"
                      : "mt-2 flex justify-center rounded-lg border border-dashed border-white/25 px-6 py-10"
                  }
                  ref={refs[3]}
                  onDragOver={(e) => handleDragOver(e, 3)}
                  onDragLeave={() => handleDragLeave(3)}
                  onDrop={(e) => handleDrop(e, "readme.md", "md", 3)}
                >
                  <div className="text-center">
                    {!submissionFiles[3].downloadURL && (
                      <PaperClipIcon
                        className="mx-auto h-12 w-12 text-gray-500"
                        aria-hidden="true"
                      />
                    )}
                    {submissionFiles[3].downloadURL && (
                      <CheckIcon
                        className="mx-auto h-12 w-12 text-green-500"
                        aria-hidden="true"
                      />
                    )}
                    <div className="mt-4 flex text-sm leading-6 text-gray-400">
                      <label
                        htmlFor="file-upload-trained_model"
                        className="relative cursor-pointer rounded-md bg-gray-900 font-semibold text-white focus-within:outline-none focus-within:ring-2 focus-within:ring-indigo-600 focus-within:ring-offset-2 focus-within:ring-offset-gray-900 hover:text-indigo-500"
                      >
                        <span>Upload readme.md</span>
                        <input
                          id="file-upload-trained_model"
                          name="file-upload-trained_model"
                          onChange={(e) => {
                            handleFileChange(
                              "readme.md",
                              "md",
                              3,
                              //@ts-ignore
                              e.target.files[0]
                            );
                          }}
                          type="file"
                          className="sr-only"
                        />
                      </label>
                      <p className="pl-1">or drag and drop</p>
                    </div>
                    <p className="text-xs leading-5 text-gray-400">
                      .md up to 100KB
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
                  submissionFiles,
                  algoRef,
                  database,
                  userInfo?.user?.uid || ""
                ) //bad practice, fix later
              ) {
                const res = axios.get(
                  `https://nutc-linter-4oeeau4rxa-uc.a.run.app/?uid=${userInfo?.user?.uid}&algo_id=${algoRef.key}`
                );
                Swal.fire({
                  title: "Submission submitted!",
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
