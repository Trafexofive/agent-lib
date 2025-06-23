
import React from 'react';
import { NotificationItem, ViewType, DashboardSettings } from '../types';

interface SecurityCenterFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
  dashboardSettings: DashboardSettings;
}

const SecurityCenterFeature: React.FC<SecurityCenterFeatureProps> = ({ addNotification, dashboardSettings }) => {
  return (
    <div className="p-6 space-y-6">
      <header className="text-center">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-red-500 to-rose-600">
          Security Center
        </h1>
        <p className="text-slate-400 mt-1 text-md">
          Fortify your digital domain.
        </p>
      </header>
      <div className="bg-slate-800 p-8 rounded-xl shadow-xl text-center">
        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-16 h-16 mx-auto text-slate-500 mb-4">
          <path strokeLinecap="round" strokeLinejoin="round" d="M9 12.75L11.25 15 15 9.75m-3-7.036A11.959 11.959 0 013.598 6 11.99 11.99 0 003 9.749c0 5.592 3.824 10.29 9 11.623 5.176-1.332 9-6.03 9-11.622A11.99 11.99 0 0020.402 6a11.959 11.959 0 01-1.598-2.286" />
        </svg>
        <h2 className="text-2xl font-semibold text-slate-300 mb-2">Fortification Underway</h2>
        <p className="text-slate-400">
          Dashboard for security alerts, password management, and 2FA checks coming soon. Secure the Covenant.
        </p>
      </div>
    </div>
  );
};

export default SecurityCenterFeature;
