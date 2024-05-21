import type { NextApiRequest, NextApiResponse } from "next";
import { PrismaClient } from "@prisma/client";

const prisma = new PrismaClient();

export const revalidate = 1;

export async function POST(request: any, res: NextApiResponse) {
  try {
    const { name, email } = await request.json();
    if (!name || !email) {
      return Response.json(
        { message: "Missing required fields" },
        { status: 400 }
      );
    }
    const existingUser = await prisma.user.findUnique({
      where: { email },
    });

    if (existingUser) {
      return Response.json({ message: "User already exists" }, { status: 409 });
    }

    const user = await prisma.user.create({
      data: {
        name,
        email,
      },
    });

    return Response.json(user, {
      status: 201,
    });
  } catch (error) {
    console.error(error);
    return Response.json({ message: error }, { status: 500 });
  }
}
