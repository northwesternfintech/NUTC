'use client'

import React, { useEffect, useRef, useState, useCallback, useLayoutEffect } from 'react';
import { useVirtualizer } from '@tanstack/react-virtual';

const MAX_LOGS = 20_000;

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
  const parentRef = useRef<HTMLDivElement>(null);

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
      setLogs(prev => {
        if (prev.length >= MAX_LOGS) {
          ws.close();
          return prev;
        }
        return [...prev, event.data]
      });
    };

    // Cleanup function to close the WebSocket connection when the component unmounts
    return () => {
      ws.close();
    };
  }, [id]); // Re-run the effect if the `id` prop changes

  const rowVirtualizer = useVirtualizer({
    count: logs.length,
    getScrollElement: () => parentRef.current,
    // Estimate size of each row for initial render
    estimateSize: () => 24, 
    // Add overscan for smoother scrolling
    overscan: 10, 
  });

  // This effect handles auto-scrolling to the bottom for new logs
  useLayoutEffect(() => {
    if (!isPaused && logs.length > 0) {
      rowVirtualizer.scrollToIndex(logs.length - 1, { align: 'end' });
    }
  }, [logs.length, isPaused, rowVirtualizer]);

  const handleScroll = useCallback(() => {
    if (parentRef.current) {
      const { scrollTop, scrollHeight, clientHeight } = parentRef.current;
      // Use a small buffer to reliably detect the bottom
      const isAtBottom = scrollTop + clientHeight >= scrollHeight - 10;
      setIsPaused(!isAtBottom);
    }
  }, []);

  const isAtMax = logs.length >= MAX_LOGS;
  const virtualItems = rowVirtualizer.getVirtualItems();

  return (
    <div className="w-full max-w-5xl mx-auto mt-8">
      <div className="flex justify-between items-center mb-3 px-1">
        <h2 className="text-xl font-semibold text-white">Live Sandbox Logs</h2>
        <div className={`text-sm font-medium px-3 py-1 rounded-full transition-colors duration-300 ${isAtMax ? "bg-red-500 text-white" : isPaused ? 'bg-amber-400 text-amber-900' : 'bg-gray-700 text-gray-300'}`}>
          {isAtMax  ? "Live logs paused due to high volume, download log file after run" : isPaused ? 'Scroll to bottom to resume' : 'Auto-scrolling'}
        </div>
      </div>
      <div ref={parentRef} onScroll={handleScroll} tabIndex={0} className="h-96 bg-black text-white font-mono text-sm rounded-lg border border-gray-700 shadow-2xl overflow-y-auto focus:outline-none focus:ring-2 focus:ring-indigo-500">
        <div style={{height: `${rowVirtualizer.getTotalSize()}px`, width: "100%", position: "relative"}}>
          {/* Map over the virtual items provided by the hook */}
          {virtualItems.map((virtualItem) => (
            <div
              key={virtualItem.key}
              style={{
                position: 'absolute',
                top: 0,
                left: 0,
                width: '100%',
                height: `${virtualItem.size}px`,
                transform: `translateY(${virtualItem.start}px)`,
              }}
              className="flex items-center px-4"
            >
              <span className="text-gray-500 mr-4 select-none">{virtualItem.index + 1}</span>
              <span className="flex-1 whitespace-pre-wrap text-white">{logs[virtualItem.index]}</span>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
