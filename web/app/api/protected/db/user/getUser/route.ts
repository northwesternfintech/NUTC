import { getSession } from "@auth0/nextjs-auth0";
import { NextResponse } from "next/server";
import getUser from "../getUser";

export async function GET() {
  try {
    const session = await getSession();

    if (!session?.user?.sub) {
      return NextResponse.json({ message: "Not logged in" }, { status: 401 });
    }

    const user = await getUser(session.user.sub);
    if (user == null) {
      return NextResponse.json({ message: "User not found" }, { status: 400 });
    }

    return NextResponse.json({ user, status: 200 });
  } catch (error: any) {
    console.error(error);
    return NextResponse.json({ message: error.message }, { status: 500 });
  }
}
