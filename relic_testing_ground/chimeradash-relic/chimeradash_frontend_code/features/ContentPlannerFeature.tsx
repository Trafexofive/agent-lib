
import React from 'react';
import { NotificationItem, ViewType, DashboardSettings } from '../types';

interface ContentPlannerFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
  dashboardSettings: DashboardSettings;
}

const ContentPlannerFeature: React.FC<ContentPlannerFeatureProps> = ({ addNotification, dashboardSettings }) => {
  return (
    <div className="p-6 space-y-6">
      <header className="text-center">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-amber-500">
          Content Planner
        </h1>
        <p className="text-slate-400 mt-1 text-md">
          Strategize and schedule your creative output.
        </p>
      </header>
      <div className="bg-slate-800 p-8 rounded-xl shadow-xl text-center">
        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-16 h-16 mx-auto text-slate-500 mb-4">
          <path strokeLinecap="round" strokeLinejoin="round" d="M9 4.5v15m6-15v15m-10.875 0h15.75c.621 0 1.125-.504 1.125-1.125V5.625c0-.621-.504-1.125-1.125-1.125H4.125C3.504 4.5 3 5.004 3 5.625v12.75c0 .621.504 1.125 1.125 1.125z" />
        </svg>
        <h2 className="text-2xl font-semibold text-slate-300 mb-2">In Progress</h2>
        <p className="text-slate-400">
          Kanban-style content planning and scheduling tool under construction. Plan your narrative.
        </p>
      </div>
    </div>
  );
};

export default ContentPlannerFeature;
