"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.rejectApplicant = exports.approveApplicant = exports.emailApplication = void 0;
const emails_1 = require("./emails");
const functions = require("firebase-functions");
const crypto = require("crypto");
const admin = require("firebase-admin");
const nodemailer = require("nodemailer");
const dev_mode = process.env.FUNCTIONS_EMULATOR;
const transporter = nodemailer.createTransport({
    host: "smtp-relay.sendinblue.com",
    port: 587,
    auth: {
        user: dev_mode ? "" : functions.config().sendinblue.user,
        pass: dev_mode ? "" : functions.config().sendinblue.password,
    },
});
admin.initializeApp({
    credential: admin.credential.applicationDefault(),
    databaseURL: "https://nutc-web-default-rtdb.firebaseio.com/",
});
exports.emailApplication = functions.https.onCall(async (data, context) => {
    if (!context.auth) {
        throw new functions.https.HttpsError("unauthenticated", "The function must be called while authenticated.");
    }
    const uid = context.auth.uid;
    if (dev_mode) {
        await admin.database().ref("users").child(uid).child("isApprovedApplicant").set(true);
        return true;
    }
    const approvalLink = await generateApprovalLink(uid);
    const rejectionLink = await generateRejectionLink(uid);
    const userInfo = (await admin.database().ref("users").child(uid).once("value")).val();
    const userName = userInfo.username;
    const about = userInfo.about;
    const email = userInfo.email;
    const firstName = userInfo.firstName;
    const lastName = userInfo.lastName;
    const school = userInfo.school;
    const resumeURL = userInfo.resumeURL;
    const emailText = (0, emails_1.studentHasApplied)(firstName, lastName, email, about, userName, school, resumeURL, approvalLink, rejectionLink);
    const mailOptions = {
        from: "noreply@nutc.site",
        // to: "nuft@u.northwestern.edu",
        to: "steveewald2025@u.northwestern.edu",
        subject: "[ACTION REQUIRED] NUTC Application Submitted",
        html: emailText,
    };
    transporter.sendMail(mailOptions, (errno, _) => {
        if (errno) {
            throw new functions.https.HttpsError("internal", errno.toString());
        }
    });
    return true;
});
async function generateApprovalLink(uid) {
    const token = crypto.randomBytes(16).toString("hex");
    const ref = admin.database().ref("approvalTokens").child(token);
    await ref.set(uid);
    const region = "https://us-central1-nutc-web.cloudfunctions.net";
    // const region = "http://127.0.0.1:5001/nutc-web/us-central1";
    const link = `${region}/approveApplicant?token=${token}`;
    console.log("Approval: " + link);
    return link;
}
async function generateRejectionLink(uid) {
    const token = crypto.randomBytes(16).toString("hex");
    const ref = admin.database().ref("rejectionTokens").child(token);
    await ref.set(uid);
    const region = "https://us-central1-nutc-web.cloudfunctions.net";
    // const region = "http://127.0.0.1:5001/nutc-web/us-central1";
    const link = `${region}/rejectApplicant?token=${token}`;
    console.log("Rejection: " + link);
    return link;
}
exports.approveApplicant = functions.https.onRequest(async (req, res) => {
    const token = req.query.token;
    const ref = admin.database().ref("approvalTokens").child(token);
    const uid_to_approve = await ref.once("value");
    if (!uid_to_approve.exists()) {
        res.status(400).send("Invalid token");
        return;
    }
    const uid = uid_to_approve.val();
    const newref = admin.database().ref("users").child(uid).child("isApprovedApplicant");
    const emailpromise = await admin.database().ref("users").child(uid).child("email").once("value");
    const email = emailpromise.val();
    const namePromise = await admin.database().ref("users").child(uid).child("firstName").once("value");
    const name = namePromise.val();
    await newref.set(true);
    const mailOptions = {
        from: "noreply@nutc.site",
        to: email,
        subject: "[ACTION REQUIRED] NUTC Application Approved",
        html: (0, emails_1.approvalEmailToApplicant)(name),
    };
    transporter.sendMail(mailOptions, (errno, _) => {
        if (errno) {
            res.status(400).send(errno.toString());
        }
        else {
            res.send("Approved " + mailOptions.to + " successfully");
        }
    });
});
exports.rejectApplicant = functions.https.onRequest(async (req, res) => {
    const token = req.query.token;
    const ref = admin.database().ref("rejectionTokens").child(token);
    const uid_to_approve = await ref.once("value");
    if (!uid_to_approve.exists()) {
        res.status(400).send("Invalid token");
        return;
    }
    const uid = uid_to_approve.val();
    const newref = admin.database().ref("users").child(uid).child("isRejectedApplicant");
    const emailpromise = await admin.database().ref("users").child(uid).child("email").once("value");
    const email = emailpromise.val();
    const namePromise = await admin.database().ref("users").child(uid).child("firstName").once("value");
    const name = namePromise.val();
    await newref.set(true);
    const mailOptions = {
        from: "noreply@nutc.site",
        to: email,
        subject: "NUTC Application Update",
        html: (0, emails_1.rejectionEmailToApplicant)(name),
    };
    transporter.sendMail(mailOptions, (errno, _) => {
        if (errno) {
            res.status(400).send(errno.toString());
        }
        else {
            res.send("Rejected " + mailOptions.to + " successfully");
        }
    });
});
//# sourceMappingURL=index.js.map