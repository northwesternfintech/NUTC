-- AlterTable
ALTER TABLE "users" ADD COLUMN     "teamName" TEXT;

-- CreateTable
CREATE TABLE "teams" (
    "name" TEXT NOT NULL,

    CONSTRAINT "teams_pkey" PRIMARY KEY ("name")
);

-- CreateIndex
CREATE UNIQUE INDEX "teams_name_key" ON "teams"("name");

-- AddForeignKey
ALTER TABLE "users" ADD CONSTRAINT "users_teamName_fkey" FOREIGN KEY ("teamName") REFERENCES "teams"("name") ON DELETE SET NULL ON UPDATE CASCADE;
