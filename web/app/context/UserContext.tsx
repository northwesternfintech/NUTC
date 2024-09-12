"use client";
import { createContext, useState, useEffect, useContext } from "react";
import { ReactNode } from "react";
import { User } from "@prisma/client";

// If user==null and loading==false, user does not exist
interface DatabaseUserType {
  user: User | null;
  setUserDatabase: (user: User) => Promise<any>;
  loading: boolean;
  error: string | null;
}

const setUserDatabase = async (user: User): Promise<any> => {
  const response = await fetch("/api/db/setUser", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(user),
  });

  return await response.json();
};

const loadingDatabaseEntry: DatabaseUserType = {
  user: null,
  setUserDatabase: setUserDatabase,
  loading: true,
  error: null,
};
const DatabaseUserContext =
  createContext<DatabaseUserType>(loadingDatabaseEntry);

export const UserStateContext = ({ children }: { children: ReactNode }) => {
  const [user, setUser]: any = useState(null);
  const [error, setError]: any = useState(null);
  const [loading, setLoading]: any = useState(true);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await fetch("/api/db/getUser");
        if (!response.ok) {
          throw new Error(`Error: ${JSON.stringify(response)}`);
        }

        const json_res: User = await response.json();
        setUser(json_res);
      } catch (error) {
        setError(error);
      } finally {
        setLoading(false);
      }
    };

    fetchData();
  }, []);

  return (
    <DatabaseUserContext.Provider
      value={{ user, setUserDatabase, loading, error }}>
      {children}
    </DatabaseUserContext.Provider>
  );
};

export const useUserContext = () => useContext(DatabaseUserContext);
