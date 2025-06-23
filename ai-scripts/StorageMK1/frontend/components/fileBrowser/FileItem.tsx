
import React, { useState } from 'react';
import { StorageItem } from '../../types';
import { apiService } from '../../services/apiService';
import { useNotifications } from '../../hooks/useNotifications';
import { FolderIcon, FileIcon, DownloadIcon, PencilIcon, TrashIcon, DotsVerticalIcon } from '../icons/heroicons';
import Button from '../ui/Button';

interface FileItemProps {
  item: StorageItem;
  onNavigate: (path: string) => void;
  onRename: () => void;
  onDelete: () => void;
  maxUploadSizeMb?: number;
}

const formatBytes = (bytes?: number, decimals = 2): string => {
  if (bytes === undefined || bytes === 0) return '0 Bytes';
  const k = 1024;
  const dm = decimals < 0 ? 0 : decimals;
  const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
};

const formatDate = (dateString: string): string => {
  const date = new Date(dateString);
  return `${date.toLocaleDateString()} ${date.toLocaleTimeString()}`;
};

const FileItem: React.FC<FileItemProps> = ({ item, onNavigate, onRename, onDelete }) => {
  const { addNotification } = useNotifications();
  const [isMenuOpen, setIsMenuOpen] = useState(false);

  const handleDownload = async () => {
    try {
      await apiService.downloadFile(item.path);
    } catch (error) {
      addNotification(`Error downloading ${item.name}: ${(error as Error).message}`, 'error');
    }
  };

  const handleItemClick = (e: React.MouseEvent) => {
    // Prevent navigation if clicking on action buttons inside the row
    if ((e.target as HTMLElement).closest('button')) {
      return;
    }
    if (item.type === 'directory') {
      onNavigate(item.path);
    } else {
      // Optional: open preview or details for files on click, or do nothing
      // For now, file clicks do nothing, actions are via buttons.
    }
  };

  return (
    <tr 
      className={`hover:bg-gray-700/70 transition-colors duration-150 ${item.type === 'directory' ? 'cursor-pointer' : ''}`}
      onClick={handleItemClick}
    >
      <td className="px-4 py-3 whitespace-nowrap">
        <div className="flex items-center">
          {item.type === 'directory' ? (
            <FolderIcon className="h-6 w-6 text-primary-accent mr-3 flex-shrink-0" />
          ) : (
            <FileIcon className="h-6 w-6 text-gray-400 mr-3 flex-shrink-0" />
          )}
          <span className="text-sm font-medium text-text-primary truncate" title={item.name}>{item.name}</span>
        </div>
      </td>
      <td className="px-4 py-3 whitespace-nowrap text-sm text-text-secondary hidden sm:table-cell">{formatBytes(item.size)}</td>
      <td className="px-4 py-3 whitespace-nowrap text-sm text-text-secondary hidden md:table-cell">{formatDate(item.modified_at)}</td>
      <td className="px-4 py-3 whitespace-nowrap text-right text-sm font-medium">
        {/* Desktop Actions */}
        <div className="hidden sm:flex space-x-2 justify-end">
          {item.type === 'file' && (
            <Button onClick={handleDownload} variant="icon" size="sm" title="Download">
              <DownloadIcon className="h-5 w-5" />
            </Button>
          )}
          <Button onClick={onRename} variant="icon" size="sm" title="Rename">
            <PencilIcon className="h-5 w-5" />
          </Button>
          <Button onClick={onDelete} variant="icon" size="sm" title="Delete" className="text-red-500 hover:text-red-400">
            <TrashIcon className="h-5 w-5" />
          </Button>
        </div>
        {/* Mobile Actions Menu */}
        <div className="sm:hidden relative">
          <Button onClick={(e) => { e.stopPropagation(); setIsMenuOpen(!isMenuOpen); }} variant="icon" size="sm" title="Actions">
            <DotsVerticalIcon className="h-5 w-5" />
          </Button>
          {isMenuOpen && (
            <div 
                className="origin-top-right absolute right-0 mt-2 w-40 rounded-md shadow-lg bg-gray-700 ring-1 ring-black ring-opacity-5 focus:outline-none z-10"
                onMouseLeave={() => setIsMenuOpen(false)} // Close on mouse leave
            >
              <div className="py-1" role="menu" aria-orientation="vertical" aria-labelledby="options-menu">
                {item.type === 'file' && (
                  <button
                    onClick={(e) => { e.stopPropagation(); handleDownload(); setIsMenuOpen(false); }}
                    className="w-full text-left flex items-center px-4 py-2 text-sm text-text-primary hover:bg-gray-600 hover:text-white"
                    role="menuitem"
                  >
                    <DownloadIcon className="h-4 w-4 mr-2" /> Download
                  </button>
                )}
                <button
                  onClick={(e) => { e.stopPropagation(); onRename(); setIsMenuOpen(false); }}
                  className="w-full text-left flex items-center px-4 py-2 text-sm text-text-primary hover:bg-gray-600 hover:text-white"
                  role="menuitem"
                >
                  <PencilIcon className="h-4 w-4 mr-2" /> Rename
                </button>
                <button
                  onClick={(e) => { e.stopPropagation(); onDelete(); setIsMenuOpen(false);}}
                  className="w-full text-left flex items-center px-4 py-2 text-sm text-red-500 hover:bg-gray-600 hover:text-red-400"
                  role="menuitem"
                >
                  <TrashIcon className="h-4 w-4 mr-2" /> Delete
                </button>
              </div>
            </div>
          )}
        </div>
      </td>
    </tr>
  );
};

export default FileItem;
