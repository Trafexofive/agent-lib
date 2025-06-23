# Prototype 3: The ATWO Model (Agents, Tools, Workflows, Orchestrations)

This prototype represents a major conceptual leap from simple automations to a structured, intelligent system. It introduces a hierarchy of components designed for maximum power and reusability.

```yaml
# --- SECTION 1: TOOL DEFINITIONS ---
tools:
  - id: 'web_search_tool'
    description: 'Searches the public web for a given query.'
    service: 'platform.web_search'

  - id: 'calendar_lookup_tool'
    description: "Looks up events on the user's calendar."
    service: 'platform.calendar.get_events'

# --- SECTION 2: WORKFLOW DEFINITIONS ---
workflows:
  - id: 'save_and_notify_workflow'
    alias: 'Save Content and Send Notification'
    inputs:
      project_name: 'The name of the project.'
      content_to_save: 'The text content to be saved.'
    sequence:
      - service: 'file.write'
        data:
          path: "/data/{{ inputs.project_name | slugify }}.md"
          content: "{{ inputs.content_to_save }}"
      - service: 'notify.send'
        data:
          message: "Report '{{ inputs.project_name }}' has been saved."

# --- SECTION 3: ORCHESTRATIONS ---
orchestrations:
  - id: 'daily_briefing_orchestrator'
    alias: 'Daily Executive Briefing'
    trigger:
      - platform: 'time'
        at: '07:00:00'
    action:
      # Step 1: DELEGATE TO AN AGENT
      - agent:
          goal: >
            Create a concise daily briefing. Check my calendar for today's events.
            Then, search the web for any major news related to the companies
            mentioned in my event titles. Combine this into a summary.
          available_tools:
            - 'calendar_lookup_tool'
            - 'web_search_tool'
          response_variable: 'agent_briefing_result'

      # Step 2: USE A REUSABLE WORKFLOW
      - workflow.run:
          id: 'save_and_notify_workflow'
          inputs:
            project_name: "Daily Briefing {{ now().strftime('%Y-%m-%d') }}"
            content_to_save: "{{ agent_briefing_result.content }}"
```

### Key Concepts Introduced

*   **Tools:** The primitive, stateless capabilities or "verbs" of the platform (e.g., `web_search`).
*   **Workflows:** Reusable, declarative sequences of actions. The "scripts" or "functions" of the platform.
*   **Agents:** Autonomous entities that use a set of available Tools to achieve a high-level `goal`.
*   **Orchestrations:** The main, event-driven processes that trigger and coordinate Agents and Workflows.