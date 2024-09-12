import { ParticipantState } from "@prisma/client";
import prisma from "@/prisma/prismaClient";
import { NextRequest, NextResponse } from "next/server";

export async function GET(req: NextRequest) {
  try {
    const token = req.nextUrl.searchParams.get("token") || "";
    const accepted = req.nextUrl.searchParams.get("accept") == "true";
    const uid =
      (await prisma.emailTokens.findUnique({ where: { token } }))?.uid || "";
    console.log("Uid: " + uid + ", accepted: " + accepted);
    await prisma.user.update({
      where: {
        uid,
      },
      data: {
        participantState: accepted
          ? ParticipantState.ACCEPTED
          : ParticipantState.REJECTED,
      },
    });
    return NextResponse.json({ message: "Successful" }, { status: 200 });
  } catch (error) {
    console.log(error);
    return NextResponse.json({ message: error }, { status: 500 });
  }
}
