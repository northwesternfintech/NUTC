import prisma from "@/prisma/prismaClient";
import { getSession } from "@auth0/nextjs-auth0";
import { Profile } from "@prisma/client";
import { NextResponse } from "next/server";
import createDefaultUser from "../defaultUser";

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
    await createDefaultUser(session);

    await prisma.profile.create({
      data: profile,
    });

    return NextResponse.json({ status: 200 });
  } catch (error) {
    console.log(error);
    return NextResponse.json({ message: error }, { status: 500 });
  }
}
