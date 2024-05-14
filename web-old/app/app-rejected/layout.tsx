import RedirectOnAuth from "@/app/login/auth/redirectOnAuth";
export default function AppRejectedLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <section className="h-screen bg-gray-900">
      <RedirectOnAuth page="app-rejected" />
      {children}
    </section>
  );
}
