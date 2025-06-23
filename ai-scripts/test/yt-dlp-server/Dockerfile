FROM python:3.11-slim

WORKDIR /app

ENV PYTHONDONTWRITEBYTECODE 1
ENV PYTHONUNBUFFERED 1

COPY requirements.txt .
# For production, consider multi-stage builds to exclude dev dependencies like pytest, flake8, black
RUN pip install --no-cache-dir -r requirements.txt

# COPY . . will copy the entire build context (including ./app subfolder) into /app
# So, host ./app/main.py becomes image /app/app/main.py
COPY . .

# Create storage directories expected by the application if they are not managed by volumes from compose
# RUN mkdir -p /app/storage/forged_relics /app/storage/forge_db

EXPOSE 8000

# The command `app.main:app` implies that `uvicorn` should find a module `app` (the directory),
# and within that, a file `main.py` containing an object `app`.
# This is correct given WORKDIR /app and COPY . . which places host ./app into /app/app in image.
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]