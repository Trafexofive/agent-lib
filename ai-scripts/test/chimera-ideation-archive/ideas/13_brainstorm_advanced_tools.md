# Brainstorm: Advanced Tool Concepts

Beyond simple `log.write` or `http.request`, a truly powerful platform needs a richer set of built-in tools.

### 1. `shell.execute`

*   **Name:** `shell.execute`
*   **Description:** Executes an arbitrary shell command on the host machine where the GAP backend is running. **Extremely powerful and dangerous; should be used with caution.**
*   **Use Case:** Running a custom backup script, a git command, or any command-line utility.
*   **Parameters:** `command` (the command string), `cwd` (current working directory), `timeout`.
*   **Returns:** `stdout`, `stderr`, `return_code`.

### 2. `image.generate`

*   **Name:** `image.generate`
*   **Description:** Uses a text-to-image model (like Stable Diffusion) to generate an image from a prompt.
*   **Use Case:** An orchestration that automatically generates a blog post could also generate a relevant header image for it.
*   **Parameters:** `prompt`, `negative_prompt`, `width`, `height`, `style`.
*   **Returns:** A path to the saved image file or a base64-encoded string.

### 3. `file.transform`

*   **Name:** `file.transform`
*   **Description:** A tool for basic file manipulations and format conversions.
*   **Use Case:** Converting a CSV file to JSON, resizing an image, or extracting text from a PDF.
*   **Parameters:** `source_path`, `destination_path`, `operation` (e.g., 'csv_to_json', 'image_resize'), `options` (e.g., `{'width': 800}`).
*   **Returns:** A status object.

### 4. `human.request_input`

*   **Name:** `human.request_input`
*   **Description:** A more advanced version of `request_approval`. It can ask the user for arbitrary input, not just an approve/deny choice.
*   **Use Case:** An agent planning a trip could use this tool to ask the user, "What is your budget for the flight?" and wait for a response.
*   **Parameters:** `prompt`, `input_type` (e.g., 'text', 'number', 'choice'), `choices` (if applicable).
*   **Returns:** The data provided by the user.

### 5. `vector_db.query`

*   **Name:** `vector_db.query` (lower level than `knowledge_retriever`)
*   **Description:** Performs a raw vector similarity search against a specific vector database collection.
*   **Use Case:** An agent trying to find *conceptually similar* past incidents, even if the keywords don't match, would use this for deep analysis.
*   **Parameters:** `collection_name`, `query_vector` (or `query_text` to be vectorized), `top_k`.
*   **Returns:** A list of documents with their similarity scores.