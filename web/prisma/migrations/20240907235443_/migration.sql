/*
  Warnings:

  - The primary key for the `algos` table will be changed. If it partially fails, the table could be left without primary key constraint.
  - You are about to drop the column `algoFileKey` on the `algos` table. All the data in the column will be lost.
  - You are about to drop the column `id` on the `algos` table. All the data in the column will be lost.
  - The primary key for the `resumes` table will be changed. If it partially fails, the table could be left without primary key constraint.
  - You are about to drop the column `key` on the `resumes` table. All the data in the column will be lost.
  - You are about to drop the `AlgoFile` table. If the table is not empty, all the data it contains will be lost.
  - Added the required column `algoFileS3Key` to the `algos` table without a default value. This is not possible if the table is not empty.
  - Added the required column `lintResults` to the `algos` table without a default value. This is not possible if the table is not empty.
  - The required column `s3Key` was added to the `resumes` table with a prisma-level default value. This is not possible if the table is not empty. Please add this column as optional, then populate it before making it required.

*/
-- DropForeignKey
ALTER TABLE "AlgoFile" DROP CONSTRAINT "AlgoFile_uid_fkey";

-- DropForeignKey
ALTER TABLE "algos" DROP CONSTRAINT "algos_algoFileKey_fkey";

-- DropIndex
DROP INDEX "algos_algoFileKey_key";

-- AlterTable
ALTER TABLE "algos" DROP CONSTRAINT "algos_pkey",
DROP COLUMN "algoFileKey",
DROP COLUMN "id",
ADD COLUMN     "algoFileS3Key" TEXT NOT NULL,
ADD COLUMN     "lintFailureMessage" TEXT,
ADD COLUMN     "lintResults" TEXT NOT NULL,
ADD COLUMN     "lintSuccessMessage" TEXT,
ADD COLUMN     "sandboxLogFileURL" TEXT,
ADD CONSTRAINT "algos_pkey" PRIMARY KEY ("algoFileS3Key");

-- AlterTable
ALTER TABLE "resumes" DROP CONSTRAINT "resumes_pkey",
DROP COLUMN "key",
ADD COLUMN     "s3Key" TEXT NOT NULL,
ADD CONSTRAINT "resumes_pkey" PRIMARY KEY ("s3Key");

-- DropTable
DROP TABLE "AlgoFile";

-- CreateTable
CREATE TABLE "algo_file" (
    "s3Key" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,
    "uid" TEXT NOT NULL,

    CONSTRAINT "algo_file_pkey" PRIMARY KEY ("s3Key")
);

-- AddForeignKey
ALTER TABLE "algos" ADD CONSTRAINT "algos_algoFileS3Key_fkey" FOREIGN KEY ("algoFileS3Key") REFERENCES "algo_file"("s3Key") ON DELETE RESTRICT ON UPDATE CASCADE;

-- AddForeignKey
ALTER TABLE "algo_file" ADD CONSTRAINT "algo_file_uid_fkey" FOREIGN KEY ("uid") REFERENCES "users"("uid") ON DELETE RESTRICT ON UPDATE CASCADE;
