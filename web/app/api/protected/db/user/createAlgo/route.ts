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
      !algo.algoFileS3Key
    ) {
      return new Response("Not all fields in algo added", { status: 402 });
    }

    const session = await getSession();
    if (!session?.user.sub) {
      return NextResponse.json({ message: "Unauthorized" }, { status: 402 });
    }
    const uid = session.user.sub;

    // TODO: validation
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
            uid
          },
        },
      },
    });

    const url = `${process.env.WEBSERVER_INTERNAL_ENDPOINT}/submit/${algo.algoFileS3Key}/${algo.language}`;
    console.log("Fetching " + url);
    const submission_response = await fetch(
      url,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
      },
    );
    if (!submission_response.ok) {
      console.log("Failed to lint/sandbox");
    }
    return submission_response;
  } catch (error) {
    console.log(error);
    return NextResponse.json({ message: error }, { status: 500 });
  }
}
