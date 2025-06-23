# ChimeraDash Feature Roadmap

*Keep this document updated as features are implemented or planned.*

## I. Core Infrastructure & UI/UX

-   [X] **Dashboard Shell & Navigation**
    -   [X] Main `DashboardApp.tsx` structure
    -   [X] Sidebar Navigation (`SidebarNav.tsx`)
        -   [X] Dynamic item rendering
        -   [X] Active view highlighting
        -   [X] Collapsible sidebar state
        -   [X] Logo and Title display
    -   [X] Top Bar (`TopBar.tsx`)
        -   [X] Active view label
        -   [X] Notifications toggle
        -   [X] Command Palette toggle
    -   [X] Global Alert/Toast System (`AlertsContainer.tsx`)
-   [X] **Theming Engine**
    -   [X] CSS Variable-based theming
    -   [X] Default themes (Chimera Dark, Crimson Forge, Oceanic Abyss)
    -   [X] Theme switching capability
    -   [X] Custom theme creation and management (via Settings)
    -   [X] Font family customization
-   [X] **Command Palette (`CommandPalette.tsx`)**
    -   [X] Basic UI and invocation (Ctrl+K / Cmd+K)
    -   [X] Action listing and filtering
    -   [X] Navigation actions (Go to View)
    -   [X] Productivity Suite basic actions (Timer, Tasks)
    -   [ ] Advanced fuzzy search
    -   [ ] Contextual actions based on active view/item
    -   [ ] Vim-like navigation & advanced filtering
-   [X] **Notifications System (`NotificationsPanel.tsx`)**
    -   [X] Panel UI
    -   [X] Displaying notifications
    -   [X] Mark as read/unread
    -   [X] Dismiss individual/all
    -   [X] Basic deep linking from notifications
    -   [ ] Snooze functionality
    -   [ ] Custom sound options
    -   [ ] Enhanced deep linking to scroll/highlight specific items
-   [X] **Settings (`SettingsFeature.tsx`)**
    -   [X] General Settings (Title, Default View, Logo)
    -   [X] Appearance Settings (Theme selection, Font customization, Logo dimensions)
    -   [X] Custom Theme Editor
    -   [X] Sidebar Custom Link Management (Add/Delete URL items)
    -   [X] AI Configuration (Default models, Global System Instruction, Safety Settings, Thinking Mode)
    -   [X] Data Management (Export/Import settings, Clear specific data, Full Reset)
    -   [X] About section
-   [X] **State Management & Persistence**
    -   [X] localStorage for key settings (active view, sidebar state, themes, notifications, feature-specific data)
    -   [X] Robust parsing and default fallbacks for stored data.

## II. Core Features (Views)

### 1. OmniHoarder RAG Agent (`RAGAgentFeature.tsx`) - *Formerly OmniClip*

-   [X] **Item Aggregation & Management**
    -   [X] Add various item types (Text, Image, File, URL, Folder, Video URL, Audio File, RSS Feed URL)
    -   [X] File upload with content reading (text, dataURL for media)
    -   [X] Folder creation & basic description
    -   [X] OS Folder import (recursive file reading)
    -   [X] Item display (List & Card views)
    -   [X] Item deletion (including folder contents)
    -   [X] Item editing (name, content for applicable types)
    -   [X] Item detail view modal
    -   [X] Search/filter items (name, tags, content)
    -   [X] Basic folder navigation (breadcrumb-like in modals)
    -   [X] Download item content
-   [X] **Gemini AI Integration for RAG Items**
    -   [X] Core `geminiService.ts` for API calls
    -   [X] Automatic analysis queue for new/updated items
    -   [X] Default analyses (Summarize, Tag)
    -   [X] Type-specific analyses (Describe Image, Transcribe Audio, Get Video Info, Extract Code Structure, Analyze CSV, etc.)
    -   [X] Manual trigger for specific AI actions on items
    -   [X] Custom prompt modal for AI actions
    -   [X] Display AI analysis results in item details
    -   [X] Handling of analysis status (queued, processing, completed, failed)
-   [X] **Integrated AI Chat**
    -   [X] Chat interface within RAG Agent
    -   [X] Maintain chat history (localStorage)
    -   [X] Send messages to Gemini model
    -   [X] Display AI responses
    -   [X] Basic function calling for RAG items (retrieve_rag_items, get_rag_item_detail)
    -   [ ] Streaming responses
    -   [ ] More sophisticated context injection from RAG items
    -   [ ] User-configurable system prompt for chat

### 2. Productivity Suite (`ProductivitySuiteFeature.tsx`)

-   [X] **Unified Interface**
    -   [X] Tab-based navigation for sub-features (Timeline, Calendar, Tasks, Timer, Graphs, Search)
    -   [X] Deep linking to sub-views and items.
    -   [X] Command Palette integration for quick actions.
-   [X] **Timeline View**
    -   [X] Display upcoming events and scheduled tasks (next 7 days)
    -   [X] Chronological grouping by day
    -   [X] Clickable items to navigate to Calendar/Tasks
-   [X] **Calendar (`CalendarDaysIcon` related logic)**
    -   [X] Monthly calendar grid display
    -   [X] Navigation between months
    -   [X] Event creation and editing form
    -   [X] Support for all-day events, start/end times, descriptions, reminders
    -   [X] Display events on selected day
    -   [X] Delete events
    -   [X] Indicate days with events/tasks in grid
    -   [X] "Create Related Task" from event
    -   [ ] Recurring events
    -   [ ] ICS import/export
-   [X] **Tasks (`TasksFeature.tsx`)**
    -   [X] Add, complete, delete tasks
    -   [X] Task details: due date, due time, reminder, schedule date, priority
    -   [X] Subtask creation and management
    -   [X] Highlight navigated task item
    -   [X] Pre-fill new task from other features (e.g., Calendar)
    -   [X] Link to Calendar for due/scheduled dates
    -   [ ] Parent task auto-completion when all subtasks done
    -   [ ] Visual progress bar for parent tasks
-   [X] **Pomodoro Timer (`ClockIcon` related logic)**
    -   [X] Work, Short Break, Long Break modes
    -   [X] Configurable session durations (currently hardcoded, can be settings)
    -   [X] Visual timer progress
    -   [X] Audio notification on session end (basic beep)
    -   [X] Session logging
    -   [X] Link timer sessions to tasks
    -   [X] Create new task directly from timer
-   [X] **Graphs & Data View**
    -   [X] Task completion statistics (total, completed, pending, percentage)
    -   [X] Pomodoro session statistics (today, this week)
    -   [X] Basic bar chart for daily work sessions (last 7 days)
    -   [ ] More advanced charts (e.g., Pomodoro focus vs break time)
-   [X] **Search View**
    -   [X] Search across tasks and calendar events
    -   [X] Display search results with navigation links
    -   [ ] Advanced filtering and sorting for search results
    -   [ ] Extend search to other data sources (e.g., RAG items)

### 3. Automation Service (`AutomationServiceFeature.tsx`)

-   [X] **Core Automation Engine**
    -   [X] Create, edit, delete automations
    -   [X] Enable/disable automations
    -   [X] Automation list display
    -   [X] Execution log viewing (basic)
-   [X] **Triggers**
    -   [X] Manual Trigger
    -   [X] Time-Based Trigger (Cron expressions) - *Basic UI, execution logic not fully time-based yet*
    -   [X] Event-Based Trigger
        -   [X] RAG Agent: Item Added, Item Deleted, Analysis Complete
        -   [X] Productivity Suite: Task Completed, Calendar Event Starting
        -   [X] Event filtering options (item type, name, analysis type, etc.)
-   [X] **Actions**
    -   [X] Send Notification
    -   [X] Log Message (console)
    -   [X] Run RAG Analysis (on event item, specific item, latest by type)
    -   [X] Invoke Gemini Chat with RAG Context (basic text summary result)
    -   [X] Update RAG Item Analysis Field
    -   [X] Create Productivity Task
    -   [X] Complete Productivity Task
    -   [X] Create Calendar Event
    -   [X] Standard Utilities (Base64, URL Encode/Decode, UUID, JSON Format)
-   [X] **Execution & Context**
    -   [X] Manual run / Simulate Event & Run for event-based triggers
    -   [X] Basic context object (`{{now}}`, `{{automation}}`, `{{trigger.eventData}}`, `{{actionData}}`)
    -   [X] Placeholder replacement in action configurations
    -   [X] Relative date parsing for date fields in actions
    -   [ ] True background execution for Time-Based and Event-Based triggers (currently relies on app being open and manual/simulated runs)
    -   [ ] Action chaining / previous action result as input
    -   [ ] More robust error handling and retry mechanisms

### 4. Shortcuts (`ShortcutsFeature.tsx`)

-   [X] Display shortcuts as cards
-   [X] Open shortcut URLs in a new tab
-   [X] Default shortcuts provided
-   [X] Delete shortcuts (managed through UI directly on this view)
-   [X] Add new shortcuts (managed via Settings > Sidebar Links, as they are also sidebar items)
-   [ ] Custom icons for shortcuts
-   [ ] Grouping/categorization of shortcuts

### 5. Std::Utils (`StdUtilsFeature.tsx`)

-   [X] **Base64 Encoder/Decoder**
-   [X] **URL Encoder/Decoder**
-   [X] **Timestamp Converter** (Unix timestamp <=> Readable date)
-   [X] **JSON Formatter & Validator**
-   [X] **Text Inspector** (Char, Word, Line count)
-   [X] **UUID v4 Generator**
-   [ ] More utilities (e.g., Hash generator, Regex tester)

### 6. Std::Metrics (`StdMetricsFeature.tsx`)

-   [X] **Statistical Calculator** (Count, Sum, Mean, Median, Mode, Min, Max, Std Dev)
-   [X] **Data Size Converter** (Bytes, KB, MB, GB, TB)
-   [X] **Goal Tracker / Progress Bar**
-   [ ] More visual metric displays
-   [ ] Simple data visualization tools (e.g., generate basic charts from pasted data)

### 7. AI Tools (`AiDataProcessingFeature.tsx`)

-   [X] **Unified Interface for Standalone AI Tasks**
    -   [X] Sectioned layout for different AI tool categories
    -   [X] Input area (textarea/text/dual-textarea) for context/data
    -   [X] Output display area with copy-to-clipboard
    -   [X] Loading state indicators
    -   [X] Advanced options for custom system/user instructions per tool
-   [X] **Available AI Tools (leveraging `geminiService.ts`)**
    -   [X] Text Summarizer (`SUMMARIZE`)
    -   [X] Keyword Extractor (`EXTRACT_KEYWORDS`)
    -   [X] Sentiment Analyzer (`ANALYZE_SENTIMENT`)
    -   [X] Schema Extractor (`EXTRACT_SCHEMA`)
    -   [X] Template Generator (`GENERATE_TEMPLATE`)
    -   [X] Code Structure Analyzer (`EXTRACT_CODE_STRUCTURE`)
    -   [X] API Endpoint Identifier (`IDENTIFY_API_ENDPOINTS`)
    -   [X] Dependency Lister (`LIST_DEPENDENCIES`)
    -   [X] Comparative Text Summarizer (`COMPARATIVE_SUMMARIZE_TEXTS`)
    -   [X] Document Q&A (`DOCUMENT_QA`)
    -   [X] Writing Style & Tone Analyzer (`ANALYZE_STYLE_TONE`)
    -   [X] Story Idea Generator (`GENERATE_STORY_IDEAS`)
    -   [X] AI Marketing Copywriter (`GENERATE_MARKETING_COPY`)
    -   [X] Poem & Verse Generator (`GENERATE_POEM`)
    -   [X] CSV Data Profiler & Insights (`PROFILE_CSV_DATA_INSIGHTS`)
    -   [X] CSV to Chart Type Suggester (`SUGGEST_CSV_CHART_TYPE`)
    -   [X] JSON to CSV Converter (`CONVERT_JSON_TO_CSV`)
    -   [X] Webpage Change Detector (Conceptual) (`DETECT_WEBPAGE_CHANGES_CONCEPTUAL`)
    -   [X] URL Categorizer & Keyword Guesser (`CATEGORIZE_URL_KEYWORDS`)
    -   [X] HTML to Markdown Converter (`CONVERT_HTML_TO_MARKDOWN`)
    -   [X] Code Refactoring Suggester (`SUGGEST_CODE_REFACTORING`)
    -   [X] Unit Test Case Idea Generator (`GENERATE_UNIT_TEST_CASES`)
    -   [X] AI API Documentation Writer (`GENERATE_API_DOCUMENTATION`)
-   [ ] **Custom AI Tools & Prompt Chains** (Placeholder section exists)
    -   [ ] User interface for defining custom tools (selecting Gemini action, pre-defining system/user prompts)
    -   [ ] Interface for chaining multiple AI actions together
    -   [ ] Saving and managing user-defined tools/chains

## III. Future Enhancements / Vision (High-Level)

-   [ ] **True Background Automation**: Implement service worker or server-side component for automations that run even when the app is closed.
-   [ ] **Advanced RAG Capabilities**:
    -   [ ] Vector embeddings for semantic search within RAG items.
    -   [ ] More sophisticated context retrieval for AI Chat.
    -   [ ] Knowledge graph construction from RAG items.
-   [ ] **Real-time Data Feeds**: Integration with APIs, message queues, or other real-time sources for the RAG agent.
-   [ ] **Collaborative Features**: Multi-user support, shared vaults, and collaborative editing (long-term).
-   [ ] **Plugin System**: Allow for community or user-developed plugins/tools.
-   [ ] **Deeper OS Integration**: File system watching, native notifications (beyond browser).
-   [ ] **Enhanced Data Visualization**: More interactive and customizable charts and dashboards in Std::Metrics and Productivity Suite.
-   [ ] **Mobile Responsiveness & PWA**: Improve experience on smaller screens and offer PWA capabilities.
-   [ ] **Accessibility (A11y) Overhaul**: Systematic review and improvement of accessibility features.
-   [ ] **End-to-End Encryption** for sensitive data stored in the RAG.

This roadmap provides a snapshot. Specific tasks and priorities will evolve.
```