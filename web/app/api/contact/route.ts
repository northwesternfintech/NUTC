import { NextRequest, NextResponse } from 'next/server';
import { SendEmail } from '../sendEmail';

export async function POST(req: NextRequest) {
	const { firstName, lastName, email, message } = await req.json();

	const mailOptions = {
		from: "contact@nutc.io",
		to: 'nuft@u.northwestern.edu',
		subject: `NUTC Contact Form Submission from ${firstName} ${lastName}`,
		text: `${firstName} ${lastName} submitted a contact form on nutc.io. Respond at email: ${email}\n\n${message}`,
	};

	const res = await SendEmail(mailOptions.from, mailOptions.to, mailOptions.subject, mailOptions.text)

	if (res) {
		return NextResponse.json({ success: true }, { status: 200 });
	} else {
		return NextResponse.json({ success: false, error: "Failed to send message" }, { status: 500 });
	}
};
