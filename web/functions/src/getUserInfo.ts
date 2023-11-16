import * as functions from "firebase-functions";
import * as admin from "firebase-admin";

admin.initializeApp();

export const getUserInfo = functions.https.onCall(async (data, context) => {

  const { userId, apiKey } = data;

  if (!apiKeyIsVerified(apiKey)) {
    throw new functions.https.HttpsError(
      "permission-denied",
      "Invalid API key."
    );
  }

  try {
    const userInfoSnapshot = await admin.database().ref(`users/${userId}`).once("value");

    if (!userInfoSnapshot.exists()) {
      throw new functions.https.HttpsError(
        "not-found",
        "User not found."
      );
    }

    return userInfoSnapshot.val();
  } catch (error : any) {
    throw new functions.https.HttpsError(
      "internal",
      "An error occurred while retrieving user information.",
      error.message
    );
  }
});

function apiKeyIsVerified(apiKey: string) {
  // replace with actual api key value
  return apiKey === 'your_expected_api_key';
}