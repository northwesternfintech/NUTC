'use client'
import { UsersIcon } from '@heroicons/react/20/solid'
import { useState, useEffect, useRef } from "react"
import Swal from 'sweetalert2'

export default function Groups() {
  const teamNameRef = useRef<HTMLInputElement>(null);

  const [team, setTeam] = useState("");
  const [teamMembers, setTeamMembers] = useState([]);

  useEffect(() => {
    const getTeam = async () => {
      const currentTeam = await fetch("/api/protected/db/user/getTeam");
      const { teamName, teamMemberNames } = await currentTeam.json();
      setTeam(teamName);
      setTeamMembers(teamMemberNames);
    };
    getTeam();
  }, []);

  async function handleSetTeamHandler() {
    if (teamNameRef?.current?.value == null) { return }
    const newTeamName = teamNameRef.current.value;
    if (newTeamName == team) return;
    const response = await fetch("/api/protected/db/user/setTeam", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ newTeam: newTeamName }),
    });

    if (response.ok) {
      await Swal.fire({ icon: 'success', title: 'Team Updated', text: `Team is now ${newTeamName}` });
    } else {
      await Swal.fire({ icon: 'error', title: 'Setting team failed', text: 'Please contact nuft@u.northwestern.edu' });
    }

    setTeam(teamNameRef.current.value);
  }

  return (
    <div className="relative min-h-screen flex items-center justify-center">
      <div className="flex items-center justify-center p-4 text-center sm:p-0">
        <div className="relative transform overflow-hidden rounded-lg bg-white px-4 pb-4 pt-5 text-left shadow-xl sm:my-8 sm:w-full sm:max-w-lg sm:p-6">
          <div>
            <div className="mt-1 text-center">
              <h1 className="text-xl font-semibold leading-6 text-gray-900">
                Team Registration
              </h1>
              <div className="mt-2">
                <p className="text-base text-gray-500">
                  To form a team, create a team name and have your teammates enter the same on their computers.
                </p>
              </div>
            </div>
          </div>
          <div>
            <label htmlFor="email" className="block text-sm font-medium leading-6 text-gray-900 mt-4">
              Team Name
            </label>
            <div className="mt-2 flex rounded-md shadow-sm">
              <div className="relative flex flex-grow items-stretch focus-within:z-10">
                <div className="pointer-events-none absolute inset-y-0 left-0 flex items-center pl-3">
                  <UsersIcon aria-hidden="true" className="h-5 w-5 text-gray-400" />
                </div>
                <input
                  id="team"
                  name="team"
                  type="team"
                  defaultValue={team}
                  ref={teamNameRef}
                  className="block w-full rounded-none rounded-l-md border-0 py-1.5 pl-10 text-gray-900 ring-1 ring-inset ring-gray-300 focus:ring-2 focus:ring-inset focus:ring-indigo-600 sm:text-sm sm:leading-6"
                />
              </div>
            </div>
          </div>
          <div className="mt-5 sm:mt-6">
            <button
              type="button"
              className="inline-flex w-full justify-center rounded-md bg-indigo-600 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
              onClick={handleSetTeamHandler}
            >
              Save Team
            </button>
          </div>
          {teamMembers.length > 0 && <div><div className="relative mt-5">
            <div aria-hidden="true" className="absolute inset-0 flex items-center">
              <div className="w-full border-t border-gray-300" />
            </div>
            <div className="relative flex justify-center">
              <span className="bg-white px-2 text-sm text-gray-500">Team Members</span>
            </div>
          </div>
            <dl className="mt-5 grid gap-5 grid-cols-3 text-center">
              {teamMembers.map((item) => (
                <div key={item} className="overflow-hidden rounded-lg bg-white px-4 py-5 shadow sm:p-6 col-span-1">
                  <dd className="mt-1 text-lg font-semibold tracking-tight text-gray-900">{item}</dd>
                </div>
              ))}
            </dl></div>}
        </div>


      </div>
    </div >

  );
}
