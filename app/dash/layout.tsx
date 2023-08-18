import RedirectOnAuth from "@/app/login/auth/redirectOnAuth";
export default function DashLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <section className="h-screen bg-gray-900">
      <RedirectOnAuth />
      {children}
    </section>
  );
}

