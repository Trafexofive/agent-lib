
import React, { useState } from 'react';
import Modal from '../ui/Modal';
import Button from '../ui/Button';
import Input from '../ui/Input';
import { apiService } from '../../services/apiService';
import { useNotifications } from '../../hooks/useNotifications';

interface CreateFolderModalProps {
  currentPath: string;
  onClose: () => void;
  onCreateSuccess: () => void;
}

const CreateFolderModal: React.FC<CreateFolderModalProps> = ({ currentPath, onClose, onCreateSuccess }) => {
  const [folderName, setFolderName] = useState<string>('');
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const { addNotification } = useNotifications();

  const handleCreateFolder = async () => {
    if (!folderName.trim()) {
      setError("Folder name cannot be empty.");
      return;
    }
    if (folderName.includes('/') || folderName.includes('\\')) {
      setError("Folder name cannot contain slashes.");
      return;
    }

    setIsLoading(true);
    setError(null);
    const newFolderPath = currentPath === '/' ? `/${folderName.trim()}` : `${currentPath}/${folderName.trim()}`;
    
    try {
      await apiService.createDirectory(newFolderPath);
      onCreateSuccess();
    } catch (err) {
      const errorMessage = (err as Error).message || "Failed to create folder.";
      addNotification(errorMessage, 'error');
      setError(errorMessage);
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <Modal isOpen={true} onClose={onClose} title={`Create New Folder in ${currentPath === '/' ? 'Root' : currentPath}`}>
      <div className="space-y-4">
        <Input
          label="Folder Name"
          id="folderName"
          value={folderName}
          onChange={(e) => setFolderName(e.target.value)}
          placeholder="Enter folder name"
          disabled={isLoading}
          error={error ?? undefined}
          autoFocus
        />
        <div className="flex justify-end space-x-3 pt-2">
          <Button onClick={onClose} variant="secondary" disabled={isLoading}>
            Cancel
          </Button>
          <Button onClick={handleCreateFolder} isLoading={isLoading} disabled={!folderName.trim() || isLoading}>
            Create Folder
          </Button>
        </div>
      </div>
    </Modal>
  );
};

export default CreateFolderModal;
