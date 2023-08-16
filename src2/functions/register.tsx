import { UserInfoType } from "@/app/login/auth/context";
import { ref, update } from "firebase/database";

export default function writeNewUser(database: any, user: UserInfoType) {
  update(ref(database, "users/" + user.uid), user);
}
