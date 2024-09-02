import { getSession } from "@auth0/nextjs-auth0";
import prisma from "@/prisma/prismaClient";
import s3Client from "@/lib/s3";
import {
  PutObjectCommand,
  type PutObjectCommandInput,
} from "@aws-sdk/client-s3";
import { getSignedUrl } from "@aws-sdk/s3-request-presigner";
import { NextResponse } from "next/server";
import createDefaultUser from "../defaultUser";

export async function GET() {
  const session = await getSession();
  if (!session?.user.sub) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }
  await createDefaultUser(session);

  const uid: string = session.user.sub;
  if (!uid) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }

  const resume = await prisma.resume.create({
    data: { uid: uid },
  });
  const params = {
    Bucket: "nutc",
    Key: resume.s3Key,
    ContentType: "application/pdf",
  } satisfies PutObjectCommandInput;

  try {
    const command = new PutObjectCommand(params);
    const url = await getSignedUrl(s3Client, command, { expiresIn: 3600 });
    console.log(url);
    return NextResponse.json({ url });
  } catch (error) {
    console.error(error);
    return NextResponse.json(
      { error: "Error generating pre-signed URL" },
      { status: 500 },
    );
  }
}
