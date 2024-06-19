import prisma from '@/prisma/prismaClient'
import { Claims } from '@auth0/nextjs-auth0'

export default async function createDefaultUser(session: Claims) {
  const user = await prisma.user.findUnique({
    where: { uid: session.user.sub },
  })
  if (!user) {
    await prisma.user.create({
      data: {
        uid: session.user.sub,
        email: session.user.email,
      },
    })
  }
}
