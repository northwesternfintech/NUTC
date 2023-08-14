import RedirectOnAuth from "./auth/redirectOnAuth";
export default function LoginLayout({
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
