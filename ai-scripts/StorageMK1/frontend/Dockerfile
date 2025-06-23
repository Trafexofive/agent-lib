# Placeholder Dockerfile for user-provided frontend.
# PRAETORIAN_CHIMERA: Replace this with your actual frontend Dockerfile.
# This example assumes a Node.js/Vite based frontend for development.

FROM node:18-alpine

WORKDIR /usr/src/app

# Copy package files and install dependencies
# Adjust if you use yarn or pnpm
COPY package*.json ./
RUN npm install

# Copy the rest of your frontend code
COPY . .

# Expose the port your frontend dev server runs on (e.g., Vite default is 5173)
# This should match FRONTEND_PORT_CONTAINER in your .env file.
EXPOSE ${FRONTEND_PORT_CONTAINER:-5173}

# Command to run your frontend development server
# Adjust if your start script is different (e.g., 'yarn dev', 'npm start')
# The '--host' flag is often needed to make the dev server accessible outside the container.
CMD ["npm", "run", "dev", "--", "--host"]

# For a production build, you would typically:
# 1. RUN npm run build (or equivalent)
# 2. Use a multi-stage build with a web server like Nginx to serve static files from the build output directory (e.g., 'dist').
# Example (conceptual) multi-stage build snippet:
#
# FROM node:18-alpine as builder
# WORKDIR /usr/src/app
# COPY package*.json ./
# RUN npm install
# COPY . .
# RUN npm run build
# 
# FROM nginx:alpine
# COPY --from=builder /usr/src/app/dist /usr/share/nginx/html
# COPY nginx.conf /etc/nginx/conf.d/default.conf # (Optional custom Nginx config)
# EXPOSE 80
# CMD ["nginx", "-g", "daemon off;"]
