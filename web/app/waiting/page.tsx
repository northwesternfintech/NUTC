import Image from "next/image";
export default function Waiting() {
  return (
    <>
      <main className="relative isolate min-h-screen">
        <Image
          src="/images/background.jpeg"
          alt="Background"
          width={1500}
          height={1125}
          className="absolute inset-0 -z-10 h-full w-full object-cover object-top"
        />
        <div className="mx-auto max-w-7xl px-6 py-32 text-center sm:py-40 lg:px-8">
          <h1 className="mt-4 text-3xl font-bold tracking-tight text-white sm:text-5xl">Waiting for Results</h1>
          <p className="mt-4 text-base text-white/70 sm:mt-6">Your application will be reviewed in the following days. We&apos;ll reach out with information soon.</p>
          <div className="mt-10 flex justify-center">
            <a href="/" className="text-sm font-semibold leading-7 text-white">
              <span aria-hidden="true">&larr;</span> Back to home
            </a>
          </div>
        </div>
      </main>
    </>
  )
}

