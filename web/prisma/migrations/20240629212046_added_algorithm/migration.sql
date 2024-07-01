-- CreateTable
CREATE TABLE "algos" (
    "id" TEXT NOT NULL,
    "name" TEXT NOT NULL,
    "description" TEXT NOT NULL,
    "case" TEXT NOT NULL,
    "algoFileKey" TEXT NOT NULL,
    "uid" TEXT NOT NULL,

    CONSTRAINT "algos_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "AlgoFile" (
    "key" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,
    "uid" TEXT NOT NULL,

    CONSTRAINT "AlgoFile_pkey" PRIMARY KEY ("key")
);

-- CreateIndex
CREATE UNIQUE INDEX "algos_name_key" ON "algos"("name");

-- CreateIndex
CREATE UNIQUE INDEX "algos_algoFileKey_key" ON "algos"("algoFileKey");

-- CreateIndex
CREATE INDEX "algos_uid_idx" ON "algos"("uid");

-- AddForeignKey
ALTER TABLE "algos" ADD CONSTRAINT "algos_algoFileKey_fkey" FOREIGN KEY ("algoFileKey") REFERENCES "AlgoFile"("key") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "algos" ADD CONSTRAINT "algos_uid_fkey" FOREIGN KEY ("uid") REFERENCES "users"("uid") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "AlgoFile" ADD CONSTRAINT "AlgoFile_uid_fkey" FOREIGN KEY ("uid") REFERENCES "users"("uid") ON DELETE RESTRICT ON UPDATE CASCADE;
