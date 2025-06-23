# Pattern: Request-Enrich-Respond

This is one of the most common and powerful patterns for agents. It's used for any task that involves answering a question or fulfilling a request with information from multiple sources.

**Components:**

1.  **Trigger:** A `command` or `webhook` that contains an initial query or request.
2.  **Agent:** A reasoning agent (`researcher-agent-v1`, `assistant-agent-v1`).
3.  **Tools:** `knowledge_retriever` (for internal context), `web_search` (for external context), and `http_request` (for specific APIs).
4.  **Action:** A `notify` service to deliver the final, synthesized answer.

**Flow:**

1.  A user asks, `@gap-bot what is project chimera?`
2.  The `command` trigger fires.
3.  An orchestration tasks the `assistant-agent` with the `goal`: "Answer the user's question: 'what is project chimera?'"
4.  The agent's reasoning loop begins:
    a.  **Thought:** "'Project Chimera' sounds like an internal codename. I should check internal knowledge first."
    b.  **Action:** Calls `knowledge_retriever` with `query: 'project chimera'`. Gets back internal documents.
    c.  **Thought:** "I have the internal definition. Now I'll check the web to see if there's any public information or conflicting definitions."
    d.  **Action:** Calls `web_search` with `query: '"Project Chimera" tech'`. Gets back public articles.
    e.  **Thought:** "I have all the information. I will now synthesize a comprehensive answer combining both internal and public data."
5.  The agent formulates its final answer and returns it.
6.  The orchestration takes the agent's final answer and sends it back to the user via a `notify` service.
