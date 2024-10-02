/*
  Warnings:

  - A unique constraint covering the columns `[algoFileS3Key]` on the table `algos` will be added. If there are existing duplicate values, this will fail.

*/
-- CreateIndex
CREATE UNIQUE INDEX "algos_algoFileS3Key_key" ON "algos"("algoFileS3Key");
