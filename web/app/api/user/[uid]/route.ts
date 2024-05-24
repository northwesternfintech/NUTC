import { PrismaClient } from "@prisma/client";
import { NextApiRequest, NextApiResponse } from "next";

const prisma = new PrismaClient();
// GET /api/user/[...uid]
export async function GET(
  request: Request,
  { params }: { params: { uid: string } }
) {
  try {
    const uid = params.uid;
    if (!uid) {
      return Response.json(
        { message: "Missing required fields" },
        { status: 400 }
      );
    }
    const user = await prisma.user.findUnique({
      where: { uid },
    });
    if (!user) {
      return Response.json({ message: "User not found" }, { status: 404 });
    }
    return Response.json(user, {
      status: 200,
    });
  } catch (error) {
    console.error(error);
    return Response.json({ message: error }, { status: 500 });
  }
}
