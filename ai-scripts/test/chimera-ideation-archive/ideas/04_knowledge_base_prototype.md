# Prototype 4: Knowledge Base Integration

This concept transforms agents from stateless workers into stateful entities that can learn, recall, and use internal information. It introduces a formal knowledge layer.

```yaml
# --- KNOWLEDGE BASE DEFINITIONS ---
knowledge_bases:
  - id: 'company_product_docs'
    description: 'Official documentation for all company products.'
    type: 'vector_store' # Enables semantic search
    source: '/data/knowledge/product_docs/'

  - id: 'system_runtime_config'
    description: 'A key-value store for system runtime configuration.'
    type: 'key_value' # Enables direct key lookup
    source: '/config/runtime_constants.json'

# --- NEW TOOLS FOR KNOWLEDGE INTERACTION ---
tools:
  - id: 'knowledge_retriever'
    description: 'Queries a specific knowledge base to find relevant information.'

  - id: 'knowledge_updater'
    description: 'Adds or updates information in a write-enabled knowledge base.'

# --- KNOWLEDGE-AWARE ORCHESTRATION ---
orchestrations:
  - id: 'knowledge_assisted_content_pipeline'
    alias: 'Content Pipeline with Internal Knowledge'
    trigger:
      - platform: 'webhook'
        webhook_id: 'new-research-topic-webhook'
    action:
      - agent:
          goal: >
            A new topic has been submitted: '{{ trigger.json.topic }}'.
            1. First, query the 'company_product_docs' knowledge base.
            2. Then, use 'web_search' to augment this with public information.
            3. Synthesize all findings into a comprehensive blog post draft.
          
          # FEATURE: ATTACHING KNOWLEDGE
          # This agent can now access this specific knowledge base.
          knowledge:
            - 'company_product_docs'

          response_variable: 'draft_content'
```

### Key Concepts Introduced

*   **Knowledge Base Definition:** A formal declaration of a data source, its type (`vector_store` for semantic search on text, `key_value` for structured data), and permissions.
*   **Indexing:** A background process converts source material (like markdown files) into a queryable format.
*   **Attachment:** An agent can be granted access to specific knowledge bases via the `knowledge:` key, creating a secure context boundary.
*   **Interaction via Tools:** Agents use specific tools (`knowledge_retriever`, `knowledge_updater`) to interact with the knowledge bases, making their reasoning process explicit and auditable.
*   **Closed-Loop Learning:** An orchestration can process new information (e.g., summarize a meeting) and use the `knowledge_updater` tool to save that summary back into a knowledge base for future agents to use.