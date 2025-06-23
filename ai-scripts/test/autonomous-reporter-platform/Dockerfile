FROM python:3.11-slim

WORKDIR /app

ENV PYTHONDONTWRITEBYTECODE 1
ENV PYTHONUNBUFFERED 1

COPY requirements.txt .
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

COPY ./app /app/app
COPY .env.example /app/.env.example

RUN mkdir -p /app/platform_storage/agent_data && \
    mkdir -p /app/platform_storage/reports_db && \
    chown -R www-data:www-data /app/platform_storage || true

USER www-data

EXPOSE 8010

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8010"]