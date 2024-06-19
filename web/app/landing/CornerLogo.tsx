import Image from "next/image"

export default function CornerLogo() {
  return <header className='block absolute inset-x-0 top-0 z-50'>
    <nav
      className='flex items-center justify-between p-6 px-8'
      aria-label='Global'
    >
      <a href='/' className='-m-1.5 p-1.5'>
        <span className='sr-only'>NUFT</span>
        <Image
          src='/images/logo.png'
          width={50}
          height={50}
          className='h-8 w-auto'
          alt='Logo'
        />
      </a>
    </nav>
  </header>
}
