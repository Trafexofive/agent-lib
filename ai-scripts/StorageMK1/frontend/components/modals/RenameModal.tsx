
import React, { useState, useEffect } from 'react';
import Modal from '../ui/Modal';
import Button from '../ui/Button';
import Input from '../ui/Input';
import { apiService } from '../../services/apiService';
import { useNotifications } from '../../hooks/useNotifications';
import { StorageItem } from '../../types';

interface RenameModalProps {
  item: StorageItem;
  currentPath: string; // Current directory path, useful for constructing destination
  onClose: () => void;
  onRenameSuccess: () => void;
}

const RenameModal: React.FC<RenameModalProps> = ({ item, currentPath, onClose, onRenameSuccess }) => {
  const [newName, setNewName] = useState<string>(item.name);
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const { addNotification } = useNotifications();

  useEffect(() => {
    setNewName(item.name); // Reset when item changes
  }, [item]);

  const handleRename = async () => {
    if (!newName.trim()) {
      setError("Name cannot be empty.");
      return;
    }
    if (newName.includes('/') || newName.includes('\\')) {
      setError("Name cannot contain slashes.");
      return;
    }
    if (newName.trim() === item.name) {
      onClose(); // No change
      return;
    }

    setIsLoading(true);
    setError(null);

    const parentPath = item.path.substring(0, item.path.lastIndexOf('/'));
    const destinationPath = parentPath ? `${parentPath}/${newName.trim()}` : `/${newName.trim()}`;

    try {
      await apiService.moveItem(item.path, destinationPath);
      onRenameSuccess();
    } catch (err) {
      const errorMessage = (err as Error).message || "Failed to rename item.";
      addNotification(errorMessage, 'error');
      setError(errorMessage);
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <Modal isOpen={true} onClose={onClose} title={`Rename ${item.type}`}>
      <div className="space-y-4">
        <p className="text-sm text-text-secondary">
          Current name: <span className="font-medium text-text-primary">{item.name}</span>
        </p>
        <Input
          label="New Name"
          id="newName"
          value={newName}
          onChange={(e) => setNewName(e.target.value)}
          placeholder="Enter new name"
          disabled={isLoading}
          error={error ?? undefined}
          autoFocus
          onKeyDown={(e) => { if (e.key === 'Enter') handleRename(); }}
        />
        <div className="flex justify-end space-x-3 pt-2">
          <Button onClick={onClose} variant="secondary" disabled={isLoading}>
            Cancel
          </Button>
          <Button onClick={handleRename} isLoading={isLoading} disabled={!newName.trim() || isLoading}>
            Rename
          </Button>
        </div>
      </div>
    </Modal>
  );
};

export default RenameModal;
