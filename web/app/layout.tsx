import type { Metadata } from "next";
import { Inter } from "next/font/google";
import "./globals.css";

import { UserProvider } from "@auth0/nextjs-auth0/client";
import AuthGuardRedirectToRegistration from "./utils/AuthGuard";

const inter = Inter({ subsets: ["latin"] });

export const metadata: Metadata = {
  title: "NUTC",
  description: "Northwestern Trading Competition",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en">
      <UserProvider>
        <AuthGuardRedirectToRegistration />
        <body className={inter.className}>{children}</body>
      </UserProvider>
    </html>
  );
}
