import Image from "next/image";
import Logo from "@/app/assets/logo.png";
import GoogleLogin from "./auth/google/page";
import GithubLogin from "./auth/github/page";
export default function Login() {
  return (
    <>
      <div className="flex min-h-full flex-1 flex-col items-center justify-center px-6 py-12 lg:px-8">
        <div className="sm:mx-auto sm:w-full sm:max-w-sm">
          <Image
            className="mx-auto h-10 w-auto"
            src={Logo}
            alt="Your Company"
          />
          <h2 className="mt-10 text-center text-2xl font-bold leading-9 tracking-tight text-white">
            Sign in or create an account
          </h2>
        </div>

        <div className="mt-10 max-w-xs w-full">
          <div className="relative">
            <div
              className="absolute inset-0 flex items-center"
              aria-hidden="true"
            >
              <div className="w-full border-t border-gray-200" />
            </div>
            <div className="relative flex justify-center text-sm font-medium leading-6">
              <span className="bg-gray-900 px-6 text-white">
                Use your institution email
              </span>
            </div>
          </div>

          <div className="mt-6 grid grid-cols-2 gap-4">
            <GoogleLogin />
            <GithubLogin />
          </div>
        </div>

        <p className="mt-10 text-center text-sm text-gray-400">
          Have questions?{" "}
          <a
            href="#"
            className="font-semibold leading-6 text-indigo-400 hover:text-indigo-300"
          >
            Reach out to our tech team
          </a>
        </p>
      </div>
    </>
  );
}
