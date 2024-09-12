import { NextResponse } from "next/server";
import { getSession } from "@auth0/nextjs-auth0/edge";

// Ensure user is logged in and user object is created
export async function middleware() {
  try {
    const session = await getSession();
    if (!session?.user.sub) {
      return NextResponse.redirect("/api/auth/login");
    }

    return NextResponse.next();
  } catch (error) {
    return NextResponse.json({ message: "Unauthorized" }, { status: 400 });
  }
}

export const config = {
  matcher: ["/api/protected/:path*"],
};
