
import React, { useState, useEffect, useCallback } from 'react';
import { Shortcut, NotificationItem, ViewType } from '../types'; 

const ExternalLinkIcon = (props: React.SVGProps<SVGSVGElement>) => (
  <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
    <path strokeLinecap="round" strokeLinejoin="round" d="M13.5 6H5.25A2.25 2.25 0 003 8.25v10.5A2.25 2.25 0 005.25 21h10.5A2.25 2.25 0 0018 18.75V10.5m-10.5 6L21 3m0 0h-5.25M21 3v5.25" />
  </svg>
);

const TrashIcon = (props: React.SVGProps<SVGSVGElement>) => (
    <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}>
        <path strokeLinecap="round" strokeLinejoin="round" d="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12.56 0c1.153 0 2.24.032 3.223.094M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
    </svg>
);

const defaultShortcuts: Shortcut[] = [
  { id: 'yt', name: 'YouTube', url: 'https://www.youtube.com', description: 'Access videos and music.' },
  { id: 'gh', name: 'GitHub', url: 'https://github.com', description: 'Your code repositories.' },
  { id: 'local-router', name: 'Router Admin', url: 'http://192.168.1.1', description: 'Manage your network settings (example).'},
  { id: 'local-nas', name: 'NAS Storage', url: 'http://192.168.1.100/nas', description: 'Access your network attached storage (example).'},
  { id: 'home-assistant', name: 'Home Assistant', url: 'http://homeassistant.local:8123', description: 'Control your smart home (example).'},
  { id: 'jellyfin', name: 'Jellyfin', url: 'http://your-jellyfin-url:8096', description: 'Access your self-hosted media library.' },
  { id: 'gitea', name: 'Gitea / Git', url: 'http://your-git-server-url:3000', description: 'Your self-hosted Git repositories.' },
  { id: 'navidrome', name: 'Navidrome', url: 'http://your-music-server-url:4533', description: 'Self-hosted music streaming.'},
  { id: 'openai', name: 'OpenAI ChatGPT', url: 'https://chat.openai.com', description: 'Access OpenAI\'s ChatGPT platform.'},
  { id: 'claude', name: 'Anthropic Claude', url: 'https://claude.ai', description: 'Access Anthropic\'s Claude AI.'},
];

interface ShortcutsFeatureProps {
    addNotification: ( 
        type: NotificationItem['type'], 
        message: string, 
        sourceDetails?: { 
          sourceView?: ViewType; 
        }
      ) => void;
}

const ShortcutsFeature: React.FC<ShortcutsFeatureProps> = ({ addNotification }) => {
  const [shortcuts, setShortcuts] = useState<Shortcut[]>(() => {
    const savedShortcuts = localStorage.getItem('dashboard-shortcuts');
    if (savedShortcuts) {
      try {
        return JSON.parse(savedShortcuts);
      } catch (e) {
        console.error("Failed to parse shortcuts from localStorage", e);
        return defaultShortcuts;
      }
    }
    return defaultShortcuts;
  });

  useEffect(() => {
    localStorage.setItem('dashboard-shortcuts', JSON.stringify(shortcuts));
  }, [shortcuts]);

  const handleDeleteShortcut = useCallback((e: React.MouseEvent, idToDelete: string) => {
    e.preventDefault(); 
    e.stopPropagation(); 
    const shortcutToDelete = shortcuts.find(s => s.id === idToDelete);
    setShortcuts(prevShortcuts => prevShortcuts.filter(shortcut => shortcut.id !== idToDelete));
    if (shortcutToDelete && addNotification) {
        addNotification('info', `Shortcut "${shortcutToDelete.name}" removed.`, {sourceView: ViewType.SHORTCUTS});
    }
  }, [shortcuts, addNotification]);


  return (
    <div className="space-y-8">
      <header className="text-center mb-6">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-emerald-400 to-green-300">
          Quick Shortcuts
        </h1>
        <p className="text-slate-400 mt-1 text-md">
          Your favorite web UIs and services at your fingertips.
        </p>
      </header>
      
      {shortcuts.length === 0 ? (
         <div className="text-center py-10">
            <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" className="w-16 h-16 mx-auto text-slate-500 mb-4">
                 <path strokeLinecap="round" strokeLinejoin="round" d="M12 18.375a9.375 9.375 0 01-9.375-9.375V9a9.375 9.375 0 019.375-9.375h.375M12 18.375c0 .621.504 1.125 1.125 1.125h5.25c.621 0 1.125-.504 1.125-1.125V9A9.375 9.375 0 0012.375 0h-.375M12 18.375V0M8.25 7.5H12m0 0H8.25m3.75 0V4.125M8.25 7.5V4.125m0 0a1.875 1.875 0 100-3.75 1.875 1.875 0 000 3.75z" />
            </svg>
            <p className="text-xl text-slate-400">No shortcuts yet.</p>
            <p className="text-slate-500">You can add shortcuts in Settings!</p>
         </div>
      ) : (
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
          {shortcuts.map(shortcut => (
            <div key={shortcut.id} className="relative group">
                <a
                href={shortcut.url}
                target="_blank"
                rel="noopener noreferrer"
                className="block bg-slate-800 p-6 rounded-xl shadow-lg hover:shadow-sky-500/30 hover:bg-slate-700/70 transition-all duration-300 ease-in-out flex flex-col justify-between h-full"
                aria-label={`Open ${shortcut.name}`}
                >
                <div>
                    <div className="flex justify-between items-start mb-3">
                    <h3 className="text-xl font-semibold text-sky-400 group-hover:text-sky-300 transition-colors break-all mr-2">
                        {shortcut.name}
                    </h3>
                    <ExternalLinkIcon className="w-5 h-5 text-slate-500 group-hover:text-sky-400 transition-colors flex-shrink-0" />
                    </div>
                    {shortcut.description && (
                        <p className="text-sm text-slate-400 mb-2 h-10 overflow-hidden">
                            {shortcut.description}
                        </p>
                    )}
                </div>
                <p className="text-xs text-slate-500 truncate group-hover:text-slate-400 transition-colors" title={shortcut.url}>
                    {shortcut.url}
                </p>
                </a>
                <button
                    onClick={(e) => handleDeleteShortcut(e, shortcut.id)}
                    className="absolute top-2 right-2 p-1.5 bg-slate-700/50 hover:bg-red-500/70 rounded-full text-slate-400 hover:text-white transition-all opacity-0 group-hover:opacity-100 focus:opacity-100"
                    aria-label={`Delete shortcut: ${shortcut.name}`}
                    title="Delete Shortcut"
                >
                    <TrashIcon className="w-4 h-4" />
                </button>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

export default ShortcutsFeature;