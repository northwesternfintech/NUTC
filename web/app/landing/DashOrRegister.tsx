'use server'
import { getSession } from '@auth0/nextjs-auth0'
import { userExistsInDb } from '@/app/register/page'

export default async function DashOrRegister() {
  const session = await getSession()

  if (session?.user && await userExistsInDb(session)) {
    return <a href='/dashboard' className='text-md font-semibold leading-6 text-white'>
      Dashboard <span aria-hidden='true'>&rarr;</span>
    </a>
  } else {
    return <a href='/register' className='text-md font-semibold leading-6 text-white'>
      Apply <span aria-hidden='true'>&rarr;</span>
    </a>
  }
}

