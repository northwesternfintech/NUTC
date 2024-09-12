import prisma from "@/prisma/prismaClient";
import { getSession } from "@auth0/nextjs-auth0";
import { ParticipantState, Profile } from "@prisma/client";
import { NextResponse } from "next/server";
import createDefaultUser from "../defaultUser";
import { SendEmail } from "@/app/api/sendEmail";
import { GenerateApplicationEmail } from "./generateApplicationEmail";

export async function POST(req: Request) {
  try {
    const profile: Profile = await req.json();
    console.log(JSON.stringify(profile));
    for (const [_, value] of Object.entries(profile)) {
      if (!value) {
        return NextResponse.json(
          { message: "Not all fields in profile added" },
          { status: 402 },
        );
      }
    }

    const session = await getSession();
    if (!session?.user.sub || session.user.sub != profile.uid) {
      return NextResponse.json({ message: "Unauthorized" }, { status: 402 });
    }
    const user = await createDefaultUser(session);
    await prisma.user.update({
      where: {
        uid: user.uid,
      },
      data: {
        participantState: ParticipantState.WAITING,
      },
    });

    await prisma.profile.create({
      data: profile,
    });

    SendEmail(
      "info@nutc.io",
      session?.user.email,
      "Application Submitted",
      "Your NUTC application was submitted. You will hear back in the next 72 hours.",
    );
    GenerateApplicationEmail(user, profile);

    return NextResponse.json({ status: 200 });
  } catch (error) {
    console.log(error);
    return NextResponse.json({ message: error }, { status: 500 });
  }
}
