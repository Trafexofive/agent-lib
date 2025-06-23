


export enum ItemType {
  TEXT = 'TEXT',
  IMAGE = 'IMAGE',
  GENERIC_FILE = 'GENERIC_FILE',
  FOLDER = 'FOLDER',
  VIDEO_URL = 'VIDEO_URL',
  AUDIO_FILE = 'AUDIO_FILE',
  RSS_FEED_URL = 'RSS_FEED_URL',
}

export enum GeminiActionType {
  SUMMARIZE = 'SUMMARIZE',
  DESCRIBE = 'DESCRIBE',
  TAG = 'TAG',
  EXTRACT_KEYWORDS = 'EXTRACT_KEYWORDS',
  ANALYZE_SENTIMENT = 'ANALYZE_SENTIMENT',
  EXTRACT_SCHEMA = 'EXTRACT_SCHEMA', 
  GENERATE_TEMPLATE = 'GENERATE_TEMPLATE',
  EXTRACT_CODE_STRUCTURE = 'EXTRACT_CODE_STRUCTURE',
  IDENTIFY_API_ENDPOINTS = 'IDENTIFY_API_ENDPOINTS',
  LIST_DEPENDENCIES = 'LIST_DEPENDENCIES',
  TRANSCRIBE_AUDIO = 'TRANSCRIBE_AUDIO',
  SUMMARIZE_WEBPAGE = 'SUMMARIZE_WEBPAGE', // For URL-based summary without fetching
  GET_VIDEO_INFO = 'GET_VIDEO_INFO', // For URL-based info without fetching
  GUESS_RSS_FEED_TOPICS = 'GUESS_RSS_FEED_TOPICS', // For URL-based guess without fetching
  // CSV Specific Actions
  ANALYZE_CSV_DATA = 'ANALYZE_CSV_DATA',
  CONVERT_CSV_TO_JSON = 'CONVERT_CSV_TO_JSON',
  CONVERT_CSV_TO_MARKDOWN_TABLE = 'CONVERT_CSV_TO_MARKDOWN_TABLE',
  // Web Scraping & Analysis Actions (content is fetched HTML)
  SCRAPE_WEB_SIMPLE = 'SCRAPE_WEB_SIMPLE', // Simple text extraction / summary from fetched HTML
  SCRAPE_WEB_FULL_TEXT = 'SCRAPE_WEB_FULL_TEXT', // Comprehensive text extraction from fetched HTML
  ANALYZE_WEB_DOCUMENTATION = 'ANALYZE_WEB_DOCUMENTATION', // Focused analysis of a fetched documentation page's HTML

  // Section 1: Advanced Text & Document Processing
  COMPARATIVE_SUMMARIZE_TEXTS = 'COMPARATIVE_SUMMARIZE_TEXTS',
  DOCUMENT_QA = 'DOCUMENT_QA',
  ANALYZE_STYLE_TONE = 'ANALYZE_STYLE_TONE',

  // Section 2: Creative Content Generation
  GENERATE_STORY_IDEAS = 'GENERATE_STORY_IDEAS',
  GENERATE_MARKETING_COPY = 'GENERATE_MARKETING_COPY',
  GENERATE_POEM = 'GENERATE_POEM',

  // Section 3: Data & CSV Enhanced Tools
  PROFILE_CSV_DATA_INSIGHTS = 'PROFILE_CSV_DATA_INSIGHTS',
  SUGGEST_CSV_CHART_TYPE = 'SUGGEST_CSV_CHART_TYPE',
  CONVERT_JSON_TO_CSV = 'CONVERT_JSON_TO_CSV',

  // Section 4: Web & URL Intelligence
  DETECT_WEBPAGE_CHANGES_CONCEPTUAL = 'DETECT_WEBPAGE_CHANGES_CONCEPTUAL',
  CATEGORIZE_URL_KEYWORDS = 'CATEGORIZE_URL_KEYWORDS',
  CONVERT_HTML_TO_MARKDOWN = 'CONVERT_HTML_TO_MARKDOWN',

  // Section 5: Advanced Code & Development Aids
  SUGGEST_CODE_REFACTORING = 'SUGGEST_CODE_REFACTORING',
  GENERATE_UNIT_TEST_CASES = 'GENERATE_UNIT_TEST_CASES',
  GENERATE_API_DOCUMENTATION = 'GENERATE_API_DOCUMENTATION',
}

export interface AggregatedItem {
  id: string;
  type: ItemType;
  name:string;
  content: string; 
  originalMimeType?: string; 
  geminiAnalysis?: {
    [key in GeminiActionType | string]?: string | string[] | { status?: 'queued' | 'processing' | 'completed' | 'failed'; error?: string; };
  };
  createdAt: Date;
  parentId?: string;
  fileSize?: number;
  // Temporary properties for passing context to analysis functions
  _actionToRun?: GeminiActionType;
  _customPromptForAction?: string;
  _fullContentForAnalysis?: string; // Added for storing full content before potential truncation
}

export interface AlertMessage {
  id: string;
  type: 'success' | 'error' | 'info';
  message: string;
}

export interface Shortcut {
  id: string;
  name: string;
  url: string;
  icon?: React.ElementType; 
  description?: string;
}

export interface TaskItem {
  id: string;
  text: string;
  completed: boolean;
  createdAt: Date; 
  parentId?: string; 
  dueDate?: string; 
  dueTime?: string; 
  reminderMinutesBefore?: number; 
  scheduledDate?: string; 
  priority?: number; 
}

export enum ViewType {
  NEXUS_HUB = 'nexus_hub', 
  RAG_AGENT = 'rag_agent',
  PRODUCTIVITY_SUITE = 'productivity_suite',
  AUTOMATION_SERVICE = 'automation_service',
  SHORTCUTS = 'shortcuts',
  STD_UTILS = 'std_utils',
  STD_METRICS = 'std_metrics',
  AI_DATA_PROCESSING = 'ai_data_processing',
  SETTINGS = 'settings',
  KNOWLEDGE_GRAPH_EXPLORER = 'knowledge_graph_explorer',
  SYSTEM_MONITOR = 'system_monitor',
  FINANCIAL_TRACKER = 'financial_tracker',
  CONTENT_PLANNER = 'content_planner',
  SECURITY_CENTER = 'security_center',
}

export type TimerMode = 'WORK' | 'SHORT_BREAK' | 'LONG_BREAK';

export interface PomodoroSession {
  id: string;
  date: string; 
  type: TimerMode;
  duration: number; 
}

export interface CalendarEvent {
  id: string;
  title: string;
  date: string; 
  startTime: string; 
  endTime: string; 
  description: string;
  allDay: boolean;
  createdAt: string; 
  reminderMinutesBefore?: number; 
  attendees?: string[]; 
}

export enum ProductivitySubViewType {
  TIMER = 'TIMER',
  TASKS = 'TASKS',
  CALENDAR = 'CALENDAR',
  GRAPHS_DATA = 'GRAPHS_DATA',
  SEARCH = 'SEARCH',
  TIMELINE = 'TIMELINE',
}

export type IconName =
  | 'ClipboardIcon' | 'LinkIcon' | 'ClockIcon' | 'ClipboardDocumentCheckIcon'
  | 'WrenchScrewdriverIcon' | 'ChartPieIcon' | 'SparklesIcon' | 'CogIcon'
  | 'ExternalLinkIcon' | 'RectangleStackIcon' | 'ListBulletIcon' | 'MagnifyingGlassIcon'
  | 'TableCellsIcon' | 'Squares2X2Icon' | 'ChatBubbleLeftRightIcon' | 'BoltIcon'
  | 'CalendarDaysIcon' | 'AdjustmentsHorizontalIcon' | 'CodeBracketSquareIcon' | 'DocumentTextIcon'
  | 'CpuChipIcon' | 'PlayCircleIcon' | 'BellAlertIcon' | 'ArchiveBoxArrowDownIcon'
  | 'InboxStackIcon' | 'ArrowPathIcon' | 'KeyIcon' | 'VariableIcon' | 'ShareIcon'
  | 'Bars3Icon' | 'XMarkIcon' | 'PhotoIcon'
  | 'ChevronDownIcon' | 'ChevronUpIcon' | 'ChevronRightIcon' | 'CommandIcon'
  | 'BanknotesIcon' | 'MapIcon' | 'ShieldCheckIcon'
  | 'ServerStackIcon' | 'InformationCircleIcon' | 'PaintBrushIcon' | 'Cog8ToothIcon';


export interface SidebarNavItem {
  id: string;
  label: string;
  type: 'VIEW' | 'URL';
  view?: ViewType;
  url?: string;
  iconName: IconName;
}

export interface NotificationItem {
  id: string;
  type: 'success' | 'error' | 'info' | 'reminder';
  message: string;
  timestamp: string; 
  read: boolean;
  sourceView?: ViewType;
  sourceSubView?: ProductivitySubViewType;
  sourceItemId?: string;
}

export interface CommandPaletteAction {
  id: string;
  label: string;
  type: 'navigate' | 'action'; 
  iconName?: IconName; 
  keywords?: string[];
  section?: string; 
  actionDetail?: {
    view?: ViewType; 
    url?: string; 
    subView?: ProductivitySubViewType; 
    subViewAction?: string; 
    payload?: any; 
    itemId?: string; 
  };
}

export interface GeminiFunctionCall { name: string; args: Record<string, any>; }
export interface GeminiPart {
  text?: string;
  inlineData?: { mimeType?: string; data?: string; };
  functionCall?: GeminiFunctionCall;
  functionResponse?: { name: string; response: Record<string, any>; };
}
export interface GeminiContent { role: 'user' | 'model' | 'function'; parts: GeminiPart[]; }
export interface ChatMessage {
  id: string;
  sender: 'user' | 'ai' | 'system';
  text?: string;
  timestamp: string;
  functionCall?: GeminiFunctionCall;
  functionResponse?: { name: string, response: any, isError?: boolean };
  isLoading?: boolean;
}

export enum AutomationTriggerType {
  MANUAL = 'MANUAL',
  TIME_BASED = 'TIME_BASED', 
  EVENT_BASED = 'EVENT_BASED',
}

export enum AutomationEventSource { 
  RAG_AGENT_NEW_ITEM_ADDED = 'RAG_AGENT_NEW_ITEM_ADDED',
  RAG_AGENT_ANALYSIS_COMPLETE = 'RAG_AGENT_ANALYSIS_COMPLETE',
  RAG_AGENT_ITEM_DELETED = 'RAG_AGENT_ITEM_DELETED',
  PRODUCTIVITY_SUITE_TASK_COMPLETED = 'PRODUCTIVITY_SUITE_TASK_COMPLETED',
  PRODUCTIVITY_SUITE_CALENDAR_EVENT_STARTING = 'PRODUCTIVITY_SUITE_CALENDAR_EVENT_STARTING',
}

export enum AutomationActionType {
  SEND_NOTIFICATION = 'SEND_NOTIFICATION',
  LOG_MESSAGE = 'LOG_MESSAGE',
  RUN_RAG_ANALYSIS = 'RUN_RAG_ANALYSIS',
  INVOKE_GEMINI_CHAT_WITH_RAG_CONTEXT = 'INVOKE_GEMINI_CHAT_WITH_RAG_CONTEXT', 
  UPDATE_RAG_ITEM_ANALYSIS = 'UPDATE_RAG_ITEM_ANALYSIS', 
  CREATE_PRODUCTIVITY_TASK = 'CREATE_PRODUCTIVITY_TASK',
  COMPLETE_PRODUCTIVITY_TASK = 'COMPLETE_PRODUCTIVITY_TASK',
  CREATE_CALENDAR_EVENT = 'CREATE_CALENDAR_EVENT',
  STD_UTIL_BASE64_ENCODE = 'STD_UTIL_BASE64_ENCODE',
  STD_UTIL_BASE64_DECODE = 'STD_UTIL_BASE64_DECODE',
  STD_UTIL_URL_ENCODE = 'STD_UTIL_URL_ENCODE',
  STD_UTIL_URL_DECODE = 'STD_UTIL_URL_DECODE',
  STD_UTIL_GENERATE_UUID = 'STD_UTIL_GENERATE_UUID',
  STD_UTIL_JSON_FORMAT = 'STD_UTIL_JSON_FORMAT',
}

export interface AutomationTriggerConfig {
  cronExpression?: string; // For TIME_BASED
  eventSource?: AutomationEventSource; // For EVENT_BASED
  // RAG Event Filters
  eventItemTypeFilter?: ItemType[]; 
  eventNameFilter?: string; // For RAG_ITEM_ADDED, RAG_ITEM_DELETED, RAG_ANALYSIS_COMPLETE (Added to RAG_ANALYSIS_COMPLETE)
  eventAnalysisTypeFilter?: GeminiActionType; // For RAG_ANALYSIS_COMPLETE
  // Productivity Task Event Filters
  taskNameContainsFilter?: string; 
  taskHasDueDateFilter?: boolean; // Added
  taskPriorityFilter?: number; // Added
  // Productivity Calendar Event Filters
  minutesBeforeToTrigger?: number; 
  eventTitleContainsFilter?: string; 
  eventHasAttendeesFilter?: boolean; // Added
}

export interface AutomationActionConfig {
  // Common
  actionOutputKey?: string; // Key to store this action's output in context.actionData
  // SEND_NOTIFICATION
  notificationMessage?: string; 
  notificationType?: NotificationItem['type'];
  // LOG_MESSAGE
  logMessage?: string; 
  // RUN_RAG_ANALYSIS
  ragAnalysisTargetMode?: 'EVENT_TRIGGER_ITEM' | 'SPECIFIC_ITEM_ID' | 'LATEST_ITEM_BY_TYPE';
  ragAnalysisTargetItemId?: string; 
  ragAnalysisTargetItemType?: ItemType; 
  ragAnalysisType?: GeminiActionType;
  ragAnalysisCustomPrompt?: string; 
  // INVOKE_GEMINI_CHAT_WITH_RAG_CONTEXT
  chatPromptTemplate?: string; 
  chatContextMode?: 'NONE' | 'EVENT_TRIGGER_ITEM_CONTENT' | 'ITEM_ID_CONTENT' | 'CUSTOM_TEXT'; 
  chatContextItemId?: string; 
  chatCustomContextText?: string; 
  // UPDATE_RAG_ITEM_ANALYSIS
  updateTargetItemIdMode?: 'EVENT_TRIGGER_ITEM' | 'SPECIFIC_ITEM_ID'; // Added
  updateTargetItemId?: string;
  analysisKeyToUpdate?: string; 
  newValueMode?: 'STATIC_TEXT' | 'PREVIOUS_ACTION_RESULT'; // Added
  staticNewValue?: string | string[]; 
  previousActionResultKey?: string; // Added: Key from context.actionData
  // CREATE_PRODUCTIVITY_TASK
  taskText?: string;
  taskDueDate?: string; // YYYY-MM-DD or relative (e.g., +7d, {{now.date}})
  taskDueTime?: string; // HH:MM
  taskReminderMinutesBefore?: number; 
  taskScheduledDate?: string; // YYYY-MM-DD or relative
  taskPriority?: number; 
  // COMPLETE_PRODUCTIVITY_TASK
  completeTaskId?: string; 
  completeTaskNameQuery?: string; // Find task by name (contains)
  // CREATE_CALENDAR_EVENT
  eventTitle?: string;
  eventDate?: string; // YYYY-MM-DD or relative
  eventStartTime?: string; // HH:MM
  eventEndTime?: string; // HH:MM
  eventDescription?: string;
  eventAllDay?: boolean;
  eventReminderMinutesBeforeForCreate?: number; // Specific for this action (Added)
  // STD_UTIL Actions
  stdUtilInputString?: string;
  stdUtilJsonIndentation?: 2 | 4 | 'tab'; 
}

export interface ExecutionLogEntry {
  id: string;
  timestamp: string; 
  status: 'success' | 'failure';
  triggerType: AutomationTriggerType; 
  actionType: AutomationActionType; 
  summaryMessage: string; 
  eventDataSnapshot?: Record<string, any>; 
  actionInputSnapshot?: Record<string, any>; 
  actionOutputSnapshot?: any; 
}

export interface Automation {
  id: string;
  name: string;
  description?: string;
  isEnabled: boolean;
  trigger: {
    type: AutomationTriggerType;
    config: AutomationTriggerConfig; 
  };
  action: { 
    type: AutomationActionType;
    config: AutomationActionConfig;
  };
  lastRun?: string; 
  createdAt: string; 
  runCount: number;
  executionLogs: ExecutionLogEntry[]; 
}

export interface AutomationRunContext {
  trigger: {
    type: AutomationTriggerType;
    eventSource?: AutomationEventSource;
    eventData?: Record<string, any>; // Data specific to the event that triggered the automation
  };
  automation: {
    id: string;
    name: string;
  };
  now: { // Information about the current time of execution
    iso: string; // Full ISO string
    date: string; // YYYY-MM-DD
    time: string; // HH:MM:SS
    unix: number; // Unix timestamp (seconds)
  };
  actionData?: Record<string, any>; // To store results from previous actions in a chain
}

export interface ThemeColors {
  bgPrimary: string;
  bgSecondary: string;
  bgAccent: string;
  textPrimary: string;
  textSecondary: string;
  accentPrimary: string;
  accentSecondary: string;
  borderPrimary: string;
  success: string;
  error: string;
  info: string;
  buttonPrimaryBg?: string;
  buttonPrimaryText?: string;
  sidebarBg?: string;
  sidebarText?: string;
  sidebarAccent?: string;
  sidebarFontIconSize?: string; 
  sidebarFontItemTextSize?: string; 
  topbarBg?: string;
  cardBg?: string;
  modalBg?: string;
  inputBg?: string;
  inputBorder?: string;
  inputText?: string;
  // JSON Syntax Highlighting
  jsonKey?: string;
  jsonString?: string;
  jsonNumber?: string;
  jsonBoolean?: string;
  jsonNull?: string;
  jsonBrackets?: string; 
  jsonComma?: string;
  jsonColon?: string;
}

export interface Theme {
  name: string;
  colors: ThemeColors;
}

// Enums for AI Safety Settings - using app's own for storage and UI
export enum HarmCategory {
  HARM_CATEGORY_UNSPECIFIED = "HARM_CATEGORY_UNSPECIFIED",
  HARM_CATEGORY_DEROGATORY = "HARM_CATEGORY_DEROGATORY",
  HARM_CATEGORY_TOXICITY = "HARM_CATEGORY_TOXICITY",
  HARM_CATEGORY_VIOLENCE = "HARM_CATEGORY_VIOLENCE",
  HARM_CATEGORY_SEXUAL = "HARM_CATEGORY_SEXUAL",
  HARM_CATEGORY_MEDICAL = "HARM_CATEGORY_MEDICAL",
  HARM_CATEGORY_DANGEROUS = "HARM_CATEGORY_DANGEROUS",
  HARM_CATEGORY_HARASSMENT = "HARM_CATEGORY_HARASSMENT",
  HARM_CATEGORY_HATE_SPEECH = "HARM_CATEGORY_HATE_SPEECH",
  HARM_CATEGORY_SEXUALLY_EXPLICIT = "HARM_CATEGORY_SEXUALLY_EXPLICIT",
  HARM_CATEGORY_DANGEROUS_CONTENT = "HARM_CATEGORY_DANGEROUS_CONTENT",
}

export enum HarmBlockThreshold {
  HARM_BLOCK_THRESHOLD_UNSPECIFIED = "HARM_BLOCK_THRESHOLD_UNSPECIFIED",
  BLOCK_LOW_AND_ABOVE = "BLOCK_LOW_AND_ABOVE",
  BLOCK_MEDIUM_AND_ABOVE = "BLOCK_MEDIUM_AND_ABOVE",
  BLOCK_ONLY_HIGH = "BLOCK_ONLY_HIGH",
  BLOCK_NONE = "BLOCK_NONE",
}

export interface SafetySettingItem { 
  category: HarmCategory;
  threshold: HarmBlockThreshold;
}

export interface DashboardSettings {
  dashboardTitle: string;
  logoBase64: string | null;
  showLogo: boolean;
  currentThemeName: string;
  fontFamilyBody: string; 
  fontFamilyTitle: string; 
  logoMaxWidth?: number; 
  logoMaxHeight?: number; 
  customThemes?: Theme[]; 
  recentViews?: ViewType[];
  defaultViewOnStartup?: ViewType | null;
  // AI Global Settings
  defaultTextModel?: string;
  defaultImageModel?: string;
  globalSystemInstruction?: string;
  aiSafetySettings: SafetySettingItem[]; 
  aiThinkingEnabled: boolean; 
}

// For exporting/importing all settings
export interface FullExportedSettings {
  dashboardSettings: DashboardSettings;
  sidebarNavItems: SidebarNavItem[]; // Only custom URL items
}

// Props for ProductivitySuiteFeature specific deep linking and prefill
export interface ProductivitySuiteDeepLinkProps {
    deepLinkSubView?: ProductivitySubViewType;
    deepLinkItemId?: string;
}

export interface ProductivityTaskPrefillData {
    text: string;
    scheduledDate?: string;
    dueDate?: string;
    // any other relevant TaskItem fields for prefill
}

// For exporting/importing Automations
export interface FullExportedAutomations {
  automations: Automation[];
}

// For exporting/importing RAG Items
export interface FullExportedRAGItems {
  ragItems: AggregatedItem[];
}