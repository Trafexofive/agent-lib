FROM python:3.11-slim-bookworm

ENV PYTHONUNBUFFERED=1
ENV PYTHONPATH=/app

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . /app/aetherium_core/

CMD ["uvicorn", "aetherium_core.main:app", "--host", "0.0.0.0", "--port", "8000", "--reload", "--reload-dir", "/app/aetherium_core", "--reload-dir", "/app/workspace"]