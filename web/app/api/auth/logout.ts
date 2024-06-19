import { handleLogout } from '@auth0/nextjs-auth0';
import { NextRequest } from 'next/server';

export default async function logout(req:NextRequest, res:any) {
  try {
    await handleLogout(req, res, {
      returnTo: process.env.AUTH0_BASE_URL+"/"
    });
  } catch (error:any) {
    res.status(error.status || 500).end(error.message);
  }
}

