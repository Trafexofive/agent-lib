# Use a slim, modern Python image
FROM python:3.11-slim-bookworm

# Set environment variables to prevent Python from buffering stdout/stderr and writing .pyc files
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1

# Set the working directory in the container
WORKDIR /app

# Copy the dependencies file first to leverage Docker layer caching
COPY requirements.txt .

# Install build dependencies for psycopg2, then install Python packages, then remove build dependencies to keep the image small.
# This is a key optimization step.
RUN apt-get update && \
    apt-get install -y --no-install-recommends gcc libpq-dev && \
    pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt && \
    apt-get purge -y --auto-remove gcc libpq-dev && \
    rm -rf /var/lib/apt/lists/*

# Copy the rest of the application code into the container
COPY . .

# Make the entrypoint script executable
RUN chmod +x /app/entrypoint.sh

# Expose the port the app runs on
EXPOSE 8000

# The entrypoint script will handle DB initialization/migrations before the main app starts.
ENTRYPOINT ["/app/entrypoint.sh"]

# The command that will be executed by the entrypoint script after it's done.
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000", "--reload"]
