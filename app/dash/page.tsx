"use client";
import { Fragment, useEffect, useState } from "react";
import NoSubmissions from "./no-submissions";
import { Menu, Transition } from "@headlessui/react";
const statuses: any = {
  pending: "text-yellow-500 bg-yellow-100/10",
  success: "text-green-400 bg-green-400/10",
  failure: "text-rose-400 bg-rose-400/10",
};
const environments: any = {
  Results: "text-indigo-400 bg-indigo-400/10 ring-indigo-400/20",
  Pending: "text-gray-400 bg-gray-400/10 ring-gray-400/20",
  "Lint Output": "text-rose-400 bg-rose-400/10 ring-rose-400/30",
};
const deployments: any = [
  {
    id: 1,
    href: "#",
    projectName: "new-experimental-algo",
    teamName: "Shxiv",
    status: "pending",
    statusText: "Linting Succeeded 13m 16s ago",
    description: "Waiting for Simulation",
    environment: "Pending",
  },
  {
    id: 2,
    href: "#",
    projectName: "RL-buggy-test",
    teamName: "Shxiv",
    status: "error",
    statusText: "Linting failed 25m 52s ago",
    description: "Linting Failed",
    environment: "Lint Output",
  },
  {
    id: 3,
    href: "#",
    projectName: "DL-test-4",
    teamName: "Shxiv",
    status: "finished",
    statusText: "Simulation ran 4h 32m ago",
    description: "Finished Running",
    environment: "Results",
  },
];

const activityItems = [
  {
    title: "Linting Succeeded",
    projectName: "new-experimental-algo",
    date: "13m",
    dateTime: "2023-01-23T11:00",
  },
  {
    title: "Linting Started",
    projectName: "new-experimental-algo",
    date: "15m",
    dateTime: "2023-01-23T11:00",
  },
  {
    title: "Linting Failed",
    projectName: "RL-buggy-test",
    date: "25m",
    dateTime: "2023-01-23T11:00",
  },
  {
    title: "Simulation Run",
    projectName: "DL-test-4",
    date: "4h",
    dateTime: "2023-01-23T11:00",
  },
  {
    title: "Linting Succeeded",
    projectName: "DL-test-4",
    date: "5h",
    dateTime: "2023-01-23T11:00",
  },
  {
    title: "Linting Started",
    projectName: "DL-test-4",
    date: "5h",
    dateTime: "2023-01-23T11:00",
  },
];

import {
  ChevronRightIcon,
  ChevronUpDownIcon,
  MagnifyingGlassIcon,
} from "@heroicons/react/20/solid";
import { useUserInfo } from "../login/auth/context";
import AlgorithmType from "./algoType";

function classNames(...classes: any) {
  return classes.filter(Boolean).join(" ");
}
export default function Dashboard() {
  const [algos, setAlgos] = useState<AlgorithmType[]>([]);
  const { user } = useUserInfo();

  useEffect(() => {
    const algos = user?.algos;
    if (!algos) {
      return;
    }
    var tmpAlgos: any = [];
    //@ts-ignore
    for (const [key, value] of algos) {
      const test: AlgorithmType = value;
      tmpAlgos.push({
        id: key,
        href: `/dash/submissions/${key}`,
        projectName: value.name,
        teamName: value.name,
        status: test.lintResults,
        statusText: test.lintResults,
        description: test.description,
        environment: test.lintResults,
      });
    }
    setAlgos(tmpAlgos);
  }, [user]);

  return (
    <div>
      <div className="sticky top-0 z-40 flex h-16 shrink-0 items-center gap-x-6 border-b border-white/5 bg-[#212936] px-4 shadow-sm sm:px-6 lg:px-8">
        <div className="flex flex-1 gap-x-4 self-stretch lg:gap-x-6">
          <form className="flex flex-1" action="#" method="GET">
            <label htmlFor="search-field" className="sr-only">
              Search
            </label>
            <div className="relative w-full">
              <MagnifyingGlassIcon
                className="pointer-events-none absolute inset-y-0 left-0 h-full w-5 text-gray-500"
                aria-hidden="true"
              />
              <input
                id="search-field"
                className="block h-full w-full border-0 bg-transparent py-0 pl-8 pr-0 text-white focus:ring-0 sm:text-sm"
                placeholder="Search..."
                type="search"
                name="search"
              />
            </div>
          </form>
        </div>
      </div>
      <main className="lg:pr-96">
        <header className="flex items-center justify-between border-b border-white/5 px-4 py-4 sm:px-6 sm:py-6 lg:px-8">
          <h1 className="text-base font-semibold leading-7 text-white">
            Submissions
          </h1>

          {/* Sort dropdown */}
          <Menu as="div" className="relative">
            <Menu.Button className="flex items-center gap-x-1 text-sm font-medium leading-6 text-white">
              Sort by
              <ChevronUpDownIcon
                className="h-5 w-5 text-gray-500"
                aria-hidden="true"
              />
            </Menu.Button>
            <Transition
              as={Fragment}
              enter="transition ease-out duration-100"
              enterFrom="transform opacity-0 scale-95"
              enterTo="transform opacity-100 scale-100"
              leave="transition ease-in duration-75"
              leaveFrom="transform opacity-100 scale-100"
              leaveTo="transform opacity-0 scale-95"
            >
              <Menu.Items className="absolute right-0 z-10 mt-2.5 w-40 origin-top-right rounded-md bg-white py-2 shadow-lg ring-1 ring-gray-900/5 focus:outline-none">
                <Menu.Item>
                  {({ active }) => (
                    <a
                      href="#"
                      className={classNames(
                        active ? "bg-gray-50" : "",
                        "block px-3 py-1 text-sm leading-6 text-gray-900",
                      )}
                    >
                      Name
                    </a>
                  )}
                </Menu.Item>
                <Menu.Item>
                  {({ active }) => (
                    <a
                      href="#"
                      className={classNames(
                        active ? "bg-gray-50" : "",
                        "block px-3 py-1 text-sm leading-6 text-gray-900",
                      )}
                    >
                      Date updated
                    </a>
                  )}
                </Menu.Item>
                <Menu.Item>
                  {({ active }) => (
                    <a
                      href="#"
                      className={classNames(
                        active ? "bg-gray-50" : "",
                        "block px-3 py-1 text-sm leading-6 text-gray-900",
                      )}
                    >
                      Status
                    </a>
                  )}
                </Menu.Item>
              </Menu.Items>
            </Transition>
          </Menu>
        </header>

        {/* Deployment list */}
        <ul role="list" className="divide-y divide-white/5">
          {algos.length === 0 && <NoSubmissions />}
          {algos.map((deployment: any) => (
            <li
              key={deployment.id}
              className="relative flex items-center space-x-4 px-4 py-4 sm:px-6 lg:px-8"
            >
              <div className="min-w-0 flex-auto">
                <div className="flex items-center gap-x-3">
                  <div
                    className={classNames(
                      statuses[deployment.status],
                      "flex-none rounded-full p-1",
                    )}
                  >
                    <div className="h-2 w-2 rounded-full bg-current" />
                  </div>
                  <h2 className="min-w-0 text-sm font-semibold leading-6 text-white">
                    <a href={deployment.href} className="flex gap-x-2">
                      <span className="hidden truncate">{deployment.teamName}</span>
                      <span className="hidden text-gray-400">/</span>
                      <span className="whitespace-nowrap">
                        {deployment.projectName}
                      </span>
                      <span className="absolute inset-0" />
                    </a>
                  </h2>
                </div>
                <div className="mt-3 flex items-center gap-x-2.5 text-xs leading-5 text-gray-400">
                  <p className="truncate">{deployment.description}</p>
                  <svg
                    viewBox="0 0 2 2"
                    className="h-0.5 w-0.5 flex-none fill-gray-300"
                  >
                    <circle cx={1} cy={1} r={1} />
                  </svg>
                  <p className="whitespace-nowrap">{deployment.statusText}</p>
                </div>
              </div>
              <div
                className={classNames(
                  environments[deployment.environment],
                  "rounded-full flex-none py-1 px-2 text-xs font-medium ring-1 ring-inset",
                )}
              >
                {deployment.environment}
              </div>
              <ChevronRightIcon
                className="h-5 w-5 flex-none text-gray-400"
                aria-hidden="true"
              />
            </li>
          ))}
        </ul>
      </main>
      <aside className="bg-[#1B2230] lg:fixed lg:bottom-0 lg:right-0 lg:top-16 lg:w-96 lg:overflow-y-auto lg:border-l lg:border-white/5">
        <header className="flex items-center justify-between border-b border-white/5 px-4 py-4 sm:px-6 sm:py-6 lg:px-8">
          <h2 className="text-base font-semibold leading-7 text-white">
            Activity feed
          </h2>
          <a
            href="#"
            className="text-sm font-semibold leading-6 text-indigo-400"
          >
            View all
          </a>
        </header>
        <ul role="list" className="divide-y divide-white/5">
          {activityItems.map((item) => (
            <li key={item.title} className="px-4 py-4 sm:px-6 lg:px-8">
              <div className="flex items-center gap-x-3">
                <h3 className="flex-auto truncate text-sm font-semibold leading-6 text-white">
                  {item.title}
                </h3>
                <time
                  dateTime={item.dateTime}
                  className="flex-none text-xs text-gray-600"
                >
                  {item.date}
                </time>
              </div>
              <p className="mt-3 truncate text-sm text-gray-500">
                Output from{" "}
                <span className="text-gray-400">{item.projectName}</span>
              </p>
            </li>
          ))}
        </ul>
      </aside>
    </div>
  );
}
