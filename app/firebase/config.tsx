import { initializeApp } from 'firebase/app';

const firebaseConfig = {
  apiKey: "AIzaSyCo2l3x2DMhg5CaNy1Pyvknk_GK8v34iUc",
  authDomain: "nutc-web.firebaseapp.com",
  databaseURL: "https://nutc-web-default-rtdb.firebaseio.com",
  projectId: "nutc-web",
  storageBucket: "nutc-web.appspot.com",
  messagingSenderId: "690190717923",
  appId: "1:690190717923:web:bbab373b3199cdbba606d6"
};

export const firebaseApp = initializeApp(firebaseConfig);
