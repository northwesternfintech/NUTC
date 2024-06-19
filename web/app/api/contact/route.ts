import { NextRequest, NextResponse } from 'next/server';
import nodemailer from 'nodemailer';

const transporter = nodemailer.createTransport({
	host: 'smtp-relay.brevo.com',
	port: 587,
	auth: {
		user: process.env.SENDINBLUE_USER,
		pass: process.env.SENDINBLUE_API_KEY,
	},
});

export async function POST(req: NextRequest) {
	const { firstName, lastName, email, message } = await req.json();

	const mailOptions = {
		from: "contact@nutc.io",
		to: 'nuft@u.northwestern.edu',
		subject: `NUTC Contact Form Submission from ${firstName} ${lastName}`,
		text: `${firstName} ${lastName} submitted a contact form on nutc.io. Respond at email: ${email}\n\n${message}`,
	};

	try {
		await transporter.sendMail(mailOptions);
		return NextResponse.json({ success: true }, { status: 200 });
	} catch (error) {
		console.log(error);
		return NextResponse.json({ success: false, error: "Failed to send message" }, { status: 500 });
	}
};
