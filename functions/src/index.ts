import { emailToApplicant } from "./emails";
import * as functions from "firebase-functions";
import * as crypto from "crypto";
import * as admin from "firebase-admin";
import * as nodemailer from "nodemailer";

let transporter = nodemailer.createTransport({
  host: "smtp-relay.sendinblue.com",
  port: 587,
  auth: {
    user: functions.config().sendinblue.user,
    pass: functions.config().sendinblue.password,
  },
});

admin.initializeApp({
  credential: admin.credential.applicationDefault(),
  databaseURL: "https://nutc-web-default-rtdb.firebaseio.com/",
});

export const emailApplication = functions.https.onCall(
  async (data, context) => {
    if (!context) {
      throw new functions.https.HttpsError(
        "unauthenticated",
        "what lol",
      );
    }
    if (!context.auth) {
      throw new functions.https.HttpsError(
        "unauthenticated",
        "The function must be called while authenticated.",
      );
    }
    const uid = context.auth.uid;
    const link = generateApprovalLink(uid);
    console.log(link);
    return link;
  },
);

async function generateApprovalLink(uid: any) {
  const token: string = crypto.randomBytes(16).toString("hex");
  const ref = admin.database().ref("approvalTokens").child(token);
  await ref.set(uid);
  // const region = "https://us-central1-nutc-web.cloudfunctions.net";
  const region = "http://127.0.0.1:5001/nutc-web/us-central1";

  const link: string = `${region}/approveApplicant?token=${token}`;
  console.log("Applicant approval link generated: " + link);
  return link;
}

export const approveApplicant = functions.https.onRequest(async (req, res) => {
  const token: string = req.query.token as string;
  const ref = admin.database().ref("approvalTokens").child(token);
  const uid_to_approve = await ref.once("value");
  if (!uid_to_approve.exists()) {
    res.status(400).send("Invalid token");
    return;
  }
  const uid: string = uid_to_approve.val();
  const newref = admin.database().ref("users").child(uid).child(
    "isApprovedApplicant",
  );
  const emailpromise = await admin.database().ref("users").child(uid).child("email").once("value");
  const email = emailpromise.val();
  const namePromise = await admin.database().ref("users").child(uid).child("firstName").once("value");
  const name = namePromise.val();
  await newref.set(true);
  const mailOptions = {
    from: "noreply@nutc.site",
    to: email,
    subject: "NUTC Application Approved",
    html: emailToApplicant(name)
  };
  transporter.sendMail(mailOptions, (errno: any, _: any) => {
    if (errno) {
      res.status(400).send(errno.toString());
    } else {
      res.send("Approved " + mailOptions.to + " successfully");
    }
  });
});
