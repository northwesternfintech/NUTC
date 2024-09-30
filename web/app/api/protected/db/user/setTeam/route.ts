import { getSession } from "@auth0/nextjs-auth0";
import { NextResponse } from "next/server";
import prisma from "@/prisma/prismaClient";

export async function POST(req: Request) {
  try {
    const session = await getSession();

    if (!session?.user?.sub) {
      return NextResponse.json({ message: "Not logged in" }, { status: 401 });
    }

    const { newTeam } = await req.json();

    console.log(`Updated team to be ${newTeam}`);
    await prisma.team.upsert({ where: { name: newTeam }, update: {}, create: { name: newTeam } });

    await prisma.user.update({
      where: { uid: session.user.sub },
      data: { teamName: newTeam }
    });

    return NextResponse.json({ message: "success", status: 200 });
  } catch (error: any) {
    console.error(error);
    return NextResponse.json({ message: error.message }, { status: 500 });
  }
}
