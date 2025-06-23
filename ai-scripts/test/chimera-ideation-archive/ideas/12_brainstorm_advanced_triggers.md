# Brainstorm: Advanced Trigger Platforms

To truly "automate anything," the platform needs to react to a much wider variety of events. Here are some advanced trigger concepts.

### 1. Database Trigger

*   **Platform:** `database`
*   **Description:** Triggers when data in a connected database changes.
*   **Use Case:** When a new user signs up and a row is inserted into the `users` table, trigger a 'welcome-email' orchestration.
*   **YAML:**
    ```yaml
    trigger:
      - platform: 'database'
        connection_id: 'main_postgres_db' # ID of a configured DB connection
        schema: 'public'
        table: 'users'
        # Triggers on new rows.
        event: 'INSERT'
    ```

### 2. File System Trigger

*   **Platform:** `file.watcher`
*   **Description:** An advanced file watcher that triggers on creation, modification, or deletion of files in a directory.
*   **Use Case:** When a new `.mp4` file is dropped into `/media/uploads/new`, trigger a 'transcode-and-plex' orchestration.
*   **YAML:**
    ```yaml
    trigger:
      - platform: 'file.watcher'
        path: '/media/uploads/new'
        # Can be 'created', 'modified', 'deleted'.
        event: 'created'
        # Optional glob pattern to filter files.
        pattern: '*.mp4'
    ```

### 3. Machine Learning / Anomaly Detection Trigger

*   **Platform:** `anomaly`
*   **Description:** A conceptual trigger that integrates with a monitoring or ML system. It fires when an anomalous event is detected.
*   **Use Case:** When a Prometheus `AlertManager` webhook fires with a label `severity: anomaly`, trigger a 'sysadmin-investigation' agent.
*   **YAML:**
    ```yaml
    trigger:
      - platform: 'anomaly'
        source_id: 'prometheus_main'
        metric_name: 'cpu_usage_prediction_error'
        # The trigger fires when the value crosses a certain threshold.
        threshold: 0.9
    ```

### 4. Natural Language / Command Trigger

*   **Platform:** `command`
*   **Description:** Listens for natural language commands from a chat interface (like a Discord bot connected to the platform).
*   **Use Case:** A user types `@gap-bot remind me to take out the trash in 1 hour`. The platform parses this and triggers a 'set-reminder' orchestration.
*   **YAML:**
    ```yaml
    trigger:
      - platform: 'command'
        # The platform's NLU engine would match incoming text against these patterns.
        # This uses placeholders that become variables in the trigger data.
        patterns:
          - "remind me to {reminder_text} in {duration}"
          - "create a reminder for {reminder_text} at {time}"
    ```