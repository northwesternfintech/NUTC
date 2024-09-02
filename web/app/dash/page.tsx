"use client";
import { Fragment, useState } from "react";
import { Menu, MenuButton, MenuItem, Transition } from "@headlessui/react";
import { getAlgos } from "@/api";
import { useQuery } from "@tanstack/react-query";
import {
  ChevronRightIcon,
  ChevronUpDownIcon,
  MagnifyingGlassIcon,
} from "@heroicons/react/20/solid";
import { NoSubmissions } from "./no-submission";

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

function classNames(...classes: any) {
  return classes.filter(Boolean).join(" ");
}

export default function Dashboard() {
  const { data: algos } = useQuery({
    queryFn: async () => await getAlgos(),
    queryKey: ["algos"],
  });
  console.log(algos);

  const [searchTerm, setSearchTerm] = useState("");

  const handleSearchChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setSearchTerm(event.target.value);
  };

  return (
    <div>
      <div className="sticky top-0 z-40 flex h-16 shrink-0 items-center gap-x-6 border-b border-white/5 bg-gray-900 px-4 shadow-sm sm:px-6 lg:px-8">
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
                value={searchTerm}
                onChange={handleSearchChange}
              />
            </div>
          </form>
        </div>
      </div>
      <main className="lg:pr-0">
        <header className="flex items-center justify-between border-b border-white/5 px-4 py-4 sm:px-6 sm:py-6 lg:px-8">
          <h1 className="text-base font-semibold leading-7 text-white">
            Submissions
          </h1>

          {/* Sort dropdown */}
          <Menu as="div" className="relative">
            <MenuButton className="flex items-center gap-x-1 text-sm font-medium leading-6 text-white">
              Sort by
              <ChevronUpDownIcon
                className="h-5 w-5 text-gray-500"
                aria-hidden="true"
              />
            </MenuButton>
            <Transition
              as={Fragment}
              enter="transition ease-out duration-100"
              enterFrom="transform opacity-0 scale-95"
              enterTo="transform opacity-100 scale-100"
              leave="transition ease-in duration-75"
              leaveFrom="transform opacity-100 scale-100"
              leaveTo="transform opacity-0 scale-95">
              <div className="absolute right-0 z-10 mt-2.5 w-40 origin-top-right rounded-md bg-white py-2 shadow-lg ring-1 ring-gray-900/5 focus:outline-none">
                <MenuItem>
                  {({ focus }) => (
                    <a
                      href="#"
                      className={classNames(
                        focus ? "bg-gray-50" : "",
                        "block px-3 py-1 text-sm leading-6 text-gray-900",
                      )}>
                      Name
                    </a>
                  )}
                </MenuItem>
                <MenuItem>
                  {({ focus }) => (
                    <a
                      href="#"
                      className={classNames(
                        focus ? "bg-gray-50" : "",
                        "block px-3 py-1 text-sm leading-6 text-gray-900",
                      )}>
                      Date updated
                    </a>
                  )}
                </MenuItem>
                <MenuItem>
                  {({ focus }) => (
                    <a
                      href="#"
                      className={classNames(
                        focus ? "bg-gray-50" : "",
                        "block px-3 py-1 text-sm leading-6 text-gray-900",
                      )}>
                      Status
                    </a>
                  )}
                </MenuItem>
              </div>
            </Transition>
          </Menu>
        </header>

        {/* Deployment list */}
        {algos ? (
          <ul role="list" className="divide-y divide-white/5">
            {algos.length === 0 && <NoSubmissions />}
            {algos
              .filter(
                algo =>
                  searchTerm === "" ||
                  (algo.name &&
                    algo.name.toLowerCase().includes(searchTerm.toLowerCase())),
              )
              .map(deployment => (
                <li
                  key={deployment.algoFileS3Key}
                  className="relative flex items-center space-x-4 px-4 py-4 sm:px-6 lg:px-8">
                  <div className="min-w-0 flex-auto">
                    <div className="flex items-center gap-x-3">
                      <div
                        className={classNames(
                          statuses[deployment.lintResults],
                          "flex-none rounded-full p-1",
                        )}>
                        <div className="h-2 w-2 rounded-full bg-current" />
                      </div>
                      <h2 className="min-w-0 text-sm font-semibold leading-6 text-white">
                        <a
                          href={`/dash/submissions/${deployment.algoFileS3Key}`}
                          className="flex gap-x-2">
                          {/* <span className="hidden truncate">
                            {deployment.teamName}
                          </span> */}
                          <span className="hidden text-gray-400">/</span>
                          <span className="whitespace-nowrap">
                            {deployment.name}
                          </span>
                          <span className="absolute inset-0" />
                        </a>
                      </h2>
                    </div>
                    <div className="mt-3 flex items-center gap-x-2.5 text-xs leading-5 text-gray-400">
                      <p className="truncate">{deployment.description}</p>
                      <svg
                        viewBox="0 0 2 2"
                        className="h-0.5 w-0.5 flex-none fill-gray-300">
                        <circle cx={1} cy={1} r={1} />
                      </svg>
                      <p className="whitespace-nowrap">
                        {deployment.lintResults}
                      </p>
                    </div>
                  </div>
                  <div
                    className={classNames(
                      environments[deployment.lintResults],
                      "rounded-full flex-none py-1 px-2 text-xs font-medium ring-1 ring-inset",
                    )}>
                    {deployment.lintResults}
                  </div>
                  <ChevronRightIcon
                    className="h-5 w-5 flex-none text-gray-400"
                    aria-hidden="true"
                  />
                </li>
              ))}
          </ul>
        ) : null}
      </main>
    </div>
  );
}
