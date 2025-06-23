# Stage 1: Build the React/Vite app
# This stage assumes the user's frontend code (package.json, vite.config.ts, src, etc.)
# is placed in the 'frontend_service_code' directory.
FROM node:18-alpine AS builder

WORKDIR /app

# Copy package.json and package-lock.json (or yarn.lock)
# Ensure these files exist in your frontend_service_code directory from your chimeradash project
COPY package.json yarn.lock* package-lock.json* ./ 

# Install dependencies based on lock file
RUN if [ -f yarn.lock ]; then \
    echo "Found yarn.lock, using yarn"; \
    yarn install --frozen-lockfile; \
    elif [ -f package-lock.json ]; then \
    echo "Found package-lock.json, using npm ci"; \
    npm ci; \
    else \
    echo "No lockfile found, using npm install (this may lead to inconsistencies)"; \
    npm install; \
    fi

# Copy the rest of the application code
COPY . .

# Build the application (common Vite command from your chimeradash project's package.json)
RUN npm run build

# Stage 2: Serve the static build with Nginx
FROM nginx:1.25-alpine

# Copy built assets from the builder stage
COPY --from=builder /app/dist /usr/share/nginx/html

# Copy custom Nginx configuration
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
