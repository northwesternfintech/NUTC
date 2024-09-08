import { getAlgos, redirectIfStateNot } from "@/api";
import { DashNav } from "./dash-nav";
import { redirect } from "next/navigation";
import { ParticipantState } from "@prisma/client";

export default async function DashLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  await redirectIfStateNot(ParticipantState.ACCEPTED);

  const algos = await getAlgos();
  if (!algos) {
    redirect("/api/auth/login");
  }
  return <DashNav algos={algos}>{children}</DashNav>;
}
