"use client";
import RedirectOnAuth from "@/app/login/auth/redirectOnAuth";
import Dash from "./dash";

export default function DashLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <section className="h-screen bg-gray-900">
      <RedirectOnAuth />
      {Dash(children)}
    </section>
  );
}
