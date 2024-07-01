"use server";

import { getSession } from "@auth0/nextjs-auth0";
import prisma from "@/prisma/prismaClient";

export async function getAlgos() {
  const session = await getSession();
  if (!session?.user.sub) {
    return null;
  }

  const uid: string = session.user.sub;
  if (!uid) {
    return null;
  }

  return await prisma.algo.findMany({
    where: {
      uid: uid,
    },
    include: {
      algoFile: true,
    },
  });
}

export async function getAlgo(algoFileKey: string) {
  const session = await getSession();
  if (!session?.user.sub) {
    return null;
  }

  const uid: string = session.user.sub;
  if (!uid) {
    return null;
  }

  const algo = await prisma.algo.findUnique({
    where: {
      user: {
        uid: uid,
      },
      algoFileKey: algoFileKey,
    },
    include: {
      algoFile: true,
    },
  });

  return algo;
}
