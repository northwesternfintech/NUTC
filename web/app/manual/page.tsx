"use client";
import { useState, useEffect } from "react";

export default function Manual() {
  const [code, setCode] = useState("");
  const [goals, setGoals] = useState<string[]>([]);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    setMounted(true);
  }, []);

  const handleSubmit = async () => {
    setError("");
    setGoals([]);
    setLoading(true);

    try {
      const response = await fetch("/dict.csv");
      const csvData = await response.text();
      const lines = csvData.trim().split("\n");
      
      const upperCode = code.toUpperCase();
      let found = false;
      
      for (let i = 1; i < lines.length; i++) {
        const row = lines[i].split(",");
        const rowCode = row[0]?.trim().toUpperCase();
        
        if (rowCode === upperCode) {
          setGoals([
            row[1]?.trim() || "",
            row[2]?.trim() || "",
            row[3]?.trim() || ""
          ]);
          found = true;
          break;
        }
      }
      
      if (!found) {
        setError("Code not found. Please try again.");
      }
    } catch (err) {
      setError("Error loading data. Please ensure dict.csv is in the public folder.");
    } finally {
      setLoading(false);
    }
  };

  const handleKeyPress = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === "Enter" && code.length === 6) {
      handleSubmit();
    }
  };

  if (!mounted) {
    return null;
  }

  return (
    <div className="min-h-screen relative isolate bg-gray-900">
      <div className="absolute top-6 left-6 z-10">
        <img src="/images/logo.png" alt="Club Logo" className="h-12 w-auto" />
      </div>
      <div className="mx-auto max-w-2xl px-6 py-24 sm:py-32">
        <div className="absolute inset-y-0 left-0 -z-10 w-full overflow-hidden ring-1 ring-white/5">
          <svg
            className="absolute inset-0 h-full w-full stroke-gray-700 [mask-image:radial-gradient(100%_100%_at_top_right,white,transparent)]"
            aria-hidden="true">
            <defs>
              <pattern
                id="manual-pattern"
                width={200}
                height={200}
                x="50%"
                y={-1}
                patternUnits="userSpaceOnUse">
                <path d="M130 200V.5M.5 .5H200" fill="none" />
              </pattern>
            </defs>
            <rect
              width="100%"
              height="100%"
              strokeWidth={0}
              fill="url(#manual-pattern)"
            />
          </svg>
          <div
            className="absolute left-1/2 top-0 -translate-x-1/2 transform-gpu blur-3xl"
            aria-hidden="true">
            <div
              className="aspect-[1155/678] w-[72.1875rem] bg-gradient-to-br from-[#80caff] to-[#4f46e5] opacity-20"
              style={{
                clipPath:
                  "polygon(74.1% 56.1%, 100% 38.6%, 97.5% 73.3%, 85.5% 100%, 80.7% 98.2%, 72.5% 67.7%, 60.2% 37.8%, 52.4% 32.2%, 47.5% 41.9%, 45.2% 65.8%, 27.5% 23.5%, 0.1% 35.4%, 17.9% 0.1%, 27.6% 23.5%, 76.1% 2.6%, 74.1% 56.1%)",
              }}
            />
          </div>
        </div>

        <div className="text-center mb-12">
          <h2 className="text-3xl font-bold tracking-tight text-white">
            Manual Trading Case
          </h2>
          <p className="mt-4 text-lg leading-8 text-gray-300">
            Enter your 6-letter code to view your trading goals
          </p>
        </div>

        <div className="mb-8">
          <div className="flex flex-col items-center gap-4">
            <input
              type="text"
              value={code}
              onChange={(e: React.ChangeEvent<HTMLInputElement>) => setCode(e.target.value.toUpperCase())}
              onKeyPress={handleKeyPress}
              maxLength={6}
              placeholder="XXXXXX"
              className="w-64 text-center text-2xl tracking-widest rounded-md border-0 bg-white/5 px-3.5 py-3 text-white shadow-sm ring-1 ring-inset ring-white/10 focus:ring-2 focus:ring-inset focus:ring-indigo-500 uppercase"
            />
            <button
              onClick={handleSubmit}
              disabled={loading || code.length !== 6}
              className="rounded-md bg-indigo-500 px-6 py-2.5 text-sm font-semibold text-white shadow-sm hover:bg-indigo-400 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-500 disabled:opacity-50 disabled:cursor-not-allowed">
              {loading ? "Loading..." : "Submit Code"}
            </button>
          </div>
        </div>

        {error && (
          <div className="rounded-md bg-red-500/10 p-4 ring-1 ring-red-500/20">
            <p className="text-sm text-red-400 text-center">{error}</p>
          </div>
        )}

        {goals.length > 0 && (
          <div className="mt-8">
            <h3 className="text-xl font-semibold text-white mb-6 text-center">
              Your Trading Goals
            </h3>
            <div className="space-y-4">
              {goals.map((goal, index) => (
                <div
                  key={index}
                  className="rounded-md bg-white/5 p-6 ring-1 ring-white/10">
                  <div className="flex items-start gap-4">
                    <span className="flex-shrink-0 flex items-center justify-center w-8 h-8 rounded-full bg-indigo-500/20 text-indigo-400 font-semibold">
                      {index + 1}
                    </span>
                    <p className="text-gray-300 leading-7 flex-1">{goal}</p>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}