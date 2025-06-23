# Pattern: The Generative Feedback Loop

This is a powerful, endgame pattern that enables systems to self-improve. It treats code and configuration not as static assets, but as artifacts that can be generated, tested, and refined in a continuous loop.

**Components:**

1.  **A Goal:** A high-level objective from a user or another system (e.g., "Write a Python function that calculates Fibonacci numbers.").
2.  **Creator Agent:** An agent with strong reasoning and code generation capabilities (`content-creator-agent-v2`, `api-writer-agent-v1`). Its toolset includes writing files.
3.  **Operator/Tester Agent:** An agent that can execute code or run tests (`code-linter-agent-v1`, `sysadmin-agent-v1`). Its toolset includes `system.execute`.
4.  **A Shared State/Context:** A location where the generated artifact and the test results can be stored and read by both agents (e.g., a file path, a cache key).

**Flow:**

1.  An orchestration receives the `goal`.
2.  It tasks the **Creator Agent** with `goal`: "Write the code for a Python function to solve this problem and save it to `/tmp/test.py`."
3.  The Creator Agent generates the code and saves the file.
4.  The orchestration then tasks the **Tester Agent** with `goal`: "Execute the tests located at `/tests/test_fibonacci.py` against the generated code at `/tmp/test.py`. Report the `stdout` and `stderr`."
5.  The Tester Agent runs the test using `system.execute` and returns the results.
6.  **The Loop:** The orchestration checks the test results.
    *   **If Successful:** The loop terminates. The final code is considered complete.
    *   **If Failed:** The orchestration re-invokes the **Creator Agent** with a new, refined `goal`: "Your previous attempt failed. Here are the test results: [stderr from previous step]. Please fix the code in `/tmp/test.py` and try again."
7.  The process repeats until the tests pass or a maximum number of iterations is reached.