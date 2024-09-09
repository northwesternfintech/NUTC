import prisma from '@/prisma/prismaClient'
import { Claims } from '@auth0/nextjs-auth0'
import { ParticipantState } from '@prisma/client';

export default async function createDefaultUser(session: Claims) {
	const user = await prisma.user.findUnique({
		where: { uid: session.user.sub },
	})
	if (user) {
		return user;
	}
	return await prisma.user.create({
		data: {
			uid: session.user.sub,
			email: session.user.email,
			participantState: ParticipantState.WAITING,
		},
	})
}
