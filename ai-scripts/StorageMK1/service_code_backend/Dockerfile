FROM python:3.11-slim-bookworm

ENV PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=off \
    PIP_DISABLE_PIP_VERSION_CHECK=on \
    PIP_DEFAULT_TIMEOUT=100

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

# Ensure the base directory /storage_data (or configured value) exists inside the container.
# This path will be where the host's STORAGE_ROOT_HOST_PATH is mounted.
# The main.py also tries to create it, but this is good practice.
RUN mkdir -p /storage_data

# Port exposed by Uvicorn INSIDE the container.
# This is fixed to 8000 here as per CMD. BACKEND_PORT_CONTAINER in .env should be 8000.
# The BACKEND_PORT_HOST in .env maps the host port to this container port 8000.
EXPOSE 8000

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
