import "./globals.css";
import type { Metadata } from "next";
import { Inter } from "next/font/google";
import { FirebaseProvider } from "@/app/firebase/context";
import { UserInfoProvider } from "@/app/login/auth/context";
import AuthUpdate from "@/app/login/auth/authUpdate";
const inter = Inter({ subsets: ["latin"] });

export const metadata: Metadata = {
  title: "ICAIF FinRL 2023",
  description: "ICAIF 2023 FinRL Contest",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <UserInfoProvider>
        <FirebaseProvider>
          <AuthUpdate />
          <body className={inter.className}>{children}</body>
        </FirebaseProvider>
      </UserInfoProvider>
    </html>
  );
}
