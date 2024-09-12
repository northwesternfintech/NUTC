"use server";
import { Claims, getSession } from "@auth0/nextjs-auth0";
import { redirect } from "next/navigation";
import getUser from "@/app/api/protected/db/user/getUser";
import Form from "./Form";

async function userExistsInDb(session: Claims) {
  const user = await getUser(session?.user.sub);
  return user?.profile != null;
}

export default async function emailInput() {
  const session = await getSession();

  if (!session?.user) {
    redirect("/api/auth/login");
  }

  const userExists = await userExistsInDb(session);
  if (userExists) {
    redirect("/dash");
  }

  return <Form user={session.user} />;
}
