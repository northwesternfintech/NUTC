import { getSession } from "@auth0/nextjs-auth0";
import { getUserRegistrationState } from "@/api";
import { ParticipantState } from "@prisma/client";

export default async function DashOrRegister() {
  const session = await getSession();

  const registrationState = await getUserRegistrationState();

  if (
    session?.user == null ||
    registrationState == null ||
    registrationState === ParticipantState.PRE_REGISTRATION
  ) {
    return (
      <a
        href="/register"
        className="text-md font-semibold leading-6 text-white">
        Sign in <span aria-hidden="true">&rarr;</span>
      </a>
    );
  }
  // TODO: Change above to "Apply" when apps reopen

  return (
    <a href="/dash" className="text-md font-semibold leading-6 text-white">
      Dashboard <span aria-hidden="true">&rarr;</span>
    </a>
  );
}
