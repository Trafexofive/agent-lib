# Stage 1: Create a placeholder for static assets
FROM alpine:latest AS builder
WORKDIR /app
# Create a dummy index.html. In a real project, this stage would run 'npm run build'.
RUN mkdir -p /app/dist
RUN echo '<!DOCTYPE html><html><head><title>Comms Admin</title><style>body{background-color:#111827;color:#d1d5db;font-family:sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;}.container{text-align:center;border:1px solid #374151;padding:2rem;border-radius:0.5rem;}</style></head><body><div class="container"><h1>Praetorian Comms - Admin Dashboard</h1><p>Awaiting Vibe Integration...</p></div></body></html>' > /app/dist/index.html

# Stage 2: Serve the static files with Nginx
FROM nginx:1.25-alpine
# Copy the built/placeholder assets from the builder stage
COPY --from=builder /app/dist /usr/share/nginx/html
# Copy a custom Nginx configuration to handle SPA routing
COPY nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
