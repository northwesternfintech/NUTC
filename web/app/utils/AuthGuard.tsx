"use client";
import { useUser } from '@auth0/nextjs-auth0/client'
import { useRouter } from 'next/navigation';
import { useEffect } from 'react';

// Auth Guard for redirecting to registration page if user is not registered but logged in through Auth0
export default function AuthGuardRedirectToRegistration() {
  const { user, error, isLoading } = useUser();
  console.log(user, error, isLoading);

  // get user by uid to /api/user
  const router = useRouter();

  useEffect(() => {
    if (user) {
      const req = fetch(`/api/user/${user.sid}`, {
        method: 'GET',
      })

      req.then(res => {
        if (res.status !== 200) {
          res.json().then(data => {
            console.error(data)
          }
          )
          router.push('/registration')
        }
      })

    }
  }, [user, router])
   
  return (<></>)
}
