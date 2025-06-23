
import React from 'react';
import { StorageItem } from '../../types';
import FileItem from './FileItem';
import { FolderOpenIcon, SearchIcon } from '../icons/heroicons'; // Added SearchIcon

interface FileBrowserProps {
  items: StorageItem[];
  currentPath: string;
  onNavigate: (path: string) => void;
  onRename: (item: StorageItem) => void;
  onDelete: (item: StorageItem) => void;
}

const FileBrowser: React.FC<FileBrowserProps> = ({ items, currentPath, onNavigate, onRename, onDelete }) => {
  // items here are already filtered by search term from App.tsx
  
  if (items.length === 0 && currentPath === '/') {
    // Check if this is due to search yielding no results vs. actual empty root
    // This distinction might require knowing the original unfiltered item count or search term presence.
    // For now, simplified:
    return (
      <div className="text-center py-10 text-text-secondary">
        <FolderOpenIcon className="h-16 w-16 mx-auto mb-4 text-gray-500" />
        <p className="text-xl">Storage is empty or no items match your search.</p>
        <p>Upload files or create a new folder to get started.</p>
      </div>
    );
  }
  
  if (items.length === 0) {
    return (
      <div className="text-center py-10 text-text-secondary">
        <SearchIcon className="h-16 w-16 mx-auto mb-4 text-gray-500" />
        <p className="text-xl">No items match your search in this folder.</p>
        <p>Try a different search term or clear the search.</p>
      </div>
    );
  }

  return (
    <div className="overflow-x-auto">
      <table className="min-w-full divide-y divide-border-primary">
        <thead className="bg-gray-700/50">
          <tr>
            <th scope="col" className="px-4 py-3 text-left text-xs font-medium text-text-secondary uppercase tracking-wider">Name</th>
            <th scope="col" className="px-4 py-3 text-left text-xs font-medium text-text-secondary uppercase tracking-wider hidden sm:table-cell">Size</th>
            <th scope="col" className="px-4 py-3 text-left text-xs font-medium text-text-secondary uppercase tracking-wider hidden md:table-cell">Modified</th>
            <th scope="col" className="px-4 py-3 text-right text-xs font-medium text-text-secondary uppercase tracking-wider">Actions</th>
          </tr>
        </thead>
        <tbody className="bg-transparent divide-y divide-border-primary">
          {items.map(item => (
            <FileItem
              key={item.path} 
              item={item}
              onNavigate={onNavigate}
              onRename={() => onRename(item)}
              onDelete={() => onDelete(item)}
            />
          ))}
        </tbody>
      </table>
    </div>
  );
};

export default FileBrowser;