### Relic Setup: Generic Automation Platform (v1.1.1 Logging Fix)

**Diagnosis: A logical error was found in the `main.py` logging configuration. This targeted update fixes that specific code block.**

1.  **Apply the Fix**: This plan will overwrite only `gap_core/main.py` with the corrected version.

2.  **Restart the Service**: A simple restart is sufficient. No rebuild is needed as dependencies have not changed.

    ```bash
    make restart
    ```

3.  **VERIFY**: Check the logs. The `ValueError` will be gone, and you should now see the `OrchestrationEngine` firing every minute.

    ```bash
    make logs
    ```

    **Look for these specific log messages, which indicate success:**
    ```json
    {"message": "OrchestrationEngine started in background." ...}
    {"message": "TRIGGER FIRED: Orchestration 'Minute-by-Minute Engine Test' triggered by time pattern." ...}
    ```