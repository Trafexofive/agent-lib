/// <reference types="vite/client" />

interface ImportMetaEnv {
  readonly VITE_API_BASE_URL: string
  // Add other VITE_ prefixed env variables your app uses here
}

interface ImportMeta {
  readonly env: ImportMetaEnv
}
