"use client";
import { connectDatabaseEmulator, getDatabase } from "firebase/database";
import { connectStorageEmulator, getStorage } from "firebase/storage";
import firebase from "firebase/compat/app";
import "firebase/compat/auth";
import "firebase/compat/functions";

const firebaseConfig = {
  apiKey: "AIzaSyCa6g_38lybuuzfoBunEOhBUYwSavtF1rw",
  authDomain: "finrl-contest-f019d.firebaseapp.com",
  projectId: "finrl-contest-f019d",
  storageBucket: "finrl-contest-f019d.appspot.com",
  messagingSenderId: "382005499144",
  appId: "1:382005499144:web:739edf880af14dbc2388bd",
  measurementId: "G-QYWBD8SJPJ"
};

var database: any;
var storage: any;
var functions: any;
const app = firebase.initializeApp(firebaseConfig);

const isLocalhost = () => {
  return process && process.env.NODE_ENV === "development";
};

if (isLocalhost()) {
  database = getDatabase();
  storage = getStorage();
  connectDatabaseEmulator(database, "localhost", 9000);
  connectStorageEmulator(storage, "localhost", 9199);
  firebase.functions().useEmulator("localhost", 5001);
  firebase.auth().useEmulator("http://localhost:9099");
  functions = firebase.functions();
  if (
    typeof sessionStorage != "undefined" &&
    !sessionStorage.getItem("givenWarning")
  ) {
    alert(
      "Initializing in emulator mode. If you aren't a developer, contact support@nuft_getdomain.com immediately."
    );
    sessionStorage.setItem("givenWarning", "true");
  }
} else {
  storage = getStorage(app);
  database = getDatabase(app);
  functions = firebase.functions();
}

export { app, database, functions, storage };
