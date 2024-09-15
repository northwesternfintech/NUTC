/*
  Warnings:

  - Added the required column `year` to the `profiles` table without a default value. This is not possible if the table is not empty.

*/
-- AlterTable
ALTER TABLE "profiles" ADD COLUMN     "year" INTEGER NOT NULL;
