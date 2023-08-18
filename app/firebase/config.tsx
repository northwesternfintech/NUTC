"use client";
import { connectDatabaseEmulator, getDatabase } from "firebase/database";
import { connectStorageEmulator, getStorage } from "firebase/storage";
import firebase from "firebase/compat/app";
import "firebase/compat/auth";
import "firebase/compat/functions";

const firebaseConfig = {
  apiKey: "AIzaSyCo2l3x2DMhg5CaNy1Pyvknk_GK8v34iUc",
  authDomain: "nutc-web.firebaseapp.com",
  databaseURL: "https://nutc-web-default-rtdb.firebaseio.com",
  projectId: "nutc-web",
  storageBucket: "nutc-web.appspot.com",
  messagingSenderId: "690190717923",
  appId: "1:690190717923:web:bbab373b3199cdbba606d6",
};

var database: any;
var storage: any;
var functions: any;
const app = firebase.initializeApp(firebaseConfig);

const isLocalhost = () => {
  return (process && process.env.NODE_ENV === "development");
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
      "Initializing in emulator mode. If you aren't a developer, contact support@nuft_getdomain.com immediately.",
    );
    sessionStorage.setItem("givenWarning", "true");
  }
} else {
  storage = getStorage(app);
  database = getDatabase(app);
  functions = firebase.functions();
}

export { app, database, functions, storage };
