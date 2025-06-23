

import React from 'react';
import { NotificationItem } from '../types';

// --- Internal Icon Components ---
const BellAlertIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.857 17.082a23.848 23.848 0 005.454-1.31A8.967 8.967 0 0118 9.75v-.7V9A6 6 0 006 9v.75a8.967 8.967 0 01-2.312 6.022c1.733.64 3.56 1.017 5.454 1.31m5.714 0a24.255 24.255 0 01-5.714 0m5.714 0a3 3 0 11-5.714 0M12 18.75a9.75 9.75 0 007.242-3.346" /></svg>;
const InformationCircleIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M11.25 11.25l.041-.02a.75.75 0 011.063.852l-.708 2.836a.75.75 0 001.063.853l.041-.021M21 12a9 9 0 11-18 0 9 9 0 0118 0zm-9-3.75h.008v.008H12V8.25z" /></svg>;
const CheckCircleIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9 12.75L11.25 15 15 9.75M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const XCircleIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9.75 9.75l4.5 4.5m0-4.5l-4.5 4.5M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const EyeIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M2.036 12.322a1.012 1.012 0 010-.639C3.423 7.51 7.36 4.5 12 4.5c4.638 0 8.573 3.007 9.963 7.178.07.207.07.431 0 .639C20.577 16.49 16.64 19.5 12 19.5c-4.638 0-8.573-3.007-9.963-7.178z" /><path strokeLinecap="round" strokeLinejoin="round" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" /></svg>;
const EyeSlashIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.98 8.223A10.477 10.477 0 001.934 12C3.226 16.338 7.244 19.5 12 19.5c.993 0 1.953-.138 2.863-.395M6.228 6.228A10.45 10.45 0 0112 4.5c4.756 0 8.773 3.162 10.065 7.498a10.523 10.523 0 01-4.293 5.774M6.228 6.228L3 3m3.228 3.228l3.65 3.65m7.894 7.894L21 21m-3.228-3.228l-3.65-3.65m0 0a3 3 0 10-4.243-4.243m4.242 4.242L9.88 9.88" /></svg>;
const TrashIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12.56 0c1.153 0 2.24.032 3.223.094M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const XMarkIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6 18L18 6M6 6l12 12" /></svg>;

const getNotificationIcon = (type: NotificationItem['type']) => {
  switch (type) {
    case 'success': return <CheckCircleIcon className="w-5 h-5 text-[var(--theme-success)] flex-shrink-0" />;
    case 'error': return <XCircleIcon className="w-5 h-5 text-[var(--theme-error)] flex-shrink-0" />;
    case 'reminder': return <BellAlertIcon className="w-5 h-5 text-[var(--theme-accent-secondary)] flex-shrink-0" />; // Or a specific reminder color
    case 'info':
    default: return <InformationCircleIcon className="w-5 h-5 text-[var(--theme-info)] flex-shrink-0" />;
  }
};

const formatRelativeTime = (timestamp: string): string => {
    const now = new Date();
    const then = new Date(timestamp);
    const diffSeconds = Math.round((now.getTime() - then.getTime()) / 1000);

    if (diffSeconds < 5) return "just now";
    if (diffSeconds < 60) return `${diffSeconds}s ago`;
    
    const diffMinutes = Math.round(diffSeconds / 60);
    if (diffMinutes < 60) return `${diffMinutes}m ago`;

    const diffHours = Math.round(diffMinutes / 60);
    if (diffHours < 24) return `${diffHours}h ago`;

    const diffDays = Math.round(diffHours / 24);
    if (diffDays === 1) return "yesterday";
    if (diffDays < 7) return `${diffDays}d ago`;
    
    return then.toLocaleDateString(undefined, { month: 'short', day: 'numeric' });
};

interface NotificationsPanelProps {
  notifications: NotificationItem[];
  showPanel: boolean;
  onClose: () => void;
  onNotificationClick: (notification: NotificationItem) => void;
  onMarkReadToggle: (notificationId: string, currentReadStatus: boolean) => void;
  onDismissNotification: (notificationId: string) => void;
  onMarkAllRead: () => void;
  onDismissAll: () => void;
}

const NotificationsPanel: React.FC<NotificationsPanelProps> = ({
  notifications,
  showPanel,
  onClose,
  onNotificationClick,
  onMarkReadToggle,
  onDismissNotification,
  onMarkAllRead,
  onDismissAll,
}) => {

  return (
    <>
      {/* Backdrop */}
      {showPanel && (
          <div 
            className="fixed inset-0 bg-slate-900/70 backdrop-blur-sm z-30 transition-opacity duration-300 ease-in-out"
            onClick={onClose}
            aria-hidden="true" 
          ></div>
      )}

      {/* Panel */}
      <div
        className={`fixed top-0 right-0 h-full w-full max-w-md bg-[var(--theme-modal-bg)] shadow-2xl transform transition-transform duration-300 ease-in-out z-40 flex flex-col border-l border-[var(--theme-border-primary)]
                  ${showPanel ? 'translate-x-0' : 'translate-x-full'}`}
        role="dialog"
        aria-modal="true"
        aria-labelledby="notifications-panel-title"
      >
        <header className="flex items-center justify-between p-4 border-b border-[var(--theme-border-primary)]">
          <h2 id="notifications-panel-title" className="text-lg font-semibold text-[var(--theme-accent-primary)]">Notifications</h2>
          <button onClick={onClose} className="p-1.5 text-[var(--theme-text-secondary)] hover:text-[var(--theme-accent-primary)] rounded-full hover:bg-[var(--theme-bg-accent)]/50" aria-label="Close notifications panel">
            <XMarkIcon className="w-6 h-6" />
          </button>
        </header>

        {notifications.length === 0 ? (
          <div className="flex-grow flex flex-col items-center justify-center text-center p-6 text-[var(--theme-text-secondary)]">
            <BellAlertIcon className="w-16 h-16 text-slate-500 mb-4" />
            <p className="text-lg">No notifications yet.</p>
            <p className="text-sm text-slate-500">Stay tuned for updates!</p>
          </div>
        ) : (
          <>
            <div className="p-3 border-b border-[var(--theme-border-primary)] flex justify-end space-x-2">
              <button onClick={onMarkAllRead} className="text-xs px-2.5 py-1.5 bg-[var(--theme-bg-accent)] hover:opacity-80 text-[var(--theme-text-secondary)] rounded-md">Mark All Read</button>
              <button onClick={onDismissAll} className="text-xs px-2.5 py-1.5 bg-red-700/70 hover:bg-red-600/70 text-red-100 rounded-md">Dismiss All</button>
            </div>
            <ul className="flex-grow overflow-y-auto divide-y divide-[var(--theme-border-primary)]/50 p-2">
              {notifications.map(notification => (
                <li
                  key={notification.id}
                  className={`p-3 group hover:bg-[var(--theme-bg-accent)]/30 transition-colors ${notification.read ? 'opacity-70' : ''}`}
                >
                  <div className="flex items-start space-x-3">
                    {getNotificationIcon(notification.type)}
                    <div className="flex-grow min-w-0">
                      <p className={`text-sm font-medium break-words ${notification.read ? 'text-[var(--theme-text-secondary)]' : 'text-[var(--theme-text-primary)]'}`}>
                        {notification.message}
                      </p>
                      <p className="text-xs text-slate-500 mt-0.5">
                        {formatRelativeTime(notification.timestamp)}
                        {notification.sourceView && <span className="ml-1 text-slate-600">&bull; {notification.sourceView.replace(/_/g, ' ').toLowerCase()}</span>}
                      </p>
                    </div>
                    <div className="flex-shrink-0 flex flex-col sm:flex-row items-center sm:space-x-1 opacity-0 group-hover:opacity-100 focus-within:opacity-100 transition-opacity">
                        <button onClick={() => onMarkReadToggle(notification.id, notification.read)} title={notification.read ? "Mark as unread" : "Mark as read"} className="p-1 text-[var(--theme-text-secondary)] hover:text-[var(--theme-accent-primary)] rounded-full">
                            {notification.read ? <EyeSlashIcon className="w-4 h-4" /> : <EyeIcon className="w-4 h-4" />}
                        </button>
                        <button onClick={() => onDismissNotification(notification.id)} title="Dismiss" className="p-1 text-[var(--theme-text-secondary)] hover:text-red-500 rounded-full">
                            <TrashIcon className="w-4 h-4" />
                        </button>
                    </div>
                  </div>
                  {(notification.sourceView || notification.sourceItemId) && (
                    <button onClick={() => onNotificationClick(notification)} className="mt-1.5 text-xs text-sky-400 hover:text-sky-300 hover:underline">
                      View Details
                    </button>
                  )}
                </li>
              ))}
            </ul>
          </>
        )}
      </div>
    </>
  );
};

export default NotificationsPanel;