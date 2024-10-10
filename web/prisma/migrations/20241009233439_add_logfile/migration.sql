/*
  Warnings:

  - A unique constraint covering the columns `[logFileS3Key]` on the table `algos` will be added. If there are existing duplicate values, this will fail.

*/
-- AlterTable
ALTER TABLE "algos" ADD COLUMN     "logFileS3Key" TEXT;

-- CreateTable
CREATE TABLE "log_files" (
    "s3Key" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "updatedAt" TIMESTAMP(3) NOT NULL,

    CONSTRAINT "log_files_pkey" PRIMARY KEY ("s3Key")
);

-- CreateIndex
CREATE UNIQUE INDEX "algos_logFileS3Key_key" ON "algos"("logFileS3Key");

-- AddForeignKey
ALTER TABLE "algos" ADD CONSTRAINT "algos_logFileS3Key_fkey" FOREIGN KEY ("logFileS3Key") REFERENCES "log_files"("s3Key") ON DELETE SET NULL ON UPDATE CASCADE;
