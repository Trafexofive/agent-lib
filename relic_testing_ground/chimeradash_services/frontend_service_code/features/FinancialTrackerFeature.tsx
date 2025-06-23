
import React from 'react';
import { NotificationItem, ViewType, DashboardSettings } from '../types';

interface FinancialTrackerFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
  dashboardSettings: DashboardSettings;
}

const FinancialTrackerFeature: React.FC<FinancialTrackerFeatureProps> = ({ addNotification, dashboardSettings }) => {
  return (
    <div className="p-6 space-y-6">
      <header className="text-center">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-green-400 to-emerald-500">
          Financial Tracker
        </h1>
        <p className="text-slate-400 mt-1 text-md">
          Manage your financial landscape with precision.
        </p>
      </header>
      <div className="bg-slate-800 p-8 rounded-xl shadow-xl text-center">
        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-16 h-16 mx-auto text-slate-500 mb-4">
          <path strokeLinecap="round" strokeLinejoin="round" d="M2.25 18.75a60.07 60.07 0 0115.797 2.101c.727.198 1.453-.342 1.453-1.096V18.75M3.75 4.5v.75A.75.75 0 013 6A2.25 2.25 0 00.75 8.25v10.5a2.25 2.25 0 002.25 2.25h10.5A2.25 2.25 0 0015.75 18.75V16.5m-13.5-9L12 3m0 0l4.5 4.5M12 3v13.5" />
        </svg>
        <h2 className="text-2xl font-semibold text-slate-300 mb-2">Under Development</h2>
        <p className="text-slate-400">
          Personal finance tracking and insights module in progress. Master your C.R.E.A.M.
        </p>
      </div>
    </div>
  );
};

export default FinancialTrackerFeature;
