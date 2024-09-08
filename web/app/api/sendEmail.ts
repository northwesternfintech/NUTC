import nodemailer from 'nodemailer';

const transporter = nodemailer.createTransport({
	host: 'smtp-relay.brevo.com',
	port: 587,
	auth: {
		user: process.env.SENDINBLUE_USER,
		pass: process.env.SENDINBLUE_API_KEY,
	},
});

export async function SendEmail(from: string, to: string, subject: string, text: string) {
	const mailOptions = { from, to, subject, text };

	try {
		await transporter.sendMail(mailOptions);
		return true;
	} catch (error) {
		console.log(error);
		return false;
	}
};
