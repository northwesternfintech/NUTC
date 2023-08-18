import {
  approvalEmailToApplicant,
  rejectionEmailToApplicant,
  studentHasApplied,
} from "./emails";
import * as functions from "firebase-functions";
import * as crypto from "crypto";
import * as admin from "firebase-admin";
import * as nodemailer from "nodemailer";

const transporter = nodemailer.createTransport({
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
    if (!context.auth) {
      throw new functions.https.HttpsError(
        "unauthenticated",
        "The function must be called while authenticated.",
      );
    }
    const uid = context.auth.uid;
    const approvalLink = await generateApprovalLink(uid);
    const rejectionLink = await generateRejectionLink(uid);
    const userInfo =
      (await admin.database().ref("users").child(uid).once("value")).val();
    const userName = userInfo.username;
    const about = userInfo.about;
    const email = userInfo.email;
    const firstName = userInfo.firstName;
    const lastName = userInfo.lastName;
    const school = userInfo.school;
    const resumeURL = userInfo.resumeURL;
    const emailText = studentHasApplied(
      firstName,
      lastName,
      email,
      about,
      userName,
      school,
      resumeURL,
      approvalLink,
      rejectionLink,
    );
    const mailOptions = {
      from: "noreply@nutc.site",
      to: "steveewald2025@u.northwestern.edu",
      subject: "[ACTION REQUIRED] NUTC Application Submitted",
      html: emailText,
    };
    transporter.sendMail(mailOptions, (errno: any, _: any) => {
      if (errno) {
        throw new functions.https.HttpsError(
          "internal",
          errno.toString(),
        );
      }
    });
    return true;
  },
);

async function generateApprovalLink(uid: any) {
  const token: string = crypto.randomBytes(16).toString("hex");
  const ref = admin.database().ref("approvalTokens").child(token);
  await ref.set(uid);
  // const region = "https://us-central1-nutc-web.cloudfunctions.net";
  const region = "http://127.0.0.1:5001/nutc-web/us-central1";

  const link = `${region}/approveApplicant?token=${token}`;
  console.log("Approval: " + link);
  return link;
}

async function generateRejectionLink(uid: any) {
  const token: string = crypto.randomBytes(16).toString("hex");
  const ref = admin.database().ref("rejectionTokens").child(token);
  await ref.set(uid);
  // const region = "https://us-central1-nutc-web.cloudfunctions.net";
  const region = "http://127.0.0.1:5001/nutc-web/us-central1";

  const link = `${region}/rejectApplicant?token=${token}`;
  console.log("Rejection: " + link);
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
  const emailpromise = await admin.database().ref("users").child(uid).child(
    "email",
  ).once("value");
  const email = emailpromise.val();
  const namePromise = await admin.database().ref("users").child(uid).child(
    "firstName",
  ).once("value");
  const name = namePromise.val();
  await newref.set(true);
  const mailOptions = {
    from: "noreply@nutc.site",
    to: email,
    subject: "[ACTION REQUIRED] NUTC Application Approved",
    html: approvalEmailToApplicant(name),
  };
  transporter.sendMail(mailOptions, (errno: any, _: any) => {
    if (errno) {
      res.status(400).send(errno.toString());
    } else {
      res.send("Approved " + mailOptions.to + " successfully");
    }
  });
});

export const rejectApplicant = functions.https.onRequest(async (req, res) => {
  const token: string = req.query.token as string;
  const ref = admin.database().ref("rejectionTokens").child(token);
  const uid_to_approve = await ref.once("value");
  if (!uid_to_approve.exists()) {
    res.status(400).send("Invalid token");
    return;
  }
  const uid: string = uid_to_approve.val();
  const newref = admin.database().ref("users").child(uid).child(
    "isRejectedApplicant",
  );
  const emailpromise = await admin.database().ref("users").child(uid).child(
    "email",
  ).once("value");
  const email = emailpromise.val();
  const namePromise = await admin.database().ref("users").child(uid).child(
    "firstName",
  ).once("value");
  const name = namePromise.val();
  await newref.set(true);
  const mailOptions = {
    from: "noreply@nutc.site",
    to: email,
    subject: "NUTC Application Update",
    html: rejectionEmailToApplicant(name),
  };
  transporter.sendMail(mailOptions, (errno: any, _: any) => {
    if (errno) {
      res.status(400).send(errno.toString());
    } else {
      res.send("Rejected " + mailOptions.to + " successfully");
    }
  });
});
