/*
  Warnings:

  - A unique constraint covering the columns `[uid]` on the table `users` will be added. If there are existing duplicate values, this will fail.

*/
-- DropIndex
DROP INDEX "algos_name_key";

-- CreateIndex
CREATE UNIQUE INDEX "users_uid_key" ON "users"("uid");
