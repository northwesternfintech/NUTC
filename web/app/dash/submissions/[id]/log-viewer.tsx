'use client'

import React, { useEffect, useRef, useState, useCallback } from 'react';

/**
 * Props for the LogViewer component.
 * @property {string} id - The unique identifier used to establish the WebSocket connection.
 */
interface LogViewerProps {
  id: string;
}

/**
 * A client component that connects to a WebSocket and displays a stream of log messages.
 * It features auto-scrolling that intelligently pauses when the user scrolls up.
 */
export default function LogViewer({ id }: LogViewerProps) {
  // State to hold the array of log messages
  const [logs, setLogs] = useState<string[]>([]);
  // State to track if the user has manually scrolled up, pausing auto-scroll
  const [isPaused, setIsPaused] = useState(false);
  // Ref to the scrollable log container div
  const logContainerRef = useRef<HTMLDivElement>(null);

  // Effect to manage the WebSocket lifecycle
  useEffect(() => {
    // Ensure this code only runs on the client
    if (typeof window === 'undefined') {
      return;
    }

    const wsUrl = `wss://${process.env.NEXT_PUBLIC_LOG_WS_ENDPOINT}/ws?algo_id=${id}`;
    const ws = new WebSocket(wsUrl);

    // When the connection opens, send an "init" message
    ws.onopen = () => {
      console.log("WebSocket connection established. Sending 'init' message.");
      ws.send("init");
    };

    ws.onmessage = (event) => {
      // Append new log message from the server
      console.log("got log:", event.data);
      setLogs(prev => [...prev, event.data]);
    };

    // Cleanup function to close the WebSocket connection when the component unmounts
    return () => {
      ws.close();
    };
  }, [id]); // Re-run the effect if the `id` prop changes

  // Effect to handle the auto-scrolling behavior
  useEffect(() => {
    if (logContainerRef.current && !isPaused) {
      const element = logContainerRef.current;
      element.scrollTop = element.scrollHeight;
    }
  }, [logs, isPaused]); // Re-run whenever logs update or pause state changes

  // Callback to handle user scrolling
  const handleScroll = useCallback(() => {
    if (logContainerRef.current) {
      const { scrollTop, scrollHeight, clientHeight } = logContainerRef.current;
      // A small buffer is used to account for fractional pixel values
      const isAtBottom = scrollTop + clientHeight >= scrollHeight - 5;

      // If user scrolls up, pause auto-scrolling. If they scroll back to bottom, resume.
      setIsPaused(!isAtBottom);
    }
  }, []); // Empty dependency array as it doesn't depend on any props or state

  return (
    <div className="w-full max-w-5xl mx-auto mt-8">
      <div className="flex justify-between items-center mb-3 px-1">
        <h2 className="text-xl font-semibold text-white">Live Sandbox Logs</h2>
        <div className={`text-sm font-medium px-3 py-1 rounded-full transition-colors duration-300 ${isPaused ? 'bg-amber-400 text-amber-900' : 'bg-gray-700 text-gray-300'}`}>
          {isPaused ? 'Scroll to bottom to resume' : 'Auto-scrolling'}
        </div>
      </div>
      <div
        ref={logContainerRef}
        onScroll={handleScroll}
        className="h-96 bg-black text-white font-mono text-sm p-4 rounded-lg overflow-y-auto border border-gray-700 shadow-2xl focus:outline-none focus:ring-2 focus:ring-indigo-500"
        tabIndex={0} // Make it focusable
      >
        {logs.map((log, index) => (
          <div key={index} className="flex">
            <span className="text-gray-500 mr-4 select-none">{index + 1}</span>
            <span className="flex-1 whitespace-pre-wrap">{log}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
