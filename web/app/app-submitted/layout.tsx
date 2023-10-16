import RedirectOnAuth from "@/app/login/auth/redirectOnAuth";
export default function AppSubmittedLayout({
  children,
}: {
  children: React.ReactNode;
})  {
  return (
    <section className="h-screen bg-gray-900">
      <RedirectOnAuth page="registration" />
      {children}
    </section>
  );
}
