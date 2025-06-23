# IntelStream Agents UI - Setup Instructions (v0.1.0)

This relic contains the frontend application for IntelStream Agents, built with React, TypeScript, and Vite.

## Prerequisites:

-   Node.js (v18.x or later recommended) and npm (or yarn/pnpm).

## 1. Materialize Project

Using `relic_materializer.py`:
```bash
python relic_materializer.py path/to/this_intelstream_ui_plan.json --output-dir ./frontend_apps --force
cd ./frontend_apps/intelstream-agents-ui
```
This will create the `intelstream-agents-ui/` directory containing all the project files.

## 2. Environment Configuration

1.  After materializing, navigate to the `intelstream-agents-ui` project root directory.
2.  You will find a file named `.env.local.example`. Copy this file to a new file named `.env.local`:
    ```bash
    cp .env.local.example .env.local
    ```
3.  Open `.env.local` in a text editor.
4.  Replace `YOUR_GEMINI_API_KEY_HERE` with your actual Google AI Studio Gemini API key.
    ```env
    GEMINI_API_KEY=AIzxxxxxxxxxxxxxxxxxxxxxxxxxxx
    ```

    **Important Security Note:** The `.env.local` file is gitignored by default. This method of including the API key is suitable **only for local development**. For a deployed application, API keys should be handled securely on a backend server. The frontend should make requests to your backend, which then calls the Gemini API. Exposing API keys directly in client-side code is a significant security risk in production environments.

## 3. Install Dependencies

Open your terminal in the `intelstream-agents-ui` project root and run:
```bash
npm install
# or if you use yarn:
# yarn install
# or if you use pnpm:
# pnpm install
```

## 4. Run Locally for Development

Once dependencies are installed, start the Vite development server:
```bash
npm run dev
# or yarn dev / pnpm dev
```
This will typically start the application (by default on `http://localhost:5173` or the next available port). Open this URL in your web browser.

## 5. Build for Production

To create an optimized static build for production deployment:
```bash
npm run build
# or yarn build / pnpm build
```
The production-ready files will be generated in the `dist/` directory. These files can then be deployed to any static web hosting service (e.g., Vercel, Netlify, GitHub Pages, or your own server like Nginx/Apache).

## 6. Preview Production Build

After building, you can preview the production build locally to ensure everything works as expected:
```bash
npm run preview
# or yarn preview / pnpm preview
```
This will serve the `dist/` directory, usually on a different local port.

## Project Structure Overview:

-   `src/` or `/` (root for TSX/TS): Main application source code.
    -   `App.tsx`: Root application component, state management, routing logic.
    -   `main.tsx` or `index.tsx`: Entry point that renders the App component.
    -   `components/`: Reusable UI components.
    -   `contexts/`: React Context providers (e.g., `ToastContext`).
    -   `services/`: Modules for external interactions (e.g., `apiService.ts` for local storage abstraction, `geminiService.ts` for Gemini API calls).
    -   `types.ts`: TypeScript type definitions.
    -   `constants.tsx`: Application-wide constants (e.g., APP_NAME, icons).
-   `index.html`: The main HTML entry point for the Vite application.
-   `vite.config.ts`: Vite build and development server configuration.
-   `tsconfig.json`: TypeScript compiler options.
-   `package.json`: Project dependencies and scripts.
-   `.env.local.example`: Template for environment variables.
-   `README.md`: Basic project information (you are reading a version of this).
-   `metadata.json`: (Likely for AI Studio specific integration, if applicable).

This setup provides a fully functional frontend application for managing IntelStream Agents.