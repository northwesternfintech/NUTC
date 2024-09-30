import { getSession } from "@auth0/nextjs-auth0";
import { NextResponse } from "next/server";
import prisma from "@/prisma/prismaClient";

export async function GET() {
  try {
    const session = await getSession();

    if (!session?.user?.sub) {
      return NextResponse.json({ message: "Not logged in" }, { status: 401 });
    }

    const user = (await prisma.user.findUnique({
      where: { uid: session.user.sub },
      include: { team: { include: { members: { include: { profile: true } } } } },
    }));
    const teamName = user?.teamName;
    const teamMembers = user?.team?.members;

    var teamMemberNames: String[] = [];
    for (const teamMember of teamMembers || []) {
      if (teamMember.uid == user?.uid) continue;
      teamMemberNames.push(`${teamMember.profile?.firstName} ${teamMember.profile?.lastName}`);
    }

    return NextResponse.json({ teamName, teamMemberNames, status: 200 });
  } catch (error: any) {
    console.error(error);
    return NextResponse.json({ message: error.message }, { status: 500 });
  }
}
