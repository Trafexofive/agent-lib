FROM python:3.10-slim

WORKDIR /app

COPY requirements.txt requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

# Ensure the data directory exists and is writable if SQLite is used internally
RUN mkdir -p /app/data && chmod -R 777 /app/data

EXPOSE 8000

# For FastAPI with uvicorn
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]

# For Flask (if chosen)
# CMD ["flask", "run", "--host=0.0.0.0", "--port=8000"]
