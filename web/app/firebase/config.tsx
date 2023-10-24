"use client";
import { connectDatabaseEmulator, getDatabase } from "firebase/database";
import { connectStorageEmulator, getStorage } from "firebase/storage";
import firebase from "firebase/compat/app";
import "firebase/compat/auth";
import "firebase/compat/functions";

const firebaseConfig = {
  apiKey: "AIzaSyD1_vt8MWf2gJLUPcWl5iXrWVSQJ56yOTM",
  authDomain: "finrl-contest-2023.firebaseapp.com",
  projectId: "finrl-contest-2023",
  storageBucket: "finrl-contest-2023.appspot.com",
  messagingSenderId: "987972509982",
  appId: "1:987972509982:web:9f503ee5685b9620c9690f",
  measurementId: "G-2BS37K5ZZX",
};

var database: any;
var storage: any;
var functions: any;
const app = firebase.initializeApp(firebaseConfig);

const isLocalhost = () => {
  return process && process.env.NODE_ENV === "development";
};

// if (isLocalhost()) {
//   database = getDatabase();
//   storage = getStorage();
//   connectDatabaseEmulator(database, "localhost", 9000);
//   connectStorageEmulator(storage, "localhost", 9199);
//   firebase.functions().useEmulator("localhost", 5001);
//   firebase.auth().useEmulator("http://localhost:9099");
//   functions = firebase.functions();
//   if (
//     typeof sessionStorage != "undefined" &&
//     !sessionStorage.getItem("givenWarning")
//   ) {
//     alert(
//       "Initializing in emulator mode. If you aren't a developer, contact support@nuft_getdomain.com immediately."
//     );
//     sessionStorage.setItem("givenWarning", "true");
//   }
// } else {
storage = getStorage(app);
database = getDatabase(app);
functions = firebase.functions();
// }

export { app, database, functions, storage };
