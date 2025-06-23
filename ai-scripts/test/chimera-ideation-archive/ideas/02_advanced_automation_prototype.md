# Prototype 2: Advanced `automation.yaml` (Feature Showcase)

This prototype was designed to showcase the full power and flexibility of the automation syntax, demonstrating advanced concepts within a single, complex workflow.

```yaml
# GENERIC AUTOMATION PLATFORM - FEATURE SHOWCASE PROTOTYPE

automations:

  - id: 'ultimate_morning_routine_showcase'
    alias: 'Feature Showcase: Ultimate Morning Routine'
    description: >
      A comprehensive morning routine that prepares the environment, fetches
      data from external APIs, and reacts dynamically to real-world events.

    # -- FEATURE: Execution Mode
    mode: restart

    # -- FEATURE: Multiple & Diverse Triggers (OR Logic)
    trigger:
      - platform: 'state'
        entity_id: 'binary_sensor.bedroom_motion'
        to: 'on'
      - platform: 'time'
        at: '06:30:00'
      - platform: 'webhook'
        webhook_id: 'my-secret-morning-webhook-id'

    # -- FEATURE: Complex & Nested Conditions (AND Logic)
    condition:
      - condition: 'template'
        value_template: "{{ now().isoweekday() in }}"
      - condition: 'numeric_state'
        entity_id: 'sensor.outside_temperature'
        above: 0

    # -- FEATURE: Action Sequence with Advanced Control Flow
    action:
      # FEATURE: Variables
      - variables:
          news_api_url: 'https://newsapi.org/v2/top-headlines?country=us&apiKey=YOUR_API_KEY'
          main_speaker: 'media_player.living_room_speaker'

      # FEATURE: HTTP Request & Response Variable
      - service: 'http.request'
        response_variable: 'news_data'
        data:
          url: "{{ news_api_url }}"

      # FEATURE: Parallel Execution
      - parallel:
          - service: 'light.turn_on'
            target:
              entity_id: 'light.kitchen_main_lights'
            data:
              brightness: 150
              transition: 10
          - service: 'switch.turn_on'
            target:
              entity_id: 'switch.coffee_machine'

      # FEATURE: Conditional Logic (If/Elif/Else) using 'choose'
      - choose:
          - conditions:
              - condition: 'template'
                value_template: "{{ news_data.status == 200 and news_data.json.articles|length > 0 }}"
            sequence:
              - service: 'tts.say'
                data:
                  target: "{{ main_speaker }}"
                  message: "Good morning. Today's top headline is: {{ news_data.json.articles.title }}"
        default:
          - service: 'tts.say'
            data:
              target: "{{ main_speaker }}"
              message: "Good morning. I was unable to fetch the news."

      # FEATURE: Dynamic Waiting using 'wait_for_trigger'
      - wait_for_trigger:
          - platform: 'numeric_state'
            entity_id: 'sensor.coffee_machine_power'
            above: 1000
        timeout: '00:05:00'
        continue_on_timeout: true

      # FEATURE: Looping / Repetition using 'repeat'
      - repeat:
          while:
            - condition: 'numeric_state'
              entity_id: 'sensor.coffee_machine_power'
              above: 50
          sequence:
            - delay: '00:00:10'

      # FEATURE: Final Notification using data from multiple steps
      - service: 'notify.mobile_app'
        data:
          title: "Morning Routine Complete"
          message: "Coffee is ready. Today's top headline was about '{{ news_data.json.articles.source.name }}'."
```