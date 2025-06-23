
import React, { useState } from 'react';
import Modal from '../ui/Modal';
import Button from '../ui/Button';
import { apiService } from '../../services/apiService';
import { useNotifications } from '../../hooks/useNotifications';

interface DeleteConfirmationModalProps {
  itemPath: string;
  itemName: string;
  itemType: 'file' | 'directory';
  onClose: () => void;
  onDeleteSuccess: () => void;
}

const DeleteConfirmationModal: React.FC<DeleteConfirmationModalProps> = ({ itemPath, itemName, itemType, onClose, onDeleteSuccess }) => {
  const [isLoading, setIsLoading] = useState<boolean>(false);
  const { addNotification } = useNotifications();

  const handleDelete = async () => {
    setIsLoading(true);
    try {
      await apiService.deleteItem(itemPath);
      onDeleteSuccess();
    } catch (err) {
      addNotification(`Failed to delete ${itemName}: ${(err as Error).message}`, 'error');
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <Modal isOpen={true} onClose={onClose} title={`Delete ${itemType}`}>
      <div className="space-y-4">
        <p className="text-sm text-text-secondary">
          Are you sure you want to delete <span className="font-medium text-text-primary">{itemName}</span>?
          {itemType === 'directory' && <span className="block text-xs text-yellow-400">This will delete the directory and all its contents.</span>}
          This action cannot be undone.
        </p>
        <div className="flex justify-end space-x-3 pt-2">
          <Button onClick={onClose} variant="secondary" disabled={isLoading}>
            Cancel
          </Button>
          <Button onClick={handleDelete} variant="danger" isLoading={isLoading}>
            Delete
          </Button>
        </div>
      </div>
    </Modal>
  );
};

export default DeleteConfirmationModal;
