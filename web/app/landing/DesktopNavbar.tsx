import Image from 'next/image'
import { navigation } from './navigation'

export default function DesktopNavbar() {
  return (
    <header className='hidden lg:block absolute inset-x-0 top-0 z-50'>
      <nav
        className='flex items-center justify-between p-6 px-8'
        aria-label='Global'
      >
        <div className='flex flex-1'>
          <a href='#' className='-m-1.5 p-1.5'>
            <span className='sr-only'>NUFT</span>
            <Image
              src='/images/logo.png'
              width={50}
              height={50}
              className='h-8 w-auto'
              alt='Logo'
            />
          </a>
        </div>
        <div className='flex gap-x-16'>
          {navigation.map((item) => (
            <a
              key={item.name}
              href={item.href}
              className='text-md font-semibold leading-6 text-white'
            >
              {item.name}
            </a>
          ))}
        </div>
        <div className='flex flex-1 justify-end'>
          <a href='#' className='text-md font-semibold leading-6 text-white'>
            Dashboard <span aria-hidden='true'>&rarr;</span>
          </a>
        </div>
      </nav>
    </header>
  )
}
