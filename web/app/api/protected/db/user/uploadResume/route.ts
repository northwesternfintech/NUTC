import { getSession } from "@auth0/nextjs-auth0";
import prisma from '@/prisma/prismaClient'
import { S3Client } from "@aws-sdk/client-s3";
import { PutObjectCommand } from "@aws-sdk/client-s3";
import { getSignedUrl } from "@aws-sdk/s3-request-presigner";
import { NextResponse } from "next/server";
import createDefaultUser from "../defaultUser";

function getS3Client() {
	const region = process.env.AWS_REGION;
	const endpoint = process.env.S3_ENDPOINT;
	const accessKeyId = process.env.AWS_ACCESS_KEY_ID;
	const secretAccessKey = process.env.AWS_SECRET_ACCESS_KEY;

	if (!region || !endpoint || !accessKeyId || !secretAccessKey) {
		throw new Error('Missing AWS credentials');
	}

	return new S3Client({
		region: region,
		endpoint: endpoint,
		forcePathStyle: true,
		credentials: {
			accessKeyId: accessKeyId,
			secretAccessKey: secretAccessKey,
		},
	});
}


export async function GET() {
	const session = await getSession();
	if (!session?.user.sub) {
		return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
	}
	await createDefaultUser(session);

	const uid: string = session.user.sub;
	if (!uid) {
		return NextResponse.json({ error: 'Unauthorized' }, { status: 401 });
	}

	const resume = await prisma.resume.create({
		data: { uid: uid },
	})
	const params = {
		Bucket: 'nutc',
		Key: resume.key,
		contentType: 'application/pdf',
	};

	const s3Client = getS3Client();

	try {
		const command = new PutObjectCommand(params);
		const url = await getSignedUrl(s3Client, command, { expiresIn: 3600 });
		console.log(url);
		return NextResponse.json({ url });
	} catch (error) {
		console.error(error);
		return NextResponse.json({ error: 'Error generating pre-signed URL' }, { status: 500 });
	}
}
