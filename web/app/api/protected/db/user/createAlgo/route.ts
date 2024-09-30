import prisma from "@/prisma/prismaClient";
import { getSession } from "@auth0/nextjs-auth0";
import { Algo } from "@prisma/client";
import { NextResponse } from "next/server";

export async function POST(req: Request) {
  try {
    const algo = (await req.json()) satisfies Partial<Algo>;
    if (
      !algo.name ||
      !algo.description ||
      !algo.case ||
      !algo.language ||
      !algo.algoFileS3Key ||
      !algo.uid
    ) {
      return new Response("Not all fields in algo added", { status: 402 });
    }

    const session = await getSession();
    if (!session?.user.sub || session.user.sub != algo.uid) {
      return NextResponse.json({ message: "Unauthorized" }, { status: 402 });
    }

    await prisma.algo.create({
      data: {
        name: algo.name,
        description: algo.description,
        case: algo.case,
        language: algo.language,
        lintResults: "pending",
        algoFile: {
          connect: {
            s3Key: algo.algoFileS3Key,
          },
        },
        user: {
          connect: {
            uid: algo.uid,
          },
        },
      },
    });

    return NextResponse.json({ status: 200 });
  } catch (error) {
    console.log(error);
    return NextResponse.json({ message: error }, { status: 500 });
  }
}
