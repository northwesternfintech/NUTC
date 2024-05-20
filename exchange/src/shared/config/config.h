#pragma once

// Compile-time config shared between exchange and wrapper

#define NUTC_VERSION          "1.0"

#ifdef NUTC_LOCAL_DEV
#define FIREBASE_URL "http://firebase:9000/"
#else
#define FIREBASE_URL "https://nutc-web-default-rtdb.firebaseio.com/"
#endif
