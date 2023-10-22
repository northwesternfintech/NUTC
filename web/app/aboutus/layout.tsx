import Header from "./header";
export default function RegistrationLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <section className="bg-gray-900">
      <Header />
      <div className=" mx-auto max-w-7xl px-4 pb-12 pt-6 sm:px-6 sm:pb-24 sm:pt-12 lg:px-8">
        <div className="mx-auto max-w-2xl">
          {children}
        </div>
      </div>
    </section>
  );
}
