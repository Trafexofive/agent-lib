import path from 'path';
import { defineConfig, loadEnv } from 'vite';

export default defineConfig(({ mode }) => {
    // Load .env variables from the project root into process.env
    // Vite automatically loads .env files. VITE_ prefixed variables are exposed to client code via import.meta.env.
    // Non-VITE_ prefixed variables are available here in process.env if loaded by loadEnv.
    const env = loadEnv(mode, process.cwd(), ''); 

    // Determine allowed hosts. Start with default localhost and IPs.
    // Add the DOMAIN_NAME from .env if it exists.
    const allowed = [
        // Default allowed by Vite: localhost, .localhost, all IPs
        // We can explicitly add the domain from .env for clarity or specific needs.
    ];
    if (env.DOMAIN_NAME && env.DOMAIN_NAME !== 'localhost') {
        allowed.push(env.DOMAIN_NAME);
        allowed.push(`.${env.DOMAIN_NAME}`); // Allow subdomains
    }

    return {
      // VITE_ prefixed env variables are automatically available in import.meta.env on the client.
      // No explicit 'define' needed in vite.config.js for VITE_API_BASE_URL if you use import.meta.env in your app code.

      resolve: {
        alias: {
          '@': path.resolve(__dirname, '.'),
        }
      },
      server: {
        host: true, // Listen on all network interfaces (e.g., 0.0.0.0) to be accessible in Docker
        port: parseInt(env.FRONTEND_PORT_CONTAINER || "5173"), // Use port from .env or default
        // allowedHosts: allowed.length > 0 ? allowed : undefined, // Only set if we have specific hosts beyond default
        // To allow a specific host like 'nas.clevo.ddnsgeek.com', add it here or use a more general approach.
        // For maximum flexibility in a homelab, allowing all might be simpler if security implications are understood.
        // If you want to allow ALL hosts (use with caution, understands security implications):
        allowedHosts: true, 
        // Or, to be more specific, including the reported problematic host:
        // allowedHosts: [env.DOMAIN_NAME, 'nas.clevo.ddnsgeek.com', `.${env.DOMAIN_NAME}`].filter(Boolean),
        
        // HMR (Hot Module Replacement) configuration for Docker might be needed if proxying
        // hmr: {
        //   clientPort: parseInt(env.FRONTEND_PORT_HOST || "5173"), 
        // },
        watch: {
          usePolling: true, // Necessary for HMR in some Docker/WSL environments
        },
      }
    };
});
