import "./globals.css";
import type { Metadata } from "next";
import { Inter } from "next/font/google";
import { FirebaseProvider } from "@/app/firebase/context";
const inter = Inter({ subsets: ["latin"] });

export const metadata: Metadata = {
  title: "NUTC 2024",
  description: "NUFT Trading Competition",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <FirebaseProvider>
        <body className={inter.className}>{children}</body>
      </FirebaseProvider>
    </html>
  );
}
