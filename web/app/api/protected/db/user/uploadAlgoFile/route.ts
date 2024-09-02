import { getSession } from "@auth0/nextjs-auth0";
import prisma from "@/prisma/prismaClient";
import s3Client from "@/lib/s3";
import {
  PutObjectCommand,
  type PutObjectCommandInput,
} from "@aws-sdk/client-s3";
import { getSignedUrl } from "@aws-sdk/s3-request-presigner";
import { NextResponse } from "next/server";

export async function POST() {
  const session = await getSession();
  if (!session?.user.sub) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }

  const uid: string = session.user.sub;
  if (!uid) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }

  const algoFile = await prisma.algoFile.create({
    data: { uid },
  });

  const params = {
    Bucket: "nutc",
    Key: algoFile.s3Key,
    ContentType: "text/x-python",
  } satisfies PutObjectCommandInput;

  try {
    const command = new PutObjectCommand(params);
    const url = await getSignedUrl(s3Client, command, { expiresIn: 3600 });
    return NextResponse.json({ url: url, key: algoFile.s3Key });
  } catch (error) {
    console.error(error);
    return NextResponse.json(
      { error: "Error generating pre-signed URL" },
      { status: 500 },
    );
  }
}
