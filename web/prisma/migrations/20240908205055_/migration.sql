-- CreateTable
CREATE TABLE "email_tokens" (
    "token" TEXT NOT NULL,
    "uid" TEXT NOT NULL,

    CONSTRAINT "email_tokens_pkey" PRIMARY KEY ("token")
);
