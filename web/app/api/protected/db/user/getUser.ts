import prisma from "@/prisma/prismaClient";

export default async function getUser(uid: string) {
  try {
    const user = await prisma.user.findUnique({
      where: { uid: uid },
      include: { profile: true },
    });

    if (!user) {
      return null;
    }

    return user;
  } catch (error: any) {
    console.error(error);
    return null;
  }
}
