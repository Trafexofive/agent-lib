
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
    - CONTEXT the agent state, and the namespace variables... If we dont have this, we are not going to be able to do anything.
    - clean up the code and make it more readable.
    - clean up the repo and make it more team oriented.
    - full docker compose stack.
    - single modullar agent images.
    - swarm mode for docker (allowing live instantiation of single agent containers).


// INSPIRATIONS && IDEAS:
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

