# Prototype 1: Basic `automation.yaml`

This initial prototype established the core, human-readable structure for a simple automation. It introduced the fundamental concepts of triggers, conditions, and actions.

```yaml
# automation.yaml

automations:

  - id: 'notify_on_new_file'
    alias: 'Notify on New File'
    description: 'Sends a notification when a new file is created.'

    trigger:
      - platform: 'event'
        event_type: 'file.created'
        event_data:
          path: '/path/to/watch'

    condition:
      - condition: 'template'
        value_template: "{{ trigger.event.data.filename.endswith('.txt') }}"

    action:
      - service: 'notify.send'
        data:
          title: "New File: {{ trigger.event.data.filename }}"
          message: "A new file was created at {{ trigger.event.data.path }}."

  - id: 'turn_on_lights_at_sunset'
    alias: 'Turn on Lights at Sunset'
    description: 'Turns on the living room lights at sunset.'

    trigger:
      - platform: 'sun'
        event: 'sunset'
        offset: '-00:30:00'

    action:
      - service: 'light.turn_on'
        target:
          entity_id: 'light.living_room'
        data:
          brightness: 200
```

### Key Concepts Introduced

*   **`id`**: A unique machine-readable identifier.
*   **`alias`**: A human-friendly name for display.
*   **`description`**: Detailed explanation of the automation's purpose.
*   **`trigger`**: The event that initiates the automation (e.g., `event`, `sun`, `time`, `state`).
*   **`condition`**: An optional set of requirements that must be true for the action to run.
*   **`action`**: The task(s) the automation performs by calling a `service`.