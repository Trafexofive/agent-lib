FROM python:3.11-slim

WORKDIR /app

ENV PYTHONDONTWRITEBYTECODE 1
ENV PYTHONUNBUFFERED 1

COPY requirements.txt .
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

COPY ./app /app/app
COPY .env.example /app/.env.example # For reference, .env is used by compose

# Create directories for storage volumes within the image context
# These paths will be targeted by docker-compose volumes
RUN mkdir -p /app/persistent_storage/uploaded_assets && \
    mkdir -p /app/persistent_storage/registry_db && \
    chown -R www-data:www-data /app/persistent_storage || true # Allow failure if user is already www-data

USER www-data

EXPOSE 8009

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8009"]