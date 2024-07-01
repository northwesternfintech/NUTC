"use client";
import React, { Fragment, useState } from "react";
import Link from "next/link";
import {
  Dialog,
  DialogPanel,
  Transition,
  TransitionChild,
} from "@headlessui/react";
import Logo from "../favicon.ico";
import Image from "next/image";
import {
  ArrowDownOnSquareIcon,
  ArrowTrendingUpIcon,
  ArrowUpTrayIcon,
  Bars3Icon,
  QuestionMarkCircleIcon,
  XMarkIcon,
} from "@heroicons/react/24/outline";
import { usePathname } from "next/navigation";
import { useQuery } from "@tanstack/react-query";
import { getAlgos } from "@/api";
import { Algo } from "@prisma/client";

const navigation = [
  {
    name: "Submissions",
    href: "/dash",
    icon: ArrowTrendingUpIcon,
    activeName: "/dash",
  },
  {
    name: "FAQ",
    href: "/dash/faq",
    icon: QuestionMarkCircleIcon,
    activeName: "/dash/faq",
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
  // {
  //   name: "Partner Settings",
  //   href: "/dash/group",
  //   icon: UserPlusIcon,
  //   activeName: "/dash/group",
  // },
];

function classNames(...classes: any) {
  return classes.filter(Boolean).join(" ");
}

export function DashNav({
  children,
  algos: initialAlgos,
}: {
  children: React.ReactNode;
  algos: Algo[];
}) {
  const [sidebarOpen, setSidebarOpen] = useState(false);
  const pathName = usePathname();

  const { data: algos } = useQuery({
    queryFn: async () => await getAlgos(),
    queryKey: ["algos"],
    initialData: initialAlgos,
  });

  const algosList = (algos ?? []).map((algo, idx) => (
    <li key={algo.name}>
      <a
        href={`/dash/submissions/${algo.algoFileKey}`}
        className={classNames(
          pathName.includes(algo.algoFileKey)
            ? "bg-gray-800 text-white"
            : "text-gray-400 hover:text-white hover:bg-gray-800",
          "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
        )}>
        <span className="flex h-6 w-6 shrink-0 items-center justify-center rounded-lg border border-gray-700 bg-gray-800 text-[0.625rem] font-medium text-gray-400 group-hover:text-white">
          {idx + 1}
        </span>
        <span className="truncate">{algo.name}</span>
      </a>
    </li>
  ));

  return (
    <>
      <div>
        <Transition show={sidebarOpen} as={Fragment}>
          <Dialog
            as="div"
            className="relative z-50 lg:hidden"
            onClose={setSidebarOpen}>
            <TransitionChild
              as={Fragment}
              enter="transition-opacity ease-linear duration-300"
              enterFrom="opacity-0"
              enterTo="opacity-100"
              leave="transition-opacity ease-linear duration-300"
              leaveFrom="opacity-100"
              leaveTo="opacity-0">
              <div className="fixed inset-0 bg-gray-900/80" />
            </TransitionChild>

            <div className="fixed inset-0 flex">
              <TransitionChild
                as={Fragment}
                enter="transition ease-in-out duration-300 transform"
                enterFrom="-translate-x-full"
                enterTo="translate-x-0"
                leave="transition ease-in-out duration-300 transform"
                leaveFrom="translate-x-0"
                leaveTo="-translate-x-full">
                <DialogPanel className="relative mr-16 flex w-full max-w-xs flex-1">
                  <TransitionChild
                    as={Fragment}
                    enter="ease-in-out duration-300"
                    enterFrom="opacity-0"
                    enterTo="opacity-100"
                    leave="ease-in-out duration-300"
                    leaveFrom="opacity-100"
                    leaveTo="opacity-0">
                    <div className="absolute left-full top-0 flex w-16 justify-center pt-5">
                      <button
                        type="button"
                        className="-m-2.5 p-2.5"
                        onClick={() => setSidebarOpen(false)}>
                        <span className="sr-only">Close sidebar</span>
                        <XMarkIcon
                          className="h-6 w-6 text-white"
                          aria-hidden="true"
                        />
                      </button>
                    </div>
                  </TransitionChild>
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
                            {navigation.map(item => (
                              <li key={item.name}>
                                <Link
                                  href={item.href}
                                  className={classNames(
                                    item.activeName === pathName
                                      ? "bg-gray-800 text-white"
                                      : "text-gray-400 hover:text-white hover:bg-gray-800",
                                    "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
                                  )}>
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
                            {algosList}
                          </ul>
                        </li>
                      </ul>
                    </nav>
                  </div>
                </DialogPanel>
              </TransitionChild>
            </div>
          </Dialog>
        </Transition>

        {/* static sidebar for desktop */}
        <div className="hidden lg:fixed lg:inset-y-0 lg:z-50 lg:flex lg:w-72 lg:flex-col">
          <div
            className="flex grow flex-col gap-y-5 overflow-y-auto px-6"
            style={{ backgroundColor: "rgb(14,19,31)" }}>
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
                    {navigation.map(item => (
                      <li key={item.name}>
                        <Link
                          href={item.href}
                          className={classNames(
                            item.activeName === pathName
                              ? "bg-gray-800 text-white"
                              : "text-gray-400 hover:text-white hover:bg-gray-800",
                            "group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold",
                          )}>
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
                    {algosList}
                  </ul>
                </li>
                <li className="-mx-6 mt-auto hidden">
                  <a
                    href="#"
                    className="flex items-center gap-x-4 px-6 py-3 text-sm font-semibold leading-6 text-white hover:bg-gray-800">
                    <Image
                      className="h-8 w-8 rounded-full bg-gray-800"
                      src="https://nu-fintech.web.app/static/media/ethanhavemann.c82791f44784294bd22b.jpeg"
                      alt="ethan"
                      width={32}
                      height={32}
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
            onClick={() => setSidebarOpen(true)}>
            <span className="sr-only">Open sidebar</span>
            <Bars3Icon className="h-6 w-6" aria-hidden="true" />
          </button>
          <div className="flex-1 text-sm font-semibold leading-6 text-white">
            Dashboard
          </div>
          <a href="#">
            <span className="sr-only">Your profile</span>
            <Image
              className="h-8 w-8 rounded-full bg-gray-800"
              src="https://nu-fintech.web.app/static/media/ethanhavemann.c82791f44784294bd22b.jpeg"
              alt="ethan"
              width={32}
              height={32}
            />
          </a>
        </div>

        <main className="bg-gray-900 lg:pl-72 min-h-screen">
          <div>{children}</div>
        </main>
      </div>
    </>
  );
}
