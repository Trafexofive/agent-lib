# Stage 1: Build the Vite/React app
FROM node:18-alpine AS builder

WORKDIR /app

# Copy package.json and lock file (yarn.lock or package-lock.json)
# USER: Ensure your ChimeraDash project's package.json and lock file are in this directory
COPY package.json yarn.lock* package-lock.json* ./

# Install dependencies
# This will use yarn if yarn.lock is present, otherwise npm ci if package-lock.json is present, else npm install.
RUN if [ -f yarn.lock ]; then \
    echo "INFO: yarn.lock found, using 'yarn install --frozen-lockfile'"; \
    yarn install --frozen-lockfile; \
    elif [ -f package-lock.json ]; then \
    echo "INFO: package-lock.json found, using 'npm ci'"; \
    npm ci; \
    else \
    echo "WARNING: No lockfile found, using 'npm install'. This may lead to inconsistencies."; \
    npm install; \
    fi

# Copy the rest of your ChimeraDash frontend source code
COPY . .

# Build the application
# This runs the 'build' script defined in your ChimeraDash project's package.json
# It should output to a 'dist' folder in the /app directory.
RUN npm run build

# Stage 2: Serve the static build with Nginx
FROM nginx:1.25-alpine

# Copy built assets from the builder stage to Nginx's webroot
COPY --from=builder /app/dist /usr/share/nginx/html

# Copy custom Nginx configuration for SPA handling
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]
