
import React, { useState, useCallback, ChangeEvent } from 'react';
import Modal from '../ui/Modal';
import Button from '../ui/Button';
import { apiService } from '../../services/apiService';
import { useNotifications } from '../../hooks/useNotifications';
import { UploadIcon } from '../icons/heroicons';

interface UploadModalProps {
  currentPath: string;
  maxFileSizeMB: number;
  allowOverwrite: boolean;
  onClose: () => void;
  onUploadSuccess: () => void;
}

const UploadModal: React.FC<UploadModalProps> = ({ currentPath, maxFileSizeMB, allowOverwrite, onClose, onUploadSuccess }) => {
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [isUploading, setIsUploading] = useState<boolean>(false);
  const [uploadProgress, setUploadProgress] = useState<number>(0);
  const [error, setError] = useState<string | null>(null);
  const { addNotification } = useNotifications();

  const handleFileChange = (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      if (file.size > maxFileSizeMB * 1024 * 1024) {
        setError(`File exceeds maximum size of ${maxFileSizeMB}MB.`);
        setSelectedFile(null);
      } else {
        setSelectedFile(file);
        setError(null);
      }
    }
  };

  const handleUpload = async () => {
    if (!selectedFile) {
      setError("Please select a file to upload.");
      return;
    }
    
    setIsUploading(true);
    setError(null);
    setUploadProgress(0);

    try {
      // The backend config 'allow_overwrite_on_upload' is handled by backend.
      // We just inform the user if they are about to overwrite, if we had a way to check existence first.
      // For simplicity, backend handles overwrite logic.
      await apiService.uploadFile(currentPath, selectedFile, (progress) => {
        setUploadProgress(progress);
      });
      onUploadSuccess();
    } catch (err) {
      const errorMessage = (err as Error).message || "Upload failed. Please try again.";
      addNotification(errorMessage, 'error');
      setError(errorMessage);
    } finally {
      setIsUploading(false);
    }
  };

  return (
    <Modal isOpen={true} onClose={onClose} title={`Upload File to ${currentPath === '/' ? 'Root' : currentPath}`}>
      <div className="space-y-4">
        <div>
          <label htmlFor="file-upload" className="block text-sm font-medium text-text-secondary mb-1">
            Select file (Max: {maxFileSizeMB}MB)
          </label>
          <div className="mt-1 flex justify-center px-6 pt-5 pb-6 border-2 border-border-primary border-dashed rounded-md">
            <div className="space-y-1 text-center">
              <UploadIcon className="mx-auto h-12 w-12 text-text-secondary" />
              <div className="flex text-sm text-gray-400">
                <label
                  htmlFor="file-upload"
                  className="relative cursor-pointer bg-gray-700 rounded-md font-medium text-primary-accent hover:text-pink-400 focus-within:outline-none focus-within:ring-2 focus-within:ring-offset-2 focus-within:ring-offset-gray-800 focus-within:ring-primary-accent px-1"
                >
                  <span>Upload a file</span>
                  <input id="file-upload" name="file-upload" type="file" className="sr-only" onChange={handleFileChange} disabled={isUploading} />
                </label>
                <p className="pl-1">or drag and drop</p>
              </div>
              {selectedFile && <p className="text-xs text-gray-300 pt-2">{selectedFile.name}</p>}
              {!selectedFile && <p className="text-xs text-gray-500">PNG, JPG, PDF, etc. up to {maxFileSizeMB}MB</p>}
            </div>
          </div>
        </div>

        {error && <p className="text-sm text-red-400">{error}</p>}
        
        {isUploading && (
          <div className="w-full bg-gray-600 rounded-full h-2.5">
            <div className="bg-primary-accent h-2.5 rounded-full transition-all duration-300 ease-out" style={{ width: `${uploadProgress}%` }}></div>
            <p className="text-xs text-center text-text-secondary mt-1">{uploadProgress}%</p>
          </div>
        )}

        {!allowOverwrite && <p className="text-xs text-yellow-400">Note: Overwriting existing files is currently disabled by server configuration.</p>}
        
        <div className="flex justify-end space-x-3 pt-2">
          <Button onClick={onClose} variant="secondary" disabled={isUploading}>
            Cancel
          </Button>
          <Button onClick={handleUpload} isLoading={isUploading} disabled={!selectedFile || isUploading}>
            {isUploading ? 'Uploading...' : 'Upload'}
          </Button>
        </div>
      </div>
    </Modal>
  );
};

export default UploadModal;
