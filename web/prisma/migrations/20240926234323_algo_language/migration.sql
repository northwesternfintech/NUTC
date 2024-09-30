/*
  Warnings:

  - Added the required column `language` to the `algos` table without a default value. This is not possible if the table is not empty.

*/
-- AlterTable
ALTER TABLE "algos" ADD COLUMN     "language" TEXT NOT NULL;
