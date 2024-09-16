import { ParticipantState } from "@prisma/client";
import prisma from "@/prisma/prismaClient";
import { NextRequest, NextResponse } from "next/server";
import { SendEmail } from "../sendEmail";

export async function GET(req: NextRequest) {
	// This is shit. Please replace this if you see it.
	try {
		const token = req.nextUrl.searchParams.get("token") || "";
		const accepted = req.nextUrl.searchParams.get("accept") == "true";
		const uid =
			(await prisma.emailTokens.findUnique({ where: { token } }))?.uid || "";
		console.log("Uid: " + uid + ", accepted: " + accepted);
		await prisma.user.update({
			where: {
				uid,
			},
			data: {
				participantState: accepted
					? ParticipantState.ACCEPTED
					: ParticipantState.REJECTED,
			},
		});
		const { profile, email } = (await prisma.user.findUnique({
			where: {
				uid: uid,
			},
			select: {
				email: true,
				profile: true
			}
		})) || {};
		var text;
		var subject;
		if (accepted) {
			text = `Congratulations ${profile?.firstName}! You've been accepted to the Northwestern Trading Competition. Sponsors, free merch, prizes, presentations, and live results will take place in-person at Northwestern on October 19th from 10am to 3pm. Please *reply to this email* to confirm that you can attend in-person, as well as if you require travel reimbursement. Note that travel reimbursement is approved on a case-by-case basis. See you soon!`;
			subject = "[ACTION REQUIRED] NUTC Application Update";
		} else {
			text = "Unfortunately, you were not selected to join the Northwestern Trading Competition this year. We encourage you to re-apply next cycle.";
			subject = "NUTC Application Update";
		}
		await SendEmail("contact@nuft.io", email || "nuft@u.northwestern.edu", subject, text);
		return NextResponse.json({ message: "Successful" }, { status: 200 });
	} catch (error) {
		console.log(error);
		return NextResponse.json({ message: error }, { status: 500 });
	}
}
