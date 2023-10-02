"use client";
import { Fragment, useEffect, useState } from "react";
import Link from "next/link";
import { Dialog, Transition } from "@headlessui/react";
import Logo from "@/app/assets/logo.png";
import Image from "next/image";
import { useUserInfo } from "@/app/login/auth/context";
import {
  ArrowTrendingUpIcon,
  ArrowUpTrayIcon,
  Bars3Icon,
  QuestionMarkCircleIcon,
  XMarkIcon,
  ArrowDownOnSquareIcon
} from "@heroicons/react/24/outline";
import { usePathname } from "next/navigation";

const navigation = [
  {
    name: "Submissions",
    href: "/dash",
    icon: ArrowTrendingUpIcon,
    activeName: "/dash",
  },
  {
    name: "Download Template",
    href: "/dash/template",
    icon: ArrowDownOnSquareIcon,
    activeName: "/dash/template",
  },
  {
    name: "Upload Algorithm",
    href: "/dash/submit",
    icon: ArrowUpTrayIcon,
    activeName: "/dash/submit",
  },
  {name:"FAQ",
    href:"/dash/faq",
  icon: QuestionMarkCircleIcon,
  activeName: "/dash/faq"}
  // {
    // name: "Team Profile",
    // href: "#",
    // icon: UserGroupIcon,
    // activeName: "team-profile",
  // },
];
const submissions2 = [
  {
    id: 1,
    name: "new-experimental-algo",
    href: "#",
    initial: "1",
    current: false,
  },
  { id: 2, name: "RL-buggy-test", href: "#", initial: "2", current: false },
  { id: 3, name: "DL-test-4", href: "#", initial: "2", current: false },
];

function classNames(...classes: any) {
  return classes.filter(Boolean).join(" ");
}

export default function Dash(content: React.ReactNode) {
  const [sidebarOpen, setSidebarOpen] = useState(false);
  const [currentPage, setCurrentPage] = useState("");
  const { user } = useUserInfo();
  const pathName = usePathname();

  //array of AlgorithmType
  const [submissions, setSubmissions]: any = useState([]);

  useEffect(() => {
    setCurrentPage(pathName);
  }, [pathName]);

  useEffect(() => {
    const algos = user?.algos;
    const tmpSubmissions: any = [];
    //iterate over values
    if (!algos) {
      return;
    }
    var i = 1; //not proud of this
    //@ts-ignore
    for (const [key, value] of algos) {
      tmpSubmissions.push({
        id: key,
        name: value.name,
        href: `/dash/submissions/${key}`,
        initial: String(i),
        current: false,
      });
      i++;
    }
    if (tmpSubmissions.length > 0) {
      tmpSubmissions[0].current = true;
      setSubmissions(tmpSubmissions);
    }
  }, [user]);

  return (
    <>
      <div>
        <Transition.Root show={sidebarOpen} as={Fragment}>
          <Dialog
            as="div"
            className="relative z-50 lg:hidden"
            onClose={setSidebarOpen}
          >
            <Transition.Child
              as={Fragment}
              enter="transition-opacity ease-linear duration-300"
              enterFrom="opacity-0"
              enterTo="opacity-100"
              leave="transition-opacity ease-linear duration-300"
              leaveFrom="opacity-100"
              leaveTo="opacity-0"
            >
              <div className="fixed inset-0 bg-gray-900/80" />
            </Transition.Child>

            <div className="fixed inset-0 flex">
              <Transition.Child
                as={Fragment}
                enter="transition ease-in-out duration-300 transform"
                enterFrom="-translate-x-full"
                enterTo="translate-x-0"
                leave="transition ease-in-out duration-300 transform"
                leaveFrom="translate-x-0"
                leaveTo="-translate-x-full"
              >
                <Dialog.Panel className="relative mr-16 flex w-full max-w-xs flex-1">
                  <Transition.Child
                    as={Fragment}
                    enter="ease-in-out duration-300"
                    enterFrom="opacity-0"
                    enterTo="opacity-100"
                    leave="ease-in-out duration-300"
                    leaveFrom="opacity-100"
                    leaveTo="opacity-0"
                  >
                    <div className="absolute left-full top-0 flex w-16 justify-center pt-5">
                      <button
                        type="button"
                        className="-m-2.5 p-2.5"
                        onClick={() => setSidebarOpen(false)}
                      >
                        <span className="sr-only">Close sidebar</span>
                        <XMarkIcon
                          className="h-6 w-6 text-white"
                          aria-hidden="true"
                        />
                      </button>
                    </div>
                  </Transition.Child>
                  <div className="flex grow flex-col gap-y-5 overflow-y-auto bg-gray-900 px-6 pb-2 ring-1 ring-white/10">
                    <div className="flex h-16 shrink-0 items-center">
                      <Image
                        width="32"
                        height="32"
                        className="h-8 w-auto"
                        src={Logo}
                        alt="Your Company"
                      />
                    </div>
                    <nav className="flex flex-1 flex-col">
                      <ul role="list" className="flex flex-1 flex-col gap-y-7">
                        <li>
                          <ul role="list" className="-mx-2 space-y-1">
                            {navigation.map((item) => (
                              <li key={item.name}>
                                <Link
                                  href={item.href}
                                  className={classNames(
                                    item.activeName === currentPage
                                      ? "bg-gray-800 text-white"
                                      : "text-gray-400 hover:text-white hover:bg-gray-800",
                                    "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
                                  )}
                                >
                                  <item.icon
                                    className="h-6 w-6 shrink-0"
                                    aria-hidden="true"
                                  />
                                  {item.name}
                                </Link>
                              </li>
                            ))}
                          </ul>
                        </li>
                        <li>
                          <div className="text-xs font-semibold leading-6 text-gray-400">
                            Your submissions
                          </div>
                          <ul role="list" className="-mx-2 mt-2 space-y-1">
                            {submissions.map((team: any) => (
                              <li key={team.name}>
                                <a
                                  href={team.href}
                                  className={classNames(
                                    team.current
                                      ? "bg-gray-800 text-white"
                                      : "text-gray-400 hover:text-white hover:bg-gray-800",
                                    "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
                                  )}
                                >
                                  <span className="flex h-6 w-6 shrink-0 items-center justify-center rounded-lg border border-gray-700 bg-gray-800 text-[0.625rem] font-medium text-gray-400 group-hover:text-white">
                                    {team.initial}
                                  </span>
                                  <span className="truncate">{team.name}</span>
                                </a>
                              </li>
                            ))}
                          </ul>
                        </li>
                      </ul>
                    </nav>
                  </div>
                </Dialog.Panel>
              </Transition.Child>
            </div>
          </Dialog>
        </Transition.Root>

        {/* static sidebar for desktop */}
        <div className="hidden lg:fixed lg:inset-y-0 lg:z-50 lg:flex lg:w-72 lg:flex-col">
          <div
            className="flex grow flex-col gap-y-5 overflow-y-auto px-6"
            style={{ backgroundColor: "rgb(14,19,31)" }}
          >
            <div className="flex h-16 shrink-0 items-center">
              <Image
                height="32"
                width="32"
                className="h-8 w-auto"
                src={Logo}
                alt="Your Company"
              />
            </div>
            <nav className="flex flex-1 flex-col">
              <ul role="list" className="flex flex-1 flex-col gap-y-7">
                <li>
                  <ul role="list" className="-mx-2 space-y-1">
                    {navigation.map((item) => (
                      <li key={item.name}>
                        <Link
                          href={item.href}
                          className={classNames(
                            item.activeName === currentPage
                              ? "bg-gray-800 text-white"
                              : "text-gray-400 hover:text-white hover:bg-gray-800",
                            "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
                          )}
                        >
                          <item.icon
                            className="h-6 w-6 shrink-0"
                            aria-hidden="true"
                          />
                          {item.name}
                        </Link>
                      </li>
                    ))}
                  </ul>
                </li>
                <li>
                  <div className="text-xs font-semibold leading-6 text-gray-400">
                    Your submissions
                  </div>
                  <ul role="list" className="-mx-2 mt-2 space-y-1">
                    {submissions.map((team: any) => (
                      <li key={team.name}>
                        <a
                          href={team.href}
                          className={classNames(
                            team.current
                              ? "bg-gray-800 text-white"
                              : "text-gray-400 hover:text-white hover:bg-gray-800",
                            "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
                          )}
                        >
                          <span className="flex h-6 w-6 shrink-0 items-center justify-center rounded-lg border border-gray-700 bg-gray-800 text-[0.625rem] font-medium text-gray-400 group-hover:text-white">
                            {team.initial}
                          </span>
                          <span className="truncate">{team.name}</span>
                        </a>
                      </li>
                    ))}
                  </ul>
                </li>
                <li className="-mx-6 mt-auto hidden">
                  <a
                    href="#"
                    className="flex items-center gap-x-4 px-6 py-3 text-sm font-semibold leading-6 text-white hover:bg-gray-800"
                  >
                    <img
                      className="h-8 w-8 rounded-full bg-gray-800"
                      src="https://nu-fintech.web.app/static/media/ethanhavemann.c82791f44784294bd22b.jpeg"
                      alt=""
                    />
                    <span className="sr-only">Your profile</span>
                    <span aria-hidden="true">Steve Ewald</span>
                  </a>
                </li>
              </ul>
            </nav>
          </div>
        </div>

        <div className="sticky top-0 z-40 flex items-center gap-x-6 bg-gray-900 px-4 py-4 shadow-sm sm:px-6 lg:hidden">
          <button
            type="button"
            className="-m-2.5 p-2.5 text-gray-400 lg:hidden"
            onClick={() => setSidebarOpen(true)}
          >
            <span className="sr-only">Open sidebar</span>
            <Bars3Icon className="h-6 w-6" aria-hidden="true" />
          </button>
          <div className="flex-1 text-sm font-semibold leading-6 text-white">
            Dashboard
          </div>
          <a href="#">
            <span className="sr-only">Your profile</span>
            <img
              className="h-8 w-8 rounded-full bg-gray-800"
              src="https://nu-fintech.web.app/static/media/ethanhavemann.c82791f44784294bd22b.jpeg"
              alt=""
            />
          </a>
        </div>

        <main className="bg-gray-900 lg:pl-72">
          <div>{content}</div>
        </main>
      </div>
    </>
  );
}
