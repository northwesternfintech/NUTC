import Link from "next/link";
import {ArrowUpOnSquareIcon, PlusIcon} from "@heroicons/react/20/solid";

export default function Example() {
  return (
      <>
        <div className="text-center py-20 sm:py-24">
          <ArrowUpOnSquareIcon className="mx-auto h-12 w-12 text-gray-400" />
          <h3 className="mt-2 text-sm font-semibold text-gray-900">
            No submissions
          </h3>
          <p className="mt-1 text-sm text-gray-500">
            Get started by uploading your first algorithm.
          </p>
          <div className="mt-6">
            <Link
                className="inline-flex items-center rounded-md bg-indigo-600 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
                href="/dash/submit"
            >
              <PlusIcon className="-ml-0.5 mr-1.5 h-5 w-5" aria-hidden="true" />
              New Algorithm
            </Link>
          </div>
        </div>
      </>
  )
}
