
export interface StorageItem {
  name: string;
  type: 'file' | 'directory';
  size?: number; // In bytes, undefined for directories
  modified_at: string; // ISO date string
  path: string; // Full path relative to storage root, e.g. /documents/file.txt
}

export interface BrowseResponse {
  path: string;
  items: StorageItem[];
}

export interface BackendConfig {
  storage_base_path_container: string;
  max_upload_size_mb: number;
  default_listing_sort: string;
  allow_overwrite_on_upload: boolean;
  api_title: string;
  api_version: string;
  hidden_files_prefixes: string[];
}

export interface UploadResponse {
  message: string;
  filename: string;
  path: string;
  size: number;
}

export interface CreateFolderResponse {
  message: string;
  path: string;
}

export interface MoveResponse {
  message: string;
  source_path: string;
  destination_path: string;
}

export interface DeleteResponse {
  message: string;
  path: string;
}

export type NotificationType = 'success' | 'error' | 'info' | 'warning';

export interface Notification {
  id: string;
  message: string;
  type: NotificationType;
}
