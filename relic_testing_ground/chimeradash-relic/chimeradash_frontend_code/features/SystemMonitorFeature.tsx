
import React from 'react';
import { NotificationItem, ViewType, DashboardSettings } from '../types';

interface SystemMonitorFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
  dashboardSettings: DashboardSettings;
}

const SystemMonitorFeature: React.FC<SystemMonitorFeatureProps> = ({ addNotification, dashboardSettings }) => {
  return (
    <div className="p-6 space-y-6">
      <header className="text-center">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-cyan-400 to-blue-500">
          System Monitor
        </h1>
        <p className="text-slate-400 mt-1 text-md">
          Oversee operational metrics and service statuses.
        </p>
      </header>
      <div className="bg-slate-800 p-8 rounded-xl shadow-xl text-center">
        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-16 h-16 mx-auto text-slate-500 mb-4">
          <path strokeLinecap="round" strokeLinejoin="round" d="M3.75 12h16.5M3.75 6.75h16.5M3.75 17.25h16.5M8.25 3v18M15.75 3v18" />
        </svg>
        <h2 className="text-2xl font-semibold text-slate-300 mb-2">Coming Soon</h2>
        <p className="text-slate-400">
          Real-time system metrics and service status dashboard under development. Monitor your domain.
        </p>
      </div>
    </div>
  );
};

export default SystemMonitorFeature;
