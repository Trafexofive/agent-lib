

import React from 'react';
import { AlertMessage } from '../types'; // AlertMessage is suitable for toasts

interface AlertProps extends AlertMessage {
  onDismiss: (id: string) => void;
}

const Alert: React.FC<AlertProps> = ({ id, type, message, onDismiss }) => {
  let bgColorVar = 'var(--theme-info)';
  let textColor = 'text-white'; // High contrast against typical theme colors
  let iconPath = ''; 

  switch (type) {
    case 'success':
      bgColorVar = 'var(--theme-success)';
      iconPath = "M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"; 
      break;
    case 'error':
      bgColorVar = 'var(--theme-error)';
      iconPath = "M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z"; 
      break;
    case 'info':
    default:
      bgColorVar = 'var(--theme-info)';
      iconPath = "M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"; 
      break;
  }

  return (
    <div 
        className={`w-full max-w-md p-4 rounded-lg shadow-xl ${textColor} flex items-center space-x-3 z-50`}
        style={{ backgroundColor: bgColorVar }}
        role="alert"
        aria-live="assertive"
    >
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-6 h-6 flex-shrink-0">
        <path strokeLinecap="round" strokeLinejoin="round" d={iconPath} />
      </svg>
      <span className="flex-grow">{message}</span>
      <button 
        onClick={() => onDismiss(id)} 
        className="ml-auto -mx-1.5 -my-1.5 p-1.5 rounded-lg hover:bg-white/20 focus:outline-none focus:ring-2 focus:ring-white/50 flex-shrink-0"
        aria-label="Dismiss notification"
      >
        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-5 h-5">
          <path strokeLinecap="round" strokeLinejoin="round" d="M6 18L18 6M6 6l12 12" />
        </svg>
      </button>
    </div>
  );
};

interface AlertsContainerProps {
  alerts: AlertMessage[]; // Now specifically for toasts
  removeAlert: (id: string) => void; // Renamed to removeAlert for clarity with toasts
}

const AlertsContainer: React.FC<AlertsContainerProps> = ({ alerts, removeAlert }) => {
  return (
    <div className="fixed bottom-4 right-4 w-full max-w-md space-y-3 z-[100]" aria-live="polite">
      {alerts.map(alert => (
        <Alert key={alert.id} {...alert} onDismiss={removeAlert} />
      ))}
    </div>
  );
};

export default AlertsContainer;