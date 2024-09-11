import { SendEmail } from '../../../../sendEmail';
import { Profile, User } from '@prisma/client';
import prisma from "@/prisma/prismaClient";

export async function GenerateApplicationEmail(user: User, profile: Profile) {
	const token = (await prisma.emailTokens.create({ data: { uid: user.uid } })).token;

	const link = process.env.AUTH0_BASE_URL + `/handleReview?token=${token}`;
	const accept_link = link + "&accept=true";
	const deny_link = link + "&accept=false";

	const mailOptions = {
		from: "contact@nutc.io",
		to: 'steveewald2025@u.northwestern.edu',
		subject: `[ACTION REQUIRED] NUTC Application Submitted`,
		text: `${profile.firstName} ${profile.lastName} submitted an application. Click <a href="${accept_link}">Here</a> to accept. and <a href="${deny_link}"> here </a> to decline.`
	};

	return await SendEmail(mailOptions.from, mailOptions.to, mailOptions.subject, mailOptions.text)
};