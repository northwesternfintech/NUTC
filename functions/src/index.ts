/**
 * Import function triggers from their respective submodules:
 *
 * import {onCall} from "firebase-functions/v2/https";
 * import {onDocumentWritten} from "firebase-functions/v2/firestore";
 *
 * See a full list of supported triggers at https://firebase.google.com/docs/functions
 */

// import {onRequest} from "firebase-functions/v2/https";
// import * as logger from "firebase-functions/logger";

// Start writing functions
// https://firebase.google.com/docs/functions/typescript

// export const helloWorld = onRequest((request, response) => {
//   logger.info("Hello logs!", {structuredData: true});
//   response.send("Hello from Firebase!");
// });

import * as functions from "firebase-functions";
import * as crypto from "crypto";
import * as admin from "firebase-admin";

admin.initializeApp({
  credential: admin.credential.applicationDefault(),
  databaseURL: "https://nutc-web-default-rtdb.firebaseio.com/",
});

export const emailApplication = functions.https.onCall(
  async (data, context) => {
    if(!context) {
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

  const link: string =
    `https://us-central1-nutc-web.cloudfunctions.net/approveApplicant?token=${token}`;
  return link;
}

export const approveApplicant = functions.https.onRequest(async (req, res) => {
  //@ts-ignore
  const token: string = req.query.token;
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
  await newref.set(true);
  res.send("Applicant approved successfully");
});
