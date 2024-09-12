-- CreateEnum
CREATE TYPE "ParticipantState" AS ENUM ('PRE_REGISTRATION', 'WAITING', 'REJECTED', 'ACCEPTED');

-- AlterTable
ALTER TABLE "users" ADD COLUMN     "participantState" "ParticipantState" NOT NULL DEFAULT 'PRE_REGISTRATION';
