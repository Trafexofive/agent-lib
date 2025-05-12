
### B-line features
- B-line: having the base compose stack  fully working. backend and frontend (optional).
    - that would need: server to be refactored for the new lib changes.
    - fix the damn exec and parse. for the new schema
    - Implement Sub-agent loading, context an namespace variables/agent state, 
    - start building and testing the new agent profiles.
    - simple reporter live agent so we actually start to utilize the system.
    - Implement the first relic.
    - python/bash runtime.
    - script api.

{
  "id": "run_script",
  "type": "script",
  "params": {"runtime": "python", "source": "print('Hello')"}, // source could be a file path or inline code
  "description": "Runs a Python script to say hi",
  "tags": ["script", "test"],
  "timeout": 300,
  "retries": 3,
  "concurrency": 1,
  "schedule": "1h", // will run every hour. with the beginning of the hour as base
  "notifications": [
    {"event": "completion", "channel": "email", "recipients": ["user@example.com"]},
    {"event": "failure", "channel": "slack", "webhook": "https://slack.com/webhook"}
  ],
  "resource_limits": {
    "cpu": "0.5",
    "memory": "512MB",
    "disk": "1GB"
  },


  {
    status: string (REQUIRED, enum: SUCCESS| REQUIRES_ACTION | FAIL | ERROR),
    thoughts: array<object> (REQUIRED, object: { type: string (REQUIRED, enum: DECISION| LONG_TERM | SHORT_TERM| PLAN | OBSERVATION | QUESTION | NORM | ASSUMPTION | GOAL | HYPOTHESIS | CRITIQUE), content: string (REQUIRED) }),
    actions: array<object> (REQUIRED, object: { action: string (REQUIRED), type: string (REQUIRED, enum: tool | script | http_request | internal_function | output | workflow_control), params: object (REQUIRED, structure depends on action/type), confidence?: float (0.0-1.0), warnings?: array<string> }),
    response: string | null

  }


{
  "action": "run_script",
  "type": "script",
  "params": {"language": "python", "code": "print('Hello')"},
  "description": "Runs a Python script to say hi",
  "tags": ["script", "test"],
  "priority": 3,
  "dependencies": ["setup_env"],
  "version": "1.0.0",
  "timeout": 300,
  "retries": 3,
  "concurrency": 1,
  "schedule": "cron(0 0 * * *)",
  "notifications": [
    {"event": "completion", "channel": "email", "recipients": ["user@example.com"]},
    {"event": "failure", "channel": "slack", "webhook": "https://slack.com/webhook"}
  ],
  "env_vars": {"DEBUG": "true"},
  "input_schema": {
    "type": "object",
    "properties": {"language": {"type": "string"}, "code": {"type": "string"}},
    "required": ["language", "code"]
  },
  "output_schema": {
    "type": "object",
    "properties": {"result": {"type": "string"}},
    "required": ["result"]
  },
  "log_level": "DEBUG",
  "context": {
    "type": "docker",
    "image": "python:3.9",
    "options": {"network": "host"}
  },
  "preconditions": [
    {"type": "file_exists", "path": "/tmp/setup.done"}
  ],
  "postconditions": [
    {"type": "file_exists", "path": "/tmp/output.txt"}
  ],
  "failure_policy": {
    "on_failure": "rollback",
    "rollback_action": "cleanup_script"
  },
  "metadata": {
    "owner": "team-alpha",
    "created": "2023-10-01"
  },
  "rate_limit": {
    "max_runs": 10,
    "per": "hour"
  },
  "secrets": {
    "api_token": "vault://secrets/api/token"
  }
}

