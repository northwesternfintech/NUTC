"use server";

import { getSession } from "@auth0/nextjs-auth0";
import prisma from "@/prisma/prismaClient";
import { redirect } from "next/navigation";
import { ParticipantState } from "@prisma/client";

export async function getUserRegistrationState() {
  const session = await getSession();
  if (!session?.user.sub) {
    return null;
  }

  const uid: string = session.user.sub;
  if (!uid) {
    return null;
  }

  const res = await prisma.user.findUnique({
    where: {
      uid: uid,
    },
    select: {
      participantState: true,
    },
  });

  return res?.participantState;
}

export async function redirectIfStateNot(state: ParticipantState) {
  const userRegistrationState = await getUserRegistrationState();
  if (!userRegistrationState) {
    redirect("/api/auth/login");
  }
  if (userRegistrationState === state) return;

  if (userRegistrationState === ParticipantState.WAITING) {
    redirect("/waiting");
  } else if (userRegistrationState === ParticipantState.PRE_REGISTRATION) {
    redirect("/register");
  } else if (userRegistrationState === ParticipantState.REJECTED) {
    redirect("/rejected");
  } else {
    redirect("/dash");
  }
}

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

export async function getAlgo(algoFileS3Key: string) {
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
      algoFileS3Key,
    },
    include: {
      algoFile: true,
    },
  });

  return algo;
}
