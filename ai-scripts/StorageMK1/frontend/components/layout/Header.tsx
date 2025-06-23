
import React from 'react';
import Breadcrumbs from '../fileBrowser/Breadcrumbs';
import Button from '../ui/Button';
import Input from '../ui/Input'; // Import Input
import { UploadIcon, PlusIcon, RefreshIcon, SearchIcon } from '../icons/heroicons';

interface HeaderProps {
  currentPath: string;
  onNavigate: (path: string) => void;
  onUploadClick: () => void;
  onCreateFolderClick: () => void;
  onRefresh: () => void;
  searchTerm: string;
  onSearchChange: (term: string) => void;
}

const Header: React.FC<HeaderProps> = ({
  currentPath,
  onNavigate,
  onUploadClick,
  onCreateFolderClick,
  onRefresh,
  searchTerm,
  onSearchChange,
}) => {
  return (
    <header className="mb-6 p-4 bg-card-bg backdrop-filter backdrop-blur-xs rounded-lg shadow-lg">
      <div className="container mx-auto flex flex-col sm:flex-row justify-between items-center space-y-4 sm:space-y-0 sm:space-x-4">
        <div className="w-full sm:w-auto sm:flex-shrink">
          <Breadcrumbs currentPath={currentPath} onNavigate={onNavigate} />
        </div>
        
        <div className="w-full sm:w-auto flex flex-col sm:flex-row items-center space-y-3 sm:space-y-0 sm:space-x-3">
          {/* Search Input */}
          <div className="relative group w-full sm:w-56 md:w-64">
            <span className="absolute inset-y-0 left-0 pl-3 flex items-center pointer-events-none">
              <SearchIcon className="h-5 w-5 text-gray-400 group-focus-within:text-primary-accent transition-colors" />
            </span>
            <Input
              id="global-search-input"
              type="search"
              placeholder="Search (Ctrl+K)"
              value={searchTerm}
              onChange={(e) => onSearchChange(e.target.value)}
              className="pl-10 w-full !py-2 text-sm" // Custom padding for icon and height
            />
          </div>

          {/* Action Buttons */}
          <div className="flex space-x-2 sm:space-x-3">
            <Button onClick={onRefresh} variant="secondary" size="sm" className="flex items-center">
              <RefreshIcon className="h-5 w-5 mr-1 sm:mr-0 md:mr-1" />
              <span className="hidden md:inline">Refresh</span>
            </Button>
            <Button onClick={onUploadClick} variant="primary" size="sm" className="flex items-center">
              <UploadIcon className="h-5 w-5 mr-1 sm:mr-0 md:mr-1" />
              <span className="hidden md:inline">Upload</span>
            </Button>
            <Button onClick={onCreateFolderClick} variant="primary" size="sm" className="flex items-center">
              <PlusIcon className="h-5 w-5 mr-1 sm:mr-0 md:mr-1" />
              <span className="hidden md:inline">New Folder</span>
            </Button>
          </div>
        </div>
      </div>
    </header>
  );
};

export default Header;