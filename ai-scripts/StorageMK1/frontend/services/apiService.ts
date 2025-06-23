
import { API_BASE_URL } from '../constants';
import { BrowseResponse, BackendConfig, UploadResponse, CreateFolderResponse, MoveResponse, DeleteResponse } from '../types';

const handleRealResponse = async <T,>(response: Response): Promise<T> => {
  if (!response.ok) {
    const errorData = await response.json().catch(() => ({ detail: response.statusText }));
    throw new Error(errorData.detail || `HTTP error ${response.status}`);
  }
  return response.json() as Promise<T>;
};

const handleRealFileResponse = async (response: Response): Promise<void> => {
    if (!response.ok) {
        const errorData = await response.json().catch(() => ({ detail: response.statusText }));
        throw new Error(errorData.detail || `HTTP error ${response.status}`);
    }
    const blob = await response.blob();
    const contentDisposition = response.headers.get('content-disposition');
    let filename = 'downloaded_file';
    if (contentDisposition) {
        const filenameMatch = contentDisposition.match(/filename="?([^"]+)"?/);
        if (filenameMatch && filenameMatch[1]) {
            filename = filenameMatch[1];
        }
    }
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    a.remove();
    window.URL.revokeObjectURL(url);
};

// --- Real API Functions ---
const realApiService = {
  getBackendConfig: async (): Promise<BackendConfig> => {
    const response = await fetch(`${API_BASE_URL}/api/v1/config`);
    return handleRealResponse<BackendConfig>(response);
  },

  browsePath: async (path: string): Promise<BrowseResponse> => {
    const encodedPath = encodeURIComponent(path === '' ? '/' : path); // Ensure root is '/'
    const response = await fetch(`${API_BASE_URL}/api/v1/storage/browse?path=${encodedPath}`);
    return handleRealResponse<BrowseResponse>(response);
  },

  uploadFile: async (path: string, file: File, onProgress?: (percent: number) => void): Promise<UploadResponse> => {
    const formData = new FormData();
    formData.append('file', file);
    const encodedPath = encodeURIComponent(path === '' ? '/' : path);

    return new Promise<UploadResponse>((resolve, reject) => {
      const xhr = new XMLHttpRequest();
      xhr.open('POST', `${API_BASE_URL}/api/v1/storage/upload?path=${encodedPath}`, true);

      xhr.upload.onprogress = (event) => {
        if (event.lengthComputable && onProgress) {
          const percentComplete = Math.round((event.loaded / event.total) * 100);
          onProgress(percentComplete);
        }
      };

      xhr.onload = () => {
        if (xhr.status >= 200 && xhr.status < 300) {
          resolve(JSON.parse(xhr.responseText) as UploadResponse);
        } else {
          try {
            const errorData = JSON.parse(xhr.responseText);
            reject(new Error(errorData.detail || `HTTP error ${xhr.status}`));
          } catch (e) {
            reject(new Error(`HTTP error ${xhr.status}: ${xhr.statusText}`));
          }
        }
      };

      xhr.onerror = () => {
        reject(new Error('Network error during upload.'));
      };
      
      xhr.send(formData);
    });
  },

  downloadFile: async (filePath: string): Promise<void> => {
    const encodedFilePath = encodeURIComponent(filePath);
    const response = await fetch(`${API_BASE_URL}/api/v1/storage/download?filepath=${encodedFilePath}`);
    return handleRealFileResponse(response);
  },

  createDirectory: async (path: string): Promise<CreateFolderResponse> => {
    const response = await fetch(`${API_BASE_URL}/api/v1/storage/mkdir`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ path }),
    });
    return handleRealResponse<CreateFolderResponse>(response);
  },

  deleteItem: async (path: string): Promise<DeleteResponse> => {
    const response = await fetch(`${API_BASE_URL}/api/v1/storage/delete`, {
      method: 'DELETE',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ path }), 
    });
    return handleRealResponse<DeleteResponse>(response);
  },

  moveItem: async (sourcePath: string, destinationPath: string): Promise<MoveResponse> => {
    const response = await fetch(`${API_BASE_URL}/api/v1/storage/move`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ source_path: sourcePath, destination_path: destinationPath }),
    });
    return handleRealResponse<MoveResponse>(response);
  },
};

export const apiService = realApiService;