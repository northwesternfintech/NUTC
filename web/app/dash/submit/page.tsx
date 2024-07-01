import React from "react";
import { getSession } from "@auth0/nextjs-auth0";
import SubmissionForm from "./form";
import { redirect } from "next/navigation";

export default async function SubmitPage() {
  const session = await getSession();

  // should never happen, guarded in a higher up layout
  if (!session?.user) {
    redirect("/api/auth/login");
  }

  return <SubmissionForm user={session.user} />;
}
