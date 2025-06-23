

import React from 'react';

interface TopBarProps {
  activeViewLabel: string;
  unreadNotificationsCount: number;
  onToggleNotificationsPanel: () => void;
  onToggleCommandPalette: () => void;
  isSidebarOpen: boolean; // Added prop
}

// --- Internal Icon Components ---
const BellIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M14.857 17.082a23.848 23.848 0 005.454-1.31A8.967 8.967 0 0118 9.75v-.7V9A6 6 0 006 9v.75a8.967 8.967 0 01-2.312 6.022c1.733.64 3.56 1.017 5.454 1.31m5.714 0a24.255 24.255 0 01-5.714 0m5.714 0a3 3 0 11-5.714 0" />
  </svg>
);

const MagnifyingGlassIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M21 21l-5.197-5.197m0 0A7.5 7.5 0 105.196 5.196a7.5 7.5 0 0010.607 10.607z" />
  </svg>
);


const TopBar: React.FC<TopBarProps> = ({ 
  activeViewLabel, 
  unreadNotificationsCount, 
  onToggleNotificationsPanel,
  onToggleCommandPalette,
  isSidebarOpen // Destructure new prop
}) => {
  return (
    <header 
      className="bg-[var(--theme-topbar-bg)]/90 backdrop-blur-md shadow-lg flex items-center justify-between px-4 sm:px-6 border-b border-[var(--theme-border-primary)]"
      style={{ height: '4rem' }} // Fixed height for the top bar
    >
      <h1 className="text-xl font-semibold text-[var(--theme-accent-primary)] truncate" title={activeViewLabel}>
        {activeViewLabel.toUpperCase()}
      </h1>
      <div className="flex items-center space-x-2 sm:space-x-3">
        <button
            onClick={onToggleCommandPalette}
            className="p-2 text-[var(--theme-text-secondary)] hover:text-[var(--theme-accent-primary)] transition-colors rounded-full hover:bg-[var(--theme-bg-accent)]/50"
            aria-label="Open command palette (Ctrl+K)"
            title="Command Palette (Ctrl+K)"
        >
            <MagnifyingGlassIcon className="w-5 h-5 sm:w-6 sm:h-6" />
        </button>
        <button
          onClick={onToggleNotificationsPanel}
          className="relative p-2 text-[var(--theme-text-secondary)] hover:text-[var(--theme-accent-primary)] transition-colors rounded-full hover:bg-[var(--theme-bg-accent)]/50"
          aria-label="Toggle notifications panel"
          title="Notifications"
        >
          <BellIcon className="w-5 h-5 sm:w-6 sm:h-6" />
          {unreadNotificationsCount > 0 && (
            <span className="absolute top-0 right-0 block h-5 w-5 transform -translate-y-1/2 translate-x-1/2">
              <span className="absolute inline-flex h-full w-full rounded-full bg-[var(--theme-error)] opacity-75 animate-ping"></span>
              <span className="relative inline-flex rounded-full h-5 w-5 bg-[var(--theme-error)] text-white text-xs items-center justify-center">
                {unreadNotificationsCount > 9 ? '9+' : unreadNotificationsCount}
              </span>
            </span>
          )}
        </button>
      </div>
    </header>
  );
};

export default TopBar;