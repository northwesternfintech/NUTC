import { S3Client } from "@aws-sdk/client-s3";

const region = process.env.AWS_REGION;
const endpoint = process.env.S3_ENDPOINT;
const accessKeyId = process.env.AWS_ACCESS_KEY_ID;
const secretAccessKey = process.env.AWS_SECRET_ACCESS_KEY;

if (!region || !endpoint || !accessKeyId || !secretAccessKey) {
  throw new Error("Missing AWS credentials");
}

const s3Client = new S3Client({
  region,
  endpoint,
  forcePathStyle: true,
  credentials: {
    accessKeyId,
    secretAccessKey,
  },
});

export default s3Client;
