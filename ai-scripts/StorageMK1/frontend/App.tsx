
import React, { useState, useEffect, useCallback } from 'react';
import { StorageItem, BackendConfig } from './types';
import { apiService } from './services/apiService';
import { useNotifications } from './hooks/useNotifications';
import Header from './components/layout/Header';
import MainContentLayout from './components/layout/MainContentLayout';
import FileBrowser from './components/fileBrowser/FileBrowser';
import UploadModal from './components/modals/UploadModal';
import CreateFolderModal from './components/modals/CreateFolderModal';
import RenameModal from './components/modals/RenameModal';
import DeleteConfirmationModal from './components/modals/DeleteConfirmationModal';
import Spinner from './components/ui/Spinner';

const App: React.FC = () => {
  const [currentPath, setCurrentPath] = useState<string>('/');
  const [items, setItems] = useState<StorageItem[]>([]);
  const [isLoading, setIsLoading] = useState<boolean>(true);
  const [backendConfig, setBackendConfig] = useState<BackendConfig | null>(null);
  const [searchTerm, setSearchTerm] = useState<string>('');
  
  const [showUploadModal, setShowUploadModal] = useState<boolean>(false);
  const [showCreateFolderModal, setShowCreateFolderModal] = useState<boolean>(false);
  const [itemToRename, setItemToRename] = useState<StorageItem | null>(null);
  const [itemToDelete, setItemToDelete] = useState<StorageItem | null>(null);

  const { addNotification } = useNotifications();

  const fetchConfig = useCallback(async () => {
    try {
      const config = await apiService.getBackendConfig();
      setBackendConfig(config);
    } catch (error) {
      addNotification('Failed to load backend configuration.', 'error');
      console.error("Error fetching config:", error);
    }
  }, [addNotification]);

  const fetchItems = useCallback(async (path: string) => {
    setIsLoading(true);
    try {
      const browseResponse = await apiService.browsePath(path);
      setItems(browseResponse.items);
      setCurrentPath(browseResponse.path); // Ensure currentPath is updated from response
    } catch (error) {
      addNotification(`Error fetching items for ${path}: ${(error as Error).message}`, 'error');
      console.error("Error fetching items:", error);
      // If path fails, maybe navigate to root or previous valid path
      if (path !== '/') setCurrentPath('/'); 
    } finally {
      setIsLoading(false);
    }
  }, [addNotification]);

  useEffect(() => {
    fetchConfig();
  }, [fetchConfig]);

  useEffect(() => {
    fetchItems(currentPath);
  }, [currentPath, fetchItems]);

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key === 'k') {
        event.preventDefault();
        const searchInput = document.getElementById('global-search-input');
        if (searchInput) {
          searchInput.focus();
        }
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, []);


  const handleNavigate = (path: string) => {
    setCurrentPath(path);
    setSearchTerm(''); // Clear search when navigating
  };

  const handleRefresh = () => {
    fetchItems(currentPath);
     setSearchTerm(''); // Optionally clear search on refresh
  };

  const handleUploadSuccess = () => {
    setShowUploadModal(false);
    addNotification('File uploaded successfully!', 'success');
    fetchItems(currentPath);
  };

  const handleCreateFolderSuccess = () => {
    setShowCreateFolderModal(false);
    addNotification('Folder created successfully!', 'success');
    fetchItems(currentPath);
  };

  const handleRenameSuccess = () => {
    setItemToRename(null);
    addNotification('Item renamed successfully!', 'success');
    fetchItems(currentPath);
  };

  const handleDeleteSuccess = () => {
    setItemToDelete(null);
    addNotification('Item deleted successfully!', 'success');
    fetchItems(currentPath);
  };
  
  const filteredItems = items.filter(item =>
    item.name.toLowerCase().includes(searchTerm.toLowerCase())
  );

  return (
    <div className="min-h-screen flex flex-col p-4 md:p-8 selection:bg-primary-accent selection:text-white">
      <Header
        currentPath={currentPath}
        onNavigate={handleNavigate}
        onUploadClick={() => setShowUploadModal(true)}
        onCreateFolderClick={() => setShowCreateFolderModal(true)}
        onRefresh={handleRefresh}
        searchTerm={searchTerm}
        onSearchChange={setSearchTerm}
      />
      <MainContentLayout>
        {isLoading ? (
          <div className="flex justify-center items-center h-64">
            <Spinner size="lg" />
          </div>
        ) : (
          <FileBrowser
            items={filteredItems}
            currentPath={currentPath}
            onNavigate={handleNavigate}
            onRename={setItemToRename}
            onDelete={setItemToDelete}
          />
        )}
      </MainContentLayout>

      {showUploadModal && backendConfig && (
        <UploadModal
          currentPath={currentPath}
          maxFileSizeMB={backendConfig.max_upload_size_mb}
          allowOverwrite={backendConfig.allow_overwrite_on_upload}
          onClose={() => setShowUploadModal(false)}
          onUploadSuccess={handleUploadSuccess}
        />
      )}
      {showCreateFolderModal && (
        <CreateFolderModal
          currentPath={currentPath}
          onClose={() => setShowCreateFolderModal(false)}
          onCreateSuccess={handleCreateFolderSuccess}
        />
      )}
      {itemToRename && (
        <RenameModal
          item={itemToRename}
          currentPath={currentPath}
          onClose={() => setItemToRename(null)}
          onRenameSuccess={handleRenameSuccess}
        />
      )}
      {itemToDelete && (
        <DeleteConfirmationModal
          itemPath={itemToDelete.path}
          itemName={itemToDelete.name}
          itemType={itemToDelete.type}
          onClose={() => setItemToDelete(null)}
          onDeleteSuccess={handleDeleteSuccess}
        />
      )}
    </div>
  );
};

export default App;