import { SendEmail } from "../../../../sendEmail";
import { Profile, User } from "@prisma/client";
import prisma from "@/prisma/prismaClient";

export async function GenerateApplicationEmail(user: User, profile: Profile) {
  const token = (await prisma.emailTokens.create({ data: { uid: user.uid } }))
    .token;

  const s3Key = await prisma.user.findUnique({
    where: {
      uid: user.uid,
    },
    select: {
      Resume: {
        select: {
          s3Key: true,
        },
      },
    },
  });

  const link = process.env.AUTH0_BASE_URL + `/api/handleReview?token=${token}`;
  const accept_link = link + "&accept=true";
  const deny_link = link + "&accept=false";
  const resume_link = `${process.env.EXTERNAL_S3_ENDPOINT}/nutc/${s3Key?.Resume?.at(-1)?.s3Key
    }`;

  const mailOptions = {
    from: "contact@nutc.io",
    to: "nuft@u.northwestern.edu",
    subject: `[ACTION REQUIRED] NUTC Application Submitted`,
    text: `<ul>
		<li>First Name: ${profile.firstName}</li>
		<li>Last Name: ${profile.lastName}</li>
		<li>Email: ${user.email}</li>
		<li>School: ${profile.school}</li>
		<li>Graduation Year: ${profile.year}</li>
    <li><a href=${resume_link}>Resume</a></li>
		<li><a href="${accept_link}">Accept</a></li>
		<li><a href="${deny_link}">Deny</a></li>
		</ul>`,
  };

  return await SendEmail(
    mailOptions.from,
    mailOptions.to,
    mailOptions.subject,
    mailOptions.text,
  );
}
