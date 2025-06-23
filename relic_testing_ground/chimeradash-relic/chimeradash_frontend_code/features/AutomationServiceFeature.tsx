
import React, { useState, useEffect, useCallback, ChangeEvent } from 'react';
import { 
    Automation, AutomationTriggerType, AutomationActionType, 
    NotificationItem, ViewType, GeminiActionType,
    ExecutionLogEntry, ItemType, AggregatedItem, AutomationEventSource,
    TaskItem, CalendarEvent, AutomationRunContext,
    AutomationTriggerConfig, AutomationActionConfig, FullExportedAutomations
} from '../types';
import Spinner from '../components/Spinner';
import { analyzeWithGemini } from '../services/geminiService';

// --- Internal Icons ---
const BoltIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 13.5l10.5-11.25L12 10.5h8.25L9.75 21.75 12 13.5H3.75z" /></svg>;
const PlusIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 4.5v15m7.5-7.5h-15" /></svg>;
const TrashIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12.56 0c1.153 0 2.24.032 3.223.094M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const PencilIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M16.862 4.487l1.687-1.688a1.875 1.875 0 112.652 2.652L10.582 16.07a4.5 4.5 0 01-1.897 1.13L6 18l.8-2.685a4.5 4.5 0 011.13-1.897l8.932-8.931zm0 0L19.5 7.125M18 14v4.75A2.25 2.25 0 0115.75 21H5.25A2.25 2.25 0 013 18.75V8.25A2.25 2.25 0 015.25 6H10" /></svg>;
const PlayCircleIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /><path strokeLinecap="round" strokeLinejoin="round" d="M15.91 11.672a.375.375 0 010 .656l-5.603 3.113a.375.375 0 01-.557-.328V8.887c0-.286.307-.466.557-.327l5.603 3.112z" /></svg>;
const PowerIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M5.636 5.636a9 9 0 1012.728 0M12 3v9" /></svg>;
const ClockIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const CalendarDaysIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6.75 3v2.25M17.25 3v2.25M3 18.75V7.5a2.25 2.25 0 012.25-2.25h13.5A2.25 2.25 0 0121 7.5v11.25m-18 0A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75m-18 0v-7.5A2.25 2.25 0 015.25 9h13.5A2.25 2.25 0 0121 11.25v7.5" /></svg>;
const BellAlertIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.857 17.082a23.848 23.848 0 005.454-1.31A8.967 8.967 0 0118 9.75v-.7V9A6 6 0 006 9v.75a8.967 8.967 0 01-2.312 6.022c1.733.64 3.56 1.017 5.454 1.31m5.714 0a24.255 24.255 0 01-5.714 0m5.714 0a3 3 0 11-5.714 0M12 18.75a9.75 9.75 0 007.242-3.346" /></svg>;
const DocumentTextIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M19.5 14.25v-2.625a3.375 3.375 0 00-3.375-3.375h-1.5A1.125 1.125 0 0113.5 7.125v-1.5a3.375 3.375 0 00-3.375-3.375H8.25m.75 12h6M8.25 15h6m4.5-7.5V21a2.25 2.25 0 01-2.25 2.25H5.25A2.25 2.25 0 013 21V5.25A2.25 2.25 0 015.25 3h4.5M15 3.75V7.5h3.75" /></svg>;
const CpuChipIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 3v1.5M4.5 8.25H3m18 0h-1.5M4.5 12H3m18 0h-1.5m-15 .75h.008v.008H4.5v-.008zm0 2.25h.008v.008H4.5v-.008zM8.25 15V21H3V15h5.25zm7.5 0v5.25H21V15h-5.25zM8.25 8.25h7.5V3h-7.5v5.25zM12 12.75h.008v.008H12v-.008z" /></svg>;
const ChatBubbleLeftRightIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M20.25 8.511c.884.284 1.5 1.128 1.5 2.097v4.286c0 1.136-.847 2.1-1.98 2.193-.34.027-.68.052-1.02.072v3.091l-3.68-3.091a4.523 4.523 0 00-1.028-.296H9M3.75 9.75h1.5a.75.75 0 01.75.75v4.5a.75.75 0 01-.75.75h-1.5a.75.75 0 01-.75-.75V10.5a.75.75 0 01.75-.75zM21 9.75A2.25 2.25 0 0018.75 7.5H9A2.25 2.25 0 006.75 9.75v4.5A2.25 2.25 0 009 16.5h2.652a4.516 4.516 0 011.924.227l3.734 3.114A.75.75 0 0018 22.07V16.5A2.25 2.25 0 0015.75 14.25H9.75" /></svg>;
const InboxStackIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3.75 12h16.5m-16.5 3.75h16.5M3.75 19.5h16.5M5.625 4.5h12.75a1.875 1.875 0 010 3.75H5.625a1.875 1.875 0 010-3.75z" /></svg>;
const ListBulletIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 6.75h7.5M8.25 12h7.5m-7.5 5.25h7.5M3.75 6.75h.007v.008H3.75V6.75zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 12h.007v.008H3.75V12zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 17.25h.007v.008H3.75v-.008zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0z" /></svg>;
const ClipboardDocumentCheckIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M10.125 2.25h-4.5c-.621 0-1.125.504-1.125 1.125v17.25c0 .621.504 1.125 1.125 1.125h12.75c.621 0 1.125-.504 1.125-1.125v-9M10.125 2.25c.882 0 1.724.222 2.494.626M10.125 2.25a2.25 2.25 0 00-2.25 2.25M10.125 2.25v3.375c0 .621.504 1.125 1.125 1.125h3.375M9 15l2.25 2.25L15 12" /></svg>;
const KeyIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M15.75 5.25a3 3 0 013 3m3 0a6 6 0 01-7.029 5.912c-.563-.097-1.159.026-1.563.43L10.5 17.25H8.25v2.25H6v2.25H2.25v-2.818c0-.597.237-1.17.659-1.591l6.499-6.499c.404-.404.527-1 .43-1.563A6 6 0 1121.75 8.25z" /></svg>;
const VariableIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M4.75 4.75h14.5M4.75 19.25h14.5M12 4.75v14.5M4.75 12H12m7.25 0H12M7.5 8.25l3.75 3.75L7.5 15.75M16.5 8.25L12.75 12l3.75 3.75" /></svg>;
const ArrowDownTrayIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5M16.5 12L12 16.5m0 0L7.5 12m4.5 0V3" /></svg>;
const ArrowUpTrayIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5m-13.5-9L12 3m0 0l4.5 4.5M12 3v13.5" /></svg>;
const SparklesIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9.813 15.904L9 18.75l-.813-2.846a4.5 4.5 0 00-3.09-3.09L1.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L9 5.25l.813 2.846a4.5 4.5 0 003.09 3.09L15.75 12l-2.846.813a4.5 4.5 0 00-3.09 3.09zM18.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L24 5.25l-.813 2.846a4.5 4.5 0 00-3.09 3.09L18.25 12zm0 0l-2.846.813a4.5 4.5 0 00-3.09 3.09L12 18.75l.813-2.846a4.5 4.5 0 003.09-3.09L18.25 12z" /></svg>;


const MAX_EXECUTION_LOGS = 10;
const AUTOMATION_STORAGE_KEY = 'dashboard-automations-redline';

const initialAutomations: Automation[] = [
  { id: 'auto-log-startup', name: 'Log Startup Message', description: 'Logs a message when manually run.', isEnabled: true, trigger: { type: AutomationTriggerType.MANUAL, config: {} }, action: { type: AutomationActionType.LOG_MESSAGE, config: { logMessage: 'Automation Service Initialized by Redline Protocol.' } }, createdAt: new Date().toISOString(), runCount: 0, executionLogs: []},
  { id: 'auto-notify-test', name: 'Manual Test Notification', description: 'Sends a test notification.', isEnabled: true, trigger: { type: AutomationTriggerType.MANUAL, config: {} }, action: { type: AutomationActionType.SEND_NOTIFICATION, config: { notificationMessage: 'Test Notification from Automation Nexus!', notificationType: 'info' } }, createdAt: new Date().toISOString(), runCount: 0, executionLogs: [] },
];

const RAG_ITEMS_LOCALSTORAGE_KEY = 'rag-agent-items';
const TASKS_LOCALSTORAGE_KEY = 'dashboard-tasks';
const CALENDAR_EVENTS_LOCALSTORAGE_KEY = 'dashboard-calendarEvents';

interface AutomationServiceFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
      sourceItemId?: string; 
    }
  ) => void;
}

const replacePlaceholders = (templateString: string, context: AutomationRunContext): string => {
  if (!templateString) return '';
  return templateString.replace(/\{\{(.*?)\}\}/g, (match, key) => {
    const trimmedKey = key.trim();
    let value: any = context; 
    try {
        for (const k of trimmedKey.split('.')) {
            if (value && typeof value === 'object' && value !== null && k in value) {
                value = value[k];
            } else {
                value = undefined; 
                break;
            }
        }
    } catch (e) { value = undefined; }
    
    if (value !== undefined) {
        return typeof value === 'object' ? JSON.stringify(value) : String(value);
    }
    return match; 
  });
};

const parseRelativeDate = (dateString: string, baseDate: Date = new Date()): string => {
  const trimmedDateString = dateString ? dateString.trim() : '';
  if (!trimmedDateString) return new Date().toISOString().split('T')[0];

  const relativeDateMatch = trimmedDateString.match(/^([+-])(\d+)([dmyw])$/i);
  
  if (relativeDateMatch) {
    const sign = relativeDateMatch[1];
    const amount = parseInt(relativeDateMatch[2], 10);
    const unit = relativeDateMatch[3].toLowerCase();
    const newDate = new Date(baseDate);

    if (unit === 'd') newDate.setDate(newDate.getDate() + (sign === '+' ? amount : -amount));
    else if (unit === 'w') newDate.setDate(newDate.getDate() + (sign === '+' ? amount * 7 : -amount * 7));
    else if (unit === 'm') newDate.setMonth(newDate.getMonth() + (sign === '+' ? amount : -amount));
    else if (unit === 'y') newDate.setFullYear(newDate.getFullYear() + (sign === '+' ? amount : -amount));
    
    return newDate.toISOString().split('T')[0];
  }
  
  if (!trimmedDateString.includes("{{")) { // Avoid parsing if it looks like a placeholder
    try {
        const parsedDate = new Date(trimmedDateString);
        if (!isNaN(parsedDate.getTime())) {
            return parsedDate.toISOString().split('T')[0];
        }
    } catch (e) { /* ignore date parse errors for unresolved placeholders */ }
  }
  return trimmedDateString; // Return original string if not a relative date and might be a placeholder
};


const AutomationServiceFeature: React.FC<AutomationServiceFeatureProps> = ({ addNotification }) => {
  const [automations, setAutomations] = useState<Automation[]>(() => {
    const savedAutomations = localStorage.getItem(AUTOMATION_STORAGE_KEY);
    if (savedAutomations) {
      try { 
        const parsed = JSON.parse(savedAutomations);
        return parsed.map((auto: Automation) => ({...auto, executionLogs: Array.isArray(auto.executionLogs) ? auto.executionLogs : [] }));
      } catch (e) { console.error("Failed to parse automations", e); }
    }
    return initialAutomations;
  });

  const [showModal, setShowModal] = useState(false);
  const [editingAutomation, setEditingAutomation] = useState<Automation | null>(null);
  const [runningAutomationId, setRunningAutomationId] = useState<string | null>(null);
  const [expandedLogAutoId, setExpandedLogAutoId] = useState<string | null>(null);
  
  const [formState, setFormState] = useState<Partial<Automation>>({});
  const [mockEventDataInput, setMockEventDataInput] = useState<string>('{}'); 
  const [aiDescription, setAiDescription] = useState('');
  const [aiSuggestedConfig, setAiSuggestedConfig] = useState<string | null>(null);
  const [jsonImportModalOpen, setJsonImportModalOpen] = useState(false);
  const [jsonToImport, setJsonToImport] = useState('');


  useEffect(() => {
    localStorage.setItem(AUTOMATION_STORAGE_KEY, JSON.stringify(automations));
  }, [automations]);

  const resetForm = () => {
    setFormState({
        name: '', description: '', isEnabled: true,
        trigger: { type: AutomationTriggerType.MANUAL, config: {} },
        action: { type: AutomationActionType.LOG_MESSAGE, config: {} },
    });
    setEditingAutomation(null);
    setMockEventDataInput('{}');
    setAiDescription('');
    setAiSuggestedConfig(null);
  };

  const handleOpenModal = (automationToEdit?: Automation) => {
    if (automationToEdit) {
      setEditingAutomation(automationToEdit);
      setFormState(JSON.parse(JSON.stringify(automationToEdit))); 
      if (automationToEdit.trigger.type === AutomationTriggerType.EVENT_BASED) {
        const existingMockData = localStorage.getItem(`automation-mock-event-${automationToEdit.id}`);
        setMockEventDataInput(existingMockData || '{}');
      } else {
        setMockEventDataInput('{}');
      }
    } else {
      resetForm();
    }
    setShowModal(true);
  };
  
  const handleFormChange = (
    path: string, 
    value: any,
    isCheckboxBoolean?: boolean // For checkboxes that set boolean values
  ) => {
    setFormState(prev => {
      const keys = path.split('.');
      let newState = JSON.parse(JSON.stringify(prev)); 
      let pointer: any = newState;
  
      keys.forEach((key, index) => {
        if (index === keys.length - 1) {
          if (isCheckboxBoolean) { // Handle boolean checkboxes directly
            pointer[key] = (value as HTMLInputElement).checked;
          } else if (typeof value === 'object' && value !== null && 'target' in value && 'type' in value.target) { // Standard event object
            const target = value.target as HTMLInputElement | HTMLSelectElement | HTMLTextAreaElement;
            if (target.type === 'checkbox') {
              // For multi-select checkbox group, it's handled in its own change handler.
              // This generic one should assume single value unless specific logic is outside.
              pointer[key] = (target as HTMLInputElement).checked;
            } else {
              pointer[key] = target.value;
            }
          } else { // Direct value assignment (e.g., from custom checkbox group handler)
            pointer[key] = value;
          }
        } else {
          if (!pointer[key] || typeof pointer[key] !== 'object') {
            pointer[key] = {}; 
          }
          pointer = pointer[key];
        }
      });
      return newState;
    });
  };

  const addExecutionLog = (automationId: string, entryData: Omit<ExecutionLogEntry, 'id' | 'timestamp'>) => {
    const newLogEntry: ExecutionLogEntry = {
      id: crypto.randomUUID(),
      timestamp: new Date().toISOString(),
      ...entryData,
      actionOutputSnapshot: entryData.actionOutputSnapshot ? String(entryData.actionOutputSnapshot).substring(0, 200) + (String(entryData.actionOutputSnapshot).length > 200 ? '...' : '') : undefined,
    };
    setAutomations(prev => prev.map(auto => auto.id === automationId ? {
      ...auto,
      executionLogs: [newLogEntry, ...(auto.executionLogs || [])].slice(0, MAX_EXECUTION_LOGS)
    } : auto));
  };

  const handleSaveAutomation = () => {
    if (!formState.name?.trim()) { addNotification('error', 'Automation name is required.'); return; }
    if (!formState.trigger?.type || !formState.action?.type) { addNotification('error', 'Trigger and Action types must be selected.'); return; }

    const finalAutomation: Automation = {
      id: editingAutomation?.id || crypto.randomUUID(),
      name: formState.name.trim(),
      description: formState.description?.trim() || undefined,
      isEnabled: formState.isEnabled !== undefined ? formState.isEnabled : true,
      trigger: formState.trigger!, 
      action: formState.action!,   
      createdAt: editingAutomation?.createdAt || new Date().toISOString(),
      lastRun: editingAutomation?.lastRun,
      runCount: editingAutomation?.runCount || 0,
      executionLogs: editingAutomation?.executionLogs || [],
    };

    if (finalAutomation.trigger.type === AutomationTriggerType.EVENT_BASED) {
      try {
        JSON.parse(mockEventDataInput); 
        localStorage.setItem(`automation-mock-event-${finalAutomation.id}`, mockEventDataInput);
      } catch (e) {
        addNotification('error', 'Mock Event Data is not valid JSON. Automation saved, but mock data not updated.');
      }
    }


    if (editingAutomation) {
      setAutomations(prev => prev.map(a => a.id === finalAutomation.id ? finalAutomation : a));
      addNotification('success', `Automation "${finalAutomation.name}" updated.`);
    } else {
      setAutomations(prev => [finalAutomation, ...prev]);
      addNotification('success', `Automation "${finalAutomation.name}" created.`);
    }
    setShowModal(false); resetForm();
  };

  const handleDeleteAutomation = (automationId: string) => {
    const auto = automations.find(a => a.id === automationId);
    if (auto && confirm(`Delete automation "${auto.name}"?`)) {
      setAutomations(prev => prev.filter(a => a.id !== automationId));
      localStorage.removeItem(`automation-mock-event-${automationId}`); 
      addNotification('info', `Automation "${auto.name}" deleted.`);
    }
  };

  const handleToggleEnable = (automationId: string) => {
    setAutomations(prev => prev.map(a => a.id === automationId ? { ...a, isEnabled: !a.isEnabled } : a));
  };

  const handleRunManualAutomation = async (automation: Automation) => {
    if (!automation.isEnabled) { addNotification('info', `Automation "${automation.name}" is disabled.`); return; }
    
    let simulatedEventData: Record<string, any> = {};
    if (automation.trigger.type === AutomationTriggerType.EVENT_BASED) {
      const mockDataString = localStorage.getItem(`automation-mock-event-${automation.id}`) || mockEventDataInput || '{}';
      try {
        simulatedEventData = JSON.parse(mockDataString);
      } catch (e) {
        addNotification('error', `Failed to parse Mock Event Data for "${automation.name}". Using empty data. Error: ${e instanceof Error ? e.message : String(e)}`);
        simulatedEventData = {};
      }
    }

    setRunningAutomationId(automation.id);
    let status: 'success' | 'failure' = 'success';
    let summaryMessage = `Action: ${automation.action.type}.`;
    let actionInputSnapshot: Record<string, any> = {};
    let actionOutputSnapshot: any = null;
    const now = new Date();

    const context: AutomationRunContext = {
      trigger: {
        type: automation.trigger.type,
        eventSource: automation.trigger.config.eventSource,
        eventData: simulatedEventData 
      },
      automation: { id: automation.id, name: automation.name },
      now: { iso: now.toISOString(), date: now.toISOString().split('T')[0], time: now.toTimeString().split(' ')[0], unix: Math.floor(now.getTime() / 1000) },
      actionData: {} 
    };

    try {
      const actionConfig = JSON.parse(JSON.stringify(automation.action.config || {})); 
      const actionType = automation.action.type;
      
      for (const key in actionConfig) {
        if (typeof actionConfig[key] === 'string') {
          actionConfig[key] = replacePlaceholders(actionConfig[key], context);
        } else if (typeof actionConfig[key] === 'number' && (String(automation.action.config[key as keyof AutomationActionConfig]).includes("{{"))) {
           const resolvedNumString = replacePlaceholders(String(automation.action.config[key as keyof AutomationActionConfig]), context);
           const numVal = parseFloat(resolvedNumString);
           actionConfig[key] = isNaN(numVal) ? automation.action.config[key as keyof AutomationActionConfig] : numVal; 
        }
      }
      actionInputSnapshot = actionConfig; 

      switch (actionType) {
        case AutomationActionType.LOG_MESSAGE:
          const logMsg = actionInputSnapshot.logMessage || 'No message configured.';
          console.log(`[Automation: ${automation.name}] ${logMsg}`);
          summaryMessage = `Logged: "${logMsg.substring(0, 50)}..."`;
          actionOutputSnapshot = logMsg;
          break;

        case AutomationActionType.SEND_NOTIFICATION:
          const notifMsg = actionInputSnapshot.notificationMessage || `Automation "{{automation.name}}" executed.`;
          addNotification(actionInputSnapshot.notificationType || 'info', notifMsg, { sourceItemId: automation.id, sourceView: ViewType.AUTOMATION_SERVICE });
          summaryMessage = `Sent notification: "${notifMsg.substring(0, 50)}..."`;
          actionOutputSnapshot = notifMsg;
          break;

        case AutomationActionType.RUN_RAG_ANALYSIS: {
          if (!actionInputSnapshot.ragAnalysisType) throw new Error("RAG Analysis type not specified.");
          let ragItems: AggregatedItem[] = [];
          try {
            ragItems = JSON.parse(localStorage.getItem(RAG_ITEMS_LOCALSTORAGE_KEY) || '[]').map((i:any)=>({...i, createdAt: new Date(i.createdAt)}));
          } catch (e) { throw new Error("Failed to load RAG items from storage."); }

          let targetItem: AggregatedItem | undefined;
          const targetMode = actionInputSnapshot.ragAnalysisTargetMode || 'EVENT_TRIGGER_ITEM';

          if (targetMode === 'EVENT_TRIGGER_ITEM' && context.trigger.eventData?.itemId) {
            targetItem = ragItems.find(item => item.id === context.trigger.eventData!.itemId);
          } else if (targetMode === 'SPECIFIC_ITEM_ID' && actionInputSnapshot.ragAnalysisTargetItemId) {
            targetItem = ragItems.find(item => item.id === actionInputSnapshot.ragAnalysisTargetItemId);
          } else if (targetMode === 'LATEST_ITEM_BY_TYPE' && actionInputSnapshot.ragAnalysisTargetItemType) {
            targetItem = ragItems.filter(item => item.type === actionInputSnapshot.ragAnalysisTargetItemType).sort((a,b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime())[0];
          }
          if (!targetItem) throw new Error(`Target RAG item not found for mode: ${targetMode}.`);
          
          context.actionData = { ...(context.actionData || {}), itemId: targetItem.id, itemName: targetItem.name, itemType: targetItem.type, itemContentSnippet: targetItem.content.substring(0,100) };
          
          actionOutputSnapshot = await analyzeWithGemini(targetItem.content, actionInputSnapshot.ragAnalysisType, targetItem.originalMimeType, undefined, actionInputSnapshot.ragAnalysisCustomPrompt || '');
          
          const updatedRagItems = ragItems.map(item => 
            item.id === targetItem!.id ? { ...item, geminiAnalysis: { ...(item.geminiAnalysis || {}), [actionInputSnapshot.ragAnalysisType!]: actionOutputSnapshot } } : item
          );
          localStorage.setItem(RAG_ITEMS_LOCALSTORAGE_KEY, JSON.stringify(updatedRagItems.map(i => ({...i, createdAt: i.createdAt.toISOString()}))));
          summaryMessage = `Performed ${actionInputSnapshot.ragAnalysisType} on RAG item '${targetItem.name}'.`;
          break;
        }
        
        case AutomationActionType.CREATE_PRODUCTIVITY_TASK: {
            if (!actionInputSnapshot.taskText) throw new Error("Task text is required for Create Task action.");
            const reminderMinutesBeforeRaw = actionInputSnapshot.taskReminderMinutesBefore;
            const priorityRaw = actionInputSnapshot.taskPriority;

            const newTask: TaskItem = {
                id: crypto.randomUUID(),
                text: actionInputSnapshot.taskText,
                completed: false,
                createdAt: now,
                dueDate: actionInputSnapshot.taskDueDate ? parseRelativeDate(actionInputSnapshot.taskDueDate, now) : undefined,
                dueTime: actionInputSnapshot.taskDueTime || undefined,
                reminderMinutesBefore: reminderMinutesBeforeRaw !== undefined && !isNaN(Number(reminderMinutesBeforeRaw)) ? Number(reminderMinutesBeforeRaw) : undefined,
                scheduledDate: actionInputSnapshot.taskScheduledDate ? parseRelativeDate(actionInputSnapshot.taskScheduledDate, now) : undefined,
                priority: priorityRaw !== undefined && !isNaN(Number(priorityRaw)) ? Number(priorityRaw) : undefined,
            };
            let tasks: TaskItem[] = [];
            try { tasks = JSON.parse(localStorage.getItem(TASKS_LOCALSTORAGE_KEY) || '[]').map((t:any)=>({...t, createdAt: new Date(t.createdAt)})); } catch(e){ throw new Error("Failed to load Tasks from storage."); }
            tasks.unshift(newTask); 
            localStorage.setItem(TASKS_LOCALSTORAGE_KEY, JSON.stringify(tasks.map(t => ({...t, createdAt: t.createdAt.toISOString()}))));
            summaryMessage = `Created task: "${newTask.text.substring(0, 50)}..."`;
            actionOutputSnapshot = { taskId: newTask.id, taskText: newTask.text };
            break;
        }

        case AutomationActionType.COMPLETE_PRODUCTIVITY_TASK: {
            let tasks: TaskItem[] = [];
            try { tasks = JSON.parse(localStorage.getItem(TASKS_LOCALSTORAGE_KEY) || '[]').map((t:any)=>({...t, createdAt: new Date(t.createdAt)})); } catch(e){ throw new Error("Failed to load Tasks from storage."); }
            let taskToComplete: TaskItem | undefined;
            
            if (actionInputSnapshot.completeTaskId) {
                taskToComplete = tasks.find(t => t.id === actionInputSnapshot.completeTaskId);
            } else if (actionInputSnapshot.completeTaskNameQuery) {
                const query = (actionInputSnapshot.completeTaskNameQuery as string).toLowerCase();
                taskToComplete = tasks.find(t => !t.completed && t.text.toLowerCase().includes(query));
            } else {
                 throw new Error("Task ID or Name Query is required for Complete Task action.");
            }
            if (!taskToComplete) throw new Error("Task to complete not found.");

            const updatedTasks = tasks.map(t => t.id === taskToComplete!.id ? { ...t, completed: true } : t);
            localStorage.setItem(TASKS_LOCALSTORAGE_KEY, JSON.stringify(updatedTasks.map(t => ({...t, createdAt: t.createdAt.toISOString()}))));
            summaryMessage = `Completed task: "${taskToComplete.text.substring(0,50)}..."`;
            actionOutputSnapshot = { taskId: taskToComplete.id, taskText: taskToComplete.text };
            break;
        }

        case AutomationActionType.CREATE_CALENDAR_EVENT: {
            if (!actionInputSnapshot.eventTitle || !actionInputSnapshot.eventDate) throw new Error("Event title and date are required for Create Calendar Event action.");
            const reminderMinutesBeforeRaw = actionInputSnapshot.eventReminderMinutesBeforeForCreate;

            const newEvent: CalendarEvent = {
                id: crypto.randomUUID(),
                title: actionInputSnapshot.eventTitle,
                date: parseRelativeDate(actionInputSnapshot.eventDate, now),
                startTime: actionInputSnapshot.eventAllDay ? '' : (actionInputSnapshot.eventStartTime || '00:00'),
                endTime: actionInputSnapshot.eventAllDay ? '' : (actionInputSnapshot.eventEndTime || '00:00'),
                description: actionInputSnapshot.eventDescription || '',
                allDay: !!actionInputSnapshot.eventAllDay,
                createdAt: now.toISOString(),
                reminderMinutesBefore: reminderMinutesBeforeRaw !== undefined && !isNaN(Number(reminderMinutesBeforeRaw)) ? Number(reminderMinutesBeforeRaw) : undefined,
            };
            let events: CalendarEvent[] = [];
            try { events = JSON.parse(localStorage.getItem(CALENDAR_EVENTS_LOCALSTORAGE_KEY) || '[]'); } catch(e){ throw new Error("Failed to load Calendar Events from storage."); }
            events.unshift(newEvent); 
            localStorage.setItem(CALENDAR_EVENTS_LOCALSTORAGE_KEY, JSON.stringify(events));
            summaryMessage = `Created calendar event: "${newEvent.title.substring(0,50)}..."`;
            actionOutputSnapshot = { eventId: newEvent.id, eventTitle: newEvent.title };
            break;
        }
        
        case AutomationActionType.STD_UTIL_BASE64_ENCODE: {
            const input = actionInputSnapshot.stdUtilInputString || '';
            if (!input) throw new Error("Input string for Base64 Encode is empty.");
            actionOutputSnapshot = btoa(unescape(encodeURIComponent(input)));
            summaryMessage = `Base64 Encoded. Output: "${(actionOutputSnapshot as string).substring(0,20)}..."`;
            break;
        }
        case AutomationActionType.STD_UTIL_BASE64_DECODE: {
            const input = actionInputSnapshot.stdUtilInputString || '';
            if (!input) throw new Error("Input string for Base64 Decode is empty.");
            actionOutputSnapshot = decodeURIComponent(escape(atob(input)));
            summaryMessage = `Base64 Decoded. Output: "${(actionOutputSnapshot as string).substring(0,20)}..."`;
            break;
        }
        case AutomationActionType.STD_UTIL_URL_ENCODE: {
            const input = actionInputSnapshot.stdUtilInputString || '';
            if (!input) throw new Error("Input string for URL Encode is empty.");
            actionOutputSnapshot = encodeURIComponent(input);
            summaryMessage = `URL Encoded. Output: "${(actionOutputSnapshot as string).substring(0,20)}..."`;
            break;
        }
        case AutomationActionType.STD_UTIL_URL_DECODE: {
            const input = actionInputSnapshot.stdUtilInputString || '';
            if (!input) throw new Error("Input string for URL Decode is empty.");
            actionOutputSnapshot = decodeURIComponent(input);
            summaryMessage = `URL Decoded. Output: "${(actionOutputSnapshot as string).substring(0,20)}..."`;
            break;
        }
        case AutomationActionType.STD_UTIL_GENERATE_UUID: {
            actionOutputSnapshot = crypto.randomUUID();
            summaryMessage = `Generated UUID: ${actionOutputSnapshot}`;
            break;
        }
        case AutomationActionType.STD_UTIL_JSON_FORMAT: {
            const input = actionInputSnapshot.stdUtilInputString || '';
            if (!input) throw new Error("Input JSON string for Format is empty.");
            const parsed = JSON.parse(input);
            const indent = actionInputSnapshot.stdUtilJsonIndentation === 'tab' ? '\t' : (actionInputSnapshot.stdUtilJsonIndentation || 2);
            actionOutputSnapshot = JSON.stringify(parsed, null, indent);
            summaryMessage = `Formatted JSON. Length: ${(actionOutputSnapshot as string).length}`;
            break;
        }
        case AutomationActionType.INVOKE_GEMINI_CHAT_WITH_RAG_CONTEXT: {
            const chatPromptTpl = actionInputSnapshot.chatPromptTemplate || "Summarize this.";
            let chatContextText = "";
            if(actionInputSnapshot.chatContextMode === 'EVENT_TRIGGER_ITEM_CONTENT' && context.trigger.eventData?.itemContentSnippet) { // Ensure itemContentSnippet is populated in event data
                chatContextText = context.trigger.eventData.itemContentSnippet;
            } else if (actionInputSnapshot.chatContextMode === 'ITEM_ID_CONTENT' && actionInputSnapshot.chatContextItemId) {
                let ragItems: AggregatedItem[] = [];
                try{ ragItems = JSON.parse(localStorage.getItem(RAG_ITEMS_LOCALSTORAGE_KEY) || '[]'); } catch {}
                const item = ragItems.find(i => i.id === actionInputSnapshot.chatContextItemId);
                chatContextText = item ? item.content.substring(0, 2000) : "Context item not found."; 
            } else if (actionInputSnapshot.chatContextMode === 'CUSTOM_TEXT' && actionInputSnapshot.chatCustomContextText) {
                chatContextText = actionInputSnapshot.chatCustomContextText;
            }
            // Store the determined context in actionData for placeholder replacement in the prompt template itself
            context.actionData = { ...(context.actionData || {}), chatContext: chatContextText };
            const finalChatPrompt = replacePlaceholders(chatPromptTpl, context);
            
            actionOutputSnapshot = await analyzeWithGemini(finalChatPrompt, GeminiActionType.SUMMARIZE); 
            summaryMessage = `Invoked Gemini Chat. Prompt snippet: "${finalChatPrompt.substring(0,30)}..."`;
            break;
        }
        case AutomationActionType.UPDATE_RAG_ITEM_ANALYSIS: {
            const targetItemId = actionInputSnapshot.updateTargetItemIdMode === 'EVENT_TRIGGER_ITEM' && context.trigger.eventData?.itemId 
                ? context.trigger.eventData.itemId 
                : (actionInputSnapshot.updateTargetItemId || null);
            
            if (!targetItemId) throw new Error("Target Item ID for analysis update not found.");
            if (!actionInputSnapshot.analysisKeyToUpdate) throw new Error("Analysis Key to update not specified.");
            
            let newValue: string | string[] = "Default Updated Value"; // Default to string
            if(actionInputSnapshot.newValueMode === 'PREVIOUS_ACTION_RESULT' && actionInputSnapshot.previousActionResultKey) {
                const prevResult = context.actionData?.[actionInputSnapshot.previousActionResultKey];
                newValue = prevResult !== undefined ? prevResult : `Error: Key '${actionInputSnapshot.previousActionResultKey}' not found in previous action results.`;
            } else if (actionInputSnapshot.staticNewValue !== undefined) { // Check for undefined because empty string is valid
                newValue = actionInputSnapshot.staticNewValue; // This can be string or string[]
            }
            
            let ragItemsToUpdate: AggregatedItem[] = [];
            try{ ragItemsToUpdate = JSON.parse(localStorage.getItem(RAG_ITEMS_LOCALSTORAGE_KEY) || '[]').map((i:any)=>({...i, createdAt: new Date(i.createdAt)})); } catch {}
            
            let itemFoundForUpdate = false;
            const updatedRagItemsForAnalysis = ragItemsToUpdate.map(item => {
                if (item.id === targetItemId) {
                    itemFoundForUpdate = true;
                    return { ...item, geminiAnalysis: { ...(item.geminiAnalysis || {}), [actionInputSnapshot.analysisKeyToUpdate!]: newValue } };
                }
                return item;
            });
            if (!itemFoundForUpdate) throw new Error (`RAG Item with ID "${targetItemId}" not found for analysis update.`);
            localStorage.setItem(RAG_ITEMS_LOCALSTORAGE_KEY, JSON.stringify(updatedRagItemsForAnalysis.map(i => ({...i, createdAt: i.createdAt.toISOString()}))));
            
            summaryMessage = `Updated analysis key "${actionInputSnapshot.analysisKeyToUpdate}" on item ID "${targetItemId}".`;
            actionOutputSnapshot = { itemId: targetItemId, key: actionInputSnapshot.analysisKeyToUpdate, value: newValue };
            break;
        }
        default:
          const exhaustiveCheck: never = actionType;
          throw new Error(`Action type "${exhaustiveCheck}" execution not implemented.`);
      }
      if (actionConfig.actionOutputKey && actionOutputSnapshot !== null && actionOutputSnapshot !== undefined) {
        context.actionData = { ...(context.actionData || {}), [actionConfig.actionOutputKey]: actionOutputSnapshot };
      }
      addNotification('success', `Automation "${automation.name}" executed successfully.`);
    } catch (error) {
      status = 'failure';
      const errMsg = error instanceof Error ? error.message : String(error);
      summaryMessage = `Error during "${automation.name}": ${errMsg}`;
      actionOutputSnapshot = error instanceof Error ? { error: errMsg, stack: error.stack?.substring(0,150) } : { error: errMsg };
      addNotification('error', summaryMessage);
      console.error(`Automation "${automation.name}" failed:`, error);
    } finally {
      addExecutionLog(automation.id, { status, triggerType: automation.trigger.type, actionType: automation.action.type, summaryMessage, eventDataSnapshot: context.trigger.eventData, actionInputSnapshot, actionOutputSnapshot });
      setAutomations(prev => prev.map(a => a.id === automation.id ? { ...a, lastRun: new Date().toISOString(), runCount: (a.runCount || 0) + 1 } : a));
      setRunningAutomationId(null);
    }
  };
  
  const getIconForTrigger = (type: AutomationTriggerType): React.ReactElement => {
    switch(type) {
        case AutomationTriggerType.MANUAL: return <PlayCircleIconInternal className="w-4 h-4 text-sky-400"/>;
        case AutomationTriggerType.TIME_BASED: return <ClockIconInternal className="w-4 h-4 text-amber-400"/>;
        case AutomationTriggerType.EVENT_BASED: return <BellAlertIconInternal className="w-4 h-4 text-purple-400"/>;
        default: return <BoltIconInternal className="w-4 h-4 text-slate-400"/>;
    }
  };

  const getIconForAction = (type: AutomationActionType): React.ReactElement => {
    switch(type) {
        case AutomationActionType.LOG_MESSAGE: return <DocumentTextIconInternal className="w-4 h-4 text-green-400"/>;
        case AutomationActionType.SEND_NOTIFICATION: return <BellAlertIconInternal className="w-4 h-4 text-blue-400"/>;
        case AutomationActionType.RUN_RAG_ANALYSIS: return <CpuChipIconInternal className="w-4 h-4 text-indigo-400"/>;
        case AutomationActionType.INVOKE_GEMINI_CHAT_WITH_RAG_CONTEXT: return <ChatBubbleLeftRightIconInternal className="w-4 h-4 text-teal-400"/>;
        case AutomationActionType.UPDATE_RAG_ITEM_ANALYSIS: return <InboxStackIconInternal className="w-4 h-4 text-orange-400"/>;
        case AutomationActionType.CREATE_PRODUCTIVITY_TASK: return <ClipboardDocumentCheckIconInternal className="w-4 h-4 text-lime-400"/>;
        case AutomationActionType.COMPLETE_PRODUCTIVITY_TASK: return <ClipboardDocumentCheckIconInternal className="w-4 h-4 text-emerald-400 line-through"/>;
        case AutomationActionType.CREATE_CALENDAR_EVENT: return <CalendarDaysIconInternal className="w-4 h-4 text-fuchsia-400"/>;
        case AutomationActionType.STD_UTIL_BASE64_ENCODE: case AutomationActionType.STD_UTIL_BASE64_DECODE: case AutomationActionType.STD_UTIL_URL_ENCODE: case AutomationActionType.STD_UTIL_URL_DECODE: case AutomationActionType.STD_UTIL_JSON_FORMAT: return <KeyIconInternal className="w-4 h-4 text-yellow-400"/>;
        case AutomationActionType.STD_UTIL_GENERATE_UUID: return <VariableIconInternal className="w-4 h-4 text-rose-400"/>;
        default: return <BoltIconInternal className="w-4 h-4 text-slate-400"/>;
    }
  };

  const renderEventSourceFilters = (config: AutomationTriggerConfig, path: string) => {
    switch(config.eventSource) {
        case AutomationEventSource.RAG_AGENT_NEW_ITEM_ADDED:
        case AutomationEventSource.RAG_AGENT_ITEM_DELETED:
        case AutomationEventSource.RAG_AGENT_ANALYSIS_COMPLETE:
            return (<>
                <div><label className="block text-slate-400 mb-0.5">Filter by Item Type (Optional)</label>
                    <div className="grid grid-cols-2 gap-1">{Object.values(ItemType).map(it => (<label key={it} className="flex items-center space-x-1.5 p-1 bg-[var(--theme-input-bg)]/70 rounded hover:bg-[var(--theme-input-bg)]"><input type="checkbox" value={it} checked={config.eventItemTypeFilter?.includes(it) || false} onChange={e => { const cF = config.eventItemTypeFilter || []; const nF = e.target.checked ? [...cF, it] : cF.filter(x => x !== it); handleFormChange(`${path}.eventItemTypeFilter`, nF.length > 0 ? nF : undefined, true);}} className="h-3 w-3 text-pink-500 bg-slate-500 border-slate-400 rounded focus:ring-pink-400"/><span>{it.replace(/_/g, ' ')}</span></label>))}</div>
                </div>
                <div><label className="block text-slate-400 mb-0.5">Filter by Name (Contains, Optional) <span title="Variables like {{trigger.eventData.itemName}} available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={config.eventNameFilter || ''} onChange={e => handleFormChange(`${path}.eventNameFilter`, e.target.value || undefined)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>
                 {config.eventSource === AutomationEventSource.RAG_AGENT_ANALYSIS_COMPLETE && (
                    <div><label className="block text-slate-400 mb-0.5">Filter by Analysis Type (Optional)</label><select value={config.eventAnalysisTypeFilter || ''} onChange={e => handleFormChange(`${path}.eventAnalysisTypeFilter`, e.target.value as GeminiActionType || undefined)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"><option value="">Any Analysis</option>{Object.values(GeminiActionType).map(act => <option key={act} value={act}>{act.replace(/_/g, ' ')}</option>)}</select></div>
                 )}
            </>);
        case AutomationEventSource.PRODUCTIVITY_SUITE_TASK_COMPLETED:
            return (<>
                <div><label className="block text-slate-400 mb-0.5">Task Name Contains (Optional) <span title="Variables like {{trigger.eventData.taskText}} available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={config.taskNameContainsFilter || ''} onChange={e => handleFormChange(`${path}.taskNameContainsFilter`, e.target.value || undefined)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>
                <label className="flex items-center space-x-1.5 p-1"><input type="checkbox" checked={!!config.taskHasDueDateFilter} onChange={e => handleFormChange(`${path}.taskHasDueDateFilter`, e.target.checked ? true : undefined, true)} className="h-3 w-3 text-pink-500"/><span>Only if Task Has Due Date</span></label>
                <div><label className="block text-slate-400 mb-0.5">Priority Equals (Optional) <span title="Variables like {{trigger.eventData.priority}} available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={config.taskPriorityFilter === undefined ? '' : String(config.taskPriorityFilter)} onChange={e => handleFormChange(`${path}.taskPriorityFilter`, e.target.value ? Number(e.target.value) : undefined)} placeholder="e.g., 1 or {{trigger.eventData.priority}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>
            </>);
        case AutomationEventSource.PRODUCTIVITY_SUITE_CALENDAR_EVENT_STARTING:
            return (<>
                <div><label className="block text-slate-400 mb-0.5">Minutes Before Event to Trigger (e.g., 15)* <span title="Variables like {{trigger.eventData.minutesBefore}} available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={config.minutesBeforeToTrigger === undefined ? '' : String(config.minutesBeforeToTrigger)} onChange={e => handleFormChange(`${path}.minutesBeforeToTrigger`, e.target.value ? Number(e.target.value) : undefined)} placeholder="e.g., 15 or {{trigger.eventData.reminderOffset}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>
                <div><label className="block text-slate-400 mb-0.5">Event Title Contains (Optional) <span title="Variables like {{trigger.eventData.eventTitle}} available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={config.eventTitleContainsFilter || ''} onChange={e => handleFormChange(`${path}.eventTitleContainsFilter`, e.target.value || undefined)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>
                <label className="flex items-center space-x-1.5 p-1"><input type="checkbox" checked={!!config.eventHasAttendeesFilter} onChange={e => handleFormChange(`${path}.eventHasAttendeesFilter`, e.target.checked ? true : undefined, true)} className="h-3 w-3 text-pink-500"/><span>Only if Event Has Attendees</span></label>
            </>);
        default: return <p className="text-slate-500 italic text-xs">No specific filters for this event source.</p>;
    }
  };

  const handleGenerateWithAI = useCallback(() => {
    if(!aiDescription.trim()) {
        addNotification('info', 'Please describe the automation you want the AI to generate.');
        return;
    }
    addNotification('info', `Redline AI: Automation generation initiated for: "${aiDescription.substring(0,50)}...". This feature is under active development.`);
    console.log("AI Automation Generation Requested:", aiDescription);
    // Mock AI response for UI demonstration
    setAiSuggestedConfig(JSON.stringify({
        name: `AI: ${aiDescription.substring(0,20)}...`,
        isEnabled: true,
        trigger: { type: AutomationTriggerType.MANUAL, config: {} },
        action: { type: AutomationActionType.LOG_MESSAGE, config: { logMessage: `AI generated this based on: ${aiDescription.substring(0,30)}...` }}
    }, null, 2));
  }, [aiDescription, addNotification]);

  const handleExportAutomations = () => {
    const exportData: FullExportedAutomations = { automations };
    const jsonString = `data:text/json;charset=utf-8,${encodeURIComponent(JSON.stringify(exportData, null, 2))}`;
    const link = document.createElement("a");
    link.href = jsonString;
    link.download = `chimeradash_automations_${new Date().toISOString().split('T')[0]}.json`;
    link.click();
    addNotification('success', 'All automations exported!');
  };

  const handleConfirmImport = () => {
    if (!jsonToImport.trim()) {
      addNotification('error', 'No JSON data to import.');
      return;
    }
    try {
      const imported = JSON.parse(jsonToImport) as FullExportedAutomations;
      if (imported.automations && Array.isArray(imported.automations)) {
        if (confirm("Importing automations will REPLACE all current automations. Are you sure?")) {
          setAutomations(imported.automations.map(auto => ({...auto, executionLogs: auto.executionLogs || []}))); // Ensure logs array exists
          addNotification('success', 'Automations imported successfully!');
          setJsonImportModalOpen(false);
          setJsonToImport('');
        }
      } else {
        addNotification('error', 'Invalid automations file format. Root "automations" array missing.');
      }
    } catch (err) {
      addNotification('error', `Failed to parse JSON automations: ${err instanceof Error ? err.message : 'Unknown error'}.`);
      console.error("Automations import error:", err);
    }
  };


  return (
    <div className="space-y-6">
      <header className="text-center">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-purple-500 via-pink-500 to-red-600">
          Automation Nexus <span className="text-sm align-super bg-red-600 text-white px-1 rounded">Redline</span>
        </h1>
        <p className="text-[var(--theme-text-secondary)] mt-1 text-md">Orchestrate. Integrate. Dominate. <code className="text-xs text-pink-400">{"{{UNLIMITED_AUTOMATION}}"}</code> Protocol Active.</p>
      </header>

      <div className="flex flex-wrap justify-between items-center gap-3">
        <div className="flex space-x-2">
           <button onClick={handleExportAutomations} className="px-3 py-1.5 bg-green-700 hover:bg-green-800 text-white text-xs font-semibold rounded-md shadow flex items-center space-x-1.5">
             <ArrowDownTrayIconInternal className="w-4 h-4"/> <span>Export All</span>
           </button>
           <button onClick={() => setJsonImportModalOpen(true)} className="px-3 py-1.5 bg-blue-700 hover:bg-blue-800 text-white text-xs font-semibold rounded-md shadow flex items-center space-x-1.5">
            <ArrowUpTrayIconInternal className="w-4 h-4"/> <span>Import</span>
           </button>
        </div>
        <button onClick={() => handleOpenModal()} className="px-4 py-2 bg-[var(--theme-accent-primary)] hover:opacity-90 text-[var(--theme-button-primary-text)] font-semibold rounded-md shadow-lg flex items-center space-x-2 transition-transform hover:scale-105">
          <PlusIconInternal className="w-5 h-5"/> <span>Craft New Automation</span>
        </button>
      </div>

      {automations.length === 0 ? (
        <div className="text-center py-12 bg-[var(--theme-card-bg)]/70 rounded-xl shadow-inner">
          <BoltIconInternal className="w-20 h-20 mx-auto text-slate-600 mb-5" />
          <p className="text-xl text-[var(--theme-text-secondary)]">The Nexus is quiet. No automations yet.</p>
          <p className="text-slate-500">Forge your first automated workflow!</p>
        </div>
      ) : (
        <div className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-6">
          {automations.map(auto => (
            <div key={auto.id} className={`p-5 rounded-xl shadow-xl flex flex-col justify-between transition-all duration-300 border ${auto.isEnabled ? 'bg-[var(--theme-card-bg)] border-[var(--theme-border-primary)] hover:border-pink-500/70 hover:shadow-pink-500/20' : 'bg-[var(--theme-card-bg)]/60 border-[var(--theme-border-primary)]/50 opacity-70'}`}>
              <div>
                <div className="flex justify-between items-start mb-3">
                  <h3 className="text-xl font-semibold text-pink-400 break-all mr-2">{auto.name}</h3>
                  <div className="flex items-center space-x-1">
                    <button onClick={() => handleToggleEnable(auto.id)} title={auto.isEnabled ? 'Disable Automation' : 'Enable Automation'} className={`p-1.5 rounded-full transition-colors ${auto.isEnabled ? 'text-green-400 hover:text-green-300 hover:bg-[var(--theme-bg-accent)]/50' : 'text-red-500 hover:text-red-400 hover:bg-[var(--theme-bg-accent)]/50'}`}> <PowerIconInternal className="w-5 h-5"/> </button>
                  </div>
                </div>
                {auto.description && <p className="text-sm text-[var(--theme-text-secondary)] mb-4 h-10 overflow-hidden italic">{auto.description}</p>}
                <div className="text-xs space-y-1.5 mb-4 border-t border-b border-[var(--theme-border-primary)] py-3">
                    <div className="flex items-center space-x-2"><span className="text-[var(--theme-text-secondary)] w-16">Trigger:</span>{getIconForTrigger(auto.trigger.type)} <span className="text-sky-300">{auto.trigger.type.replace(/_/g, ' ')}</span> {auto.trigger.config.eventSource && <span className="text-purple-300 text-xs">({auto.trigger.config.eventSource.replace(/_/g,' ')})</span>}</div>
                    <div className="flex items-center space-x-2"><span className="text-[var(--theme-text-secondary)] w-16">Action:</span>{getIconForAction(auto.action.type)} <span className="text-sky-300">{auto.action.type.replace(/_/g, ' ')}</span></div>
                    <p><span className="text-[var(--theme-text-secondary)]">Last Run:</span> {auto.lastRun ? new Date(auto.lastRun).toLocaleString() : 'Never'}</p>
                    <p><span className="text-[var(--theme-text-secondary)]">Run Count:</span> {auto.runCount || 0}</p>
                </div>
              </div>
              <div className="flex items-center justify-between mt-auto">
                 <button onClick={() => setExpandedLogAutoId(expandedLogAutoId === auto.id ? null : auto.id)} className="text-xs text-[var(--theme-text-secondary)] hover:text-sky-300 flex items-center">
                     <ListBulletIconInternal className="w-4 h-4 mr-1"/>Logs ({auto.executionLogs.length})
                </button>
                <div className="flex items-center space-x-1.5">
                    { (auto.trigger.type === AutomationTriggerType.MANUAL || auto.trigger.type === AutomationTriggerType.EVENT_BASED ) && (
                        <button onClick={() => handleRunManualAutomation(auto)} disabled={!auto.isEnabled || runningAutomationId === auto.id} className="p-2 text-green-400 hover:text-green-300 rounded-full hover:bg-[var(--theme-bg-accent)]/50 disabled:opacity-50 disabled:cursor-not-allowed flex items-center" title={auto.trigger.type === AutomationTriggerType.MANUAL ? "Run Now" : "Simulate Event & Run"}>
                            {runningAutomationId === auto.id ? <Spinner size="w-5 h-5"/> : <PlayCircleIconInternal className="w-5 h-5"/>}
                        </button>
                    )}
                    <button onClick={() => handleOpenModal(auto)} className="p-2 text-[var(--theme-text-secondary)] hover:text-sky-300 rounded-full hover:bg-[var(--theme-bg-accent)]/50" title="Edit"> <PencilIconInternal className="w-4 h-4"/> </button>
                    <button onClick={() => handleDeleteAutomation(auto.id)} className="p-2 text-[var(--theme-text-secondary)] hover:text-red-400 rounded-full hover:bg-[var(--theme-bg-accent)]/50" title="Delete"> <TrashIconInternal className="w-4 h-4"/> </button>
                </div>
              </div>
              {expandedLogAutoId === auto.id && (
                <div className="mt-3 pt-3 border-t border-[var(--theme-border-primary)] max-h-48 overflow-y-auto text-xs space-y-1.5 pr-1">
                    {auto.executionLogs.length === 0 ? <p className="text-slate-500 italic">No logs yet.</p> : auto.executionLogs.map(log => (
                        <div key={log.id} className={`p-1.5 rounded ${log.status === 'success' ? 'bg-green-800/30' : 'bg-red-800/30'}`}>
                            <p className="font-medium text-slate-300">{new Date(log.timestamp).toLocaleString()} - {log.status.toUpperCase()}</p>
                            <p className="text-slate-400 truncate" title={log.summaryMessage}>{log.summaryMessage}</p>
                            {log.actionOutputSnapshot && <p className="text-slate-500 truncate" title={typeof log.actionOutputSnapshot === 'string' ? log.actionOutputSnapshot : JSON.stringify(log.actionOutputSnapshot)}>Output: {typeof log.actionOutputSnapshot === 'string' ? log.actionOutputSnapshot : JSON.stringify(log.actionOutputSnapshot)}</p>}
                        </div>
                    ))}
                </div>
              )}
            </div>
          ))}
        </div>
      )}

      {showModal && (
        <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-md flex items-center justify-center z-[100] p-4" onClick={() => { setShowModal(false); resetForm();}}>
          <form onSubmit={(e) => { e.preventDefault(); handleSaveAutomation(); }} className="bg-[var(--theme-modal-bg)] p-5 rounded-xl shadow-2xl w-full max-w-2xl space-y-5 border border-[var(--theme-border-primary)] max-h-[90vh] overflow-y-auto" onClick={e => e.stopPropagation()}>
            <h3 className="text-2xl font-semibold text-pink-400 mb-3">{editingAutomation ? 'Edit Automation Orchestration' : 'Craft New Automation'}</h3>
            
            <fieldset className="border border-[var(--theme-border-primary)] p-3 rounded-lg space-y-3">
                <legend className="text-sm font-semibold text-[var(--theme-text-secondary)] px-1">General</legend>
                <div><label htmlFor="autoName" className="block text-sm font-medium text-[var(--theme-text-secondary)] mb-1">Name*</label><input type="text" id="autoName" value={formState.name || ''} onChange={e => handleFormChange('name', e.target.value)} required className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500 text-[var(--theme-input-text)]" /></div>
                <div><label htmlFor="autoDesc" className="block text-sm font-medium text-[var(--theme-text-secondary)] mb-1">Description</label><textarea id="autoDesc" value={formState.description || ''} onChange={e => handleFormChange('description', e.target.value)} rows={2} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500 text-[var(--theme-input-text)]" /></div>
                <div className="flex items-center"><input type="checkbox" id="autoEnabled" checked={formState.isEnabled !== undefined ? formState.isEnabled : true} onChange={e => handleFormChange('isEnabled', e, true)} className="h-4 w-4 text-pink-500 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded focus:ring-pink-400" /><label htmlFor="autoEnabled" className="ml-2 text-sm text-[var(--theme-text-secondary)]">Enabled</label></div>
            </fieldset>

            <fieldset className="border border-[var(--theme-border-primary)] p-3 rounded-lg space-y-3 text-xs">
                <legend className="text-sm font-semibold text-[var(--theme-text-secondary)] px-1">Trigger (<span title="Variables like {{trigger.eventData...}} available in actions"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span>)</legend>
                <select value={formState.trigger?.type || ''} onChange={e => handleFormChange('trigger', { type: e.target.value as AutomationTriggerType, config: {} })} className="w-full p-2 mb-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500 text-[var(--theme-input-text)]">
                    <option value="" disabled>Select Trigger Type...</option>
                    {Object.values(AutomationTriggerType).map(type => <option key={type} value={type}>{type.replace(/_/g, ' ')}</option>)}
                </select>
                {formState.trigger?.type === AutomationTriggerType.TIME_BASED && (
                    <div><label htmlFor="cronExpr" className="block text-[var(--theme-text-secondary)] mb-1">Cron Expression (<a href="https://crontab.guru/" target="_blank" rel="noopener noreferrer" className="text-sky-400 hover:underline">help</a>) <span title="Static value only for Cron"><VariableIconInternal className="w-3 h-3 inline text-slate-500"/></span></label><input type="text" id="cronExpr" value={formState.trigger?.config?.cronExpression || ''} onChange={e => handleFormChange('trigger.config.cronExpression', e.target.value)} placeholder="e.g., 0 9 * * *" className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded font-mono text-[var(--theme-input-text)]" /></div>
                )}
                {formState.trigger?.type === AutomationTriggerType.EVENT_BASED && (
                    <div className="space-y-2">
                        <select value={formState.trigger.config?.eventSource || ''} onChange={e => handleFormChange('trigger.config', { ...formState.trigger?.config, eventSource: e.target.value as AutomationEventSource })} className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]">
                            <option value="">Select Event Source...</option>
                            {Object.values(AutomationEventSource).map(src => <option key={src} value={src}>{src.replace(/_/g, ' ')}</option>)}
                        </select>
                        {renderEventSourceFilters(formState.trigger.config || {}, 'trigger.config')}
                        {(formState.trigger?.config?.eventSource) && (
                            <div className="mt-2 p-2 bg-[var(--theme-input-bg)]/50 rounded">
                                <p className="font-semibold text-[var(--theme-text-secondary)]">Mock Event Data (JSON - for "Simulate & Run"):</p>
                                <textarea 
                                    value={mockEventDataInput}
                                    onChange={(e) => setMockEventDataInput(e.target.value)}
                                    placeholder={'{\n  "itemId": "rag-item-123",\n  "itemName": "My Document",\n  "taskText": "Task Completed!",\n  "eventSpecificData": "..."\n}'}
                                    rows={4}
                                    className="w-full p-1 mt-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded font-mono text-[var(--theme-input-text)]"
                                />
                                <p className="text-slate-500 italic text-xs">This JSON will be available as `trigger.eventData` in your action templates when you click "Simulate Event & Run".</p>
                            </div>
                        )}
                    </div>
                )}
            </fieldset>
            
            <fieldset className="border border-[var(--theme-border-primary)] p-3 rounded-lg space-y-3 text-xs">
                <legend className="text-sm font-semibold text-[var(--theme-text-secondary)] px-1">Action (<span title="Variables like {{now...}}, {{automation...}}, {{actionData...}}, {{trigger...}} available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span>)</legend>
                 <select value={formState.action?.type || ''} onChange={e => handleFormChange('action', { type: e.target.value as AutomationActionType, config: {} })} className="w-full p-2 mb-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded focus:ring-1 focus:ring-pink-500 text-[var(--theme-input-text)]">
                    <option value="" disabled>Select Action Type...</option>
                    {Object.values(AutomationActionType).map(type => <option key={type} value={type}>{type.replace(/_/g, ' ')}</option>)}
                </select>
                 {/* Common config for actions that can output a value */}
                {formState.action?.type && (<div><label className="block text-slate-400 mb-0.5">Store Action Output As (Optional Key) <span title="Name of variable to store this action's result, e.g., mySummary. Usable in subsequent actions as {{actionData.mySummary}}"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action?.config?.actionOutputKey || ''} onChange={e => handleFormChange('action.config.actionOutputKey', e.target.value || undefined)} placeholder="e.g., mySummaryOutput" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>)}
                
                {formState.action?.type === AutomationActionType.LOG_MESSAGE && (<div><label className="block text-[var(--theme-text-secondary)] mb-1">Message <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config?.logMessage || ''} onChange={e => handleFormChange('action.config.logMessage', e.target.value)} rows={2} className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]" /></div>)}
                {formState.action?.type === AutomationActionType.SEND_NOTIFICATION && (<div className="space-y-1.5"><div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Message <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config?.notificationMessage || ''} onChange={e => handleFormChange('action.config.notificationMessage', e.target.value)} rows={2} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]" /></div><div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Type</label><select value={formState.action.config?.notificationType || 'info'} onChange={e => handleFormChange('action.config.notificationType', e.target.value as NotificationItem['type'])} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"><option value="info">Info</option><option value="success">Success</option><option value="error">Error</option><option value="reminder">Reminder</option></select></div></div>)}
                {formState.action?.type === AutomationActionType.RUN_RAG_ANALYSIS && (<div className="space-y-1.5">
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Target Mode <span title="Templateable if EVENT_TRIGGER_ITEM is not chosen"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><select value={formState.action.config?.ragAnalysisTargetMode || 'EVENT_TRIGGER_ITEM'} onChange={e => handleFormChange('action.config.ragAnalysisTargetMode', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"> <option value="EVENT_TRIGGER_ITEM">Event Trigger Item</option> <option value="SPECIFIC_ITEM_ID">Specific Item ID</option> <option value="LATEST_ITEM_BY_TYPE">Latest Item by Type</option></select></div>
                    {formState.action.config?.ragAnalysisTargetMode === 'SPECIFIC_ITEM_ID' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Item ID <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config.ragAnalysisTargetItemId || ''} onChange={e => handleFormChange('action.config.ragAnalysisTargetItemId', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>)}
                     {formState.action.config?.ragAnalysisTargetMode === 'LATEST_ITEM_BY_TYPE' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Item Type</label><select value={formState.action.config.ragAnalysisTargetItemType || ''} onChange={e => handleFormChange('action.config.ragAnalysisTargetItemType', e.target.value as ItemType)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"><option value="">Select Item Type...</option>{Object.values(ItemType).map(it => <option key={it} value={it}>{it}</option>)}</select></div>)}
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Analysis Type</label><select value={formState.action.config?.ragAnalysisType || ''} onChange={e => handleFormChange('action.config.ragAnalysisType', e.target.value as GeminiActionType)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"><option value="">Select Analysis...</option>{Object.values(GeminiActionType).map(act => <option key={act} value={act}>{act.replace(/_/g,' ')}</option>)}</select></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Custom Prompt (Optional - uses <code className="text-xs text-pink-400">{"{{actionData.itemName}}"}</code>, etc.) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config?.ragAnalysisCustomPrompt || ''} onChange={e => handleFormChange('action.config.ragAnalysisCustomPrompt', e.target.value)} rows={2} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                </div>)}
                {formState.action?.type === AutomationActionType.CREATE_PRODUCTIVITY_TASK && (<div className="space-y-1.5">
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Task Text* <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.taskText || ''} onChange={e => handleFormChange('action.config.taskText', e.target.value)} required className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Due Date (YYYY-MM-DD or +/-Xd/w/m/y) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.taskDueDate || ''} onChange={e => handleFormChange('action.config.taskDueDate', e.target.value)} placeholder="e.g., {{now.date}} or +7d" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Due Time (HH:MM) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.taskDueTime || ''} onChange={e => handleFormChange('action.config.taskDueTime', e.target.value)} placeholder="e.g., 17:00 or {{trigger.eventData.time}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Reminder (Mins Before) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.taskReminderMinutesBefore === undefined ? '' : String(formState.action.config?.taskReminderMinutesBefore)} onChange={e => handleFormChange('action.config.taskReminderMinutesBefore', e.target.value ? Number(e.target.value) : undefined)} placeholder="e.g., 15 or {{trigger.eventData.reminderOffset}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Scheduled Date (YYYY-MM-DD or +/-Xd/w/m/y) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.taskScheduledDate || ''} onChange={e => handleFormChange('action.config.taskScheduledDate', e.target.value)} placeholder="e.g., {{now.date}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Priority <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.taskPriority === undefined ? '' : String(formState.action.config.taskPriority)} onChange={e => handleFormChange('action.config.taskPriority', e.target.value ? Number(e.target.value) : undefined)} placeholder="e.g., 1 or {{trigger.eventData.priority}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                </div>)}
                 {formState.action?.type === AutomationActionType.COMPLETE_PRODUCTIVITY_TASK && (<div className="space-y-1.5">
                     <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Task ID <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.completeTaskId || ''} onChange={e => handleFormChange('action.config.completeTaskId', e.target.value)} placeholder="e.g., {{trigger.eventData.taskId}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                     <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">OR Task Name Query (Contains, for first incomplete) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.completeTaskNameQuery || ''} onChange={e => handleFormChange('action.config.completeTaskNameQuery', e.target.value)} placeholder="e.g., Finalize Report" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                </div>)}
                {formState.action?.type === AutomationActionType.CREATE_CALENDAR_EVENT && (<div className="space-y-1.5">
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Event Title* <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.eventTitle || ''} onChange={e => handleFormChange('action.config.eventTitle', e.target.value)} required className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Event Date (YYYY-MM-DD or +/-Xd/w/m/y)* <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.eventDate || ''} onChange={e => handleFormChange('action.config.eventDate', e.target.value)} required placeholder="e.g., {{now.date}}" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Start Time (HH:MM) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.eventStartTime || ''} onChange={e => handleFormChange('action.config.eventStartTime', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">End Time (HH:MM) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.eventEndTime || ''} onChange={e => handleFormChange('action.config.eventEndTime', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Description <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config?.eventDescription || ''} onChange={e => handleFormChange('action.config.eventDescription', e.target.value)} rows={2} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                    <label className="flex items-center space-x-1.5 p-1"><input type="checkbox" checked={!!formState.action.config?.eventAllDay} onChange={e => handleFormChange('action.config.eventAllDay', e, true)} className="h-3 w-3 text-pink-500"/><span>All Day Event</span></label>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Reminder (Mins Before) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.eventReminderMinutesBeforeForCreate === undefined ? '' : String(formState.action.config?.eventReminderMinutesBeforeForCreate)} onChange={e => handleFormChange('action.config.eventReminderMinutesBeforeForCreate', e.target.value ? Number(e.target.value) : undefined)} placeholder="e.g., 30" className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"/></div>
                </div>)}
                {(formState.action?.type === AutomationActionType.STD_UTIL_BASE64_ENCODE || formState.action?.type === AutomationActionType.STD_UTIL_BASE64_DECODE || formState.action?.type === AutomationActionType.STD_UTIL_URL_ENCODE || formState.action?.type === AutomationActionType.STD_UTIL_URL_DECODE || formState.action?.type === AutomationActionType.STD_UTIL_JSON_FORMAT) && (
                    <div><label className="block text-[var(--theme-text-secondary)] mb-1">Input String <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config?.stdUtilInputString || ''} onChange={e => handleFormChange('action.config.stdUtilInputString', e.target.value)} rows={2} className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]" />
                     {formState.action?.type === AutomationActionType.STD_UTIL_JSON_FORMAT && (<div className="mt-1"><label className="block text-[var(--theme-text-secondary)] mb-0.5">Indentation</label><select value={String(formState.action.config.stdUtilJsonIndentation || 2)} onChange={e => handleFormChange('action.config.stdUtilJsonIndentation', e.target.value === 'tab' ? 'tab' : parseInt(e.target.value) as 2|4)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"><option value="2">2 Spaces</option><option value="4">4 Spaces</option><option value="tab">Tabs</option></select></div>)}
                    </div>
                )}
                {formState.action?.type === AutomationActionType.STD_UTIL_GENERATE_UUID && (<p className="text-[var(--theme-text-secondary)] italic">Generates a new UUID. Result available in logs and for chaining (future).</p>)}
                {formState.action?.type === AutomationActionType.INVOKE_GEMINI_CHAT_WITH_RAG_CONTEXT && (<div className="space-y-1.5">
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Chat Prompt Template* <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config?.chatPromptTemplate || ''} onChange={e => handleFormChange('action.config.chatPromptTemplate', e.target.value)} rows={3} required className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]" placeholder="e.g., Summarize the following based on: {{actionData.chatContext}}"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Context Mode</label><select value={formState.action.config?.chatContextMode || 'NONE'} onChange={e => handleFormChange('action.config.chatContextMode', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"><option value="NONE">None</option><option value="EVENT_TRIGGER_ITEM_CONTENT">Event Trigger Item Content</option><option value="ITEM_ID_CONTENT">Specific RAG Item Content</option><option value="CUSTOM_TEXT">Custom Text</option></select></div>
                    {formState.action.config?.chatContextMode === 'ITEM_ID_CONTENT' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Context RAG Item ID <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config.chatContextItemId || ''} onChange={e => handleFormChange('action.config.chatContextItemId', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>)}
                    {formState.action.config?.chatContextMode === 'CUSTOM_TEXT' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Custom Context Text <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={formState.action.config.chatCustomContextText || ''} onChange={e => handleFormChange('action.config.chatCustomContextText', e.target.value)} rows={2} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>)}
                </div>)}
                 {formState.action?.type === AutomationActionType.UPDATE_RAG_ITEM_ANALYSIS && (<div className="space-y-1.5">
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Target Item Mode</label><select value={formState.action.config?.updateTargetItemIdMode || 'EVENT_TRIGGER_ITEM'} onChange={e => handleFormChange('action.config.updateTargetItemIdMode', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]"><option value="EVENT_TRIGGER_ITEM">Event Trigger Item</option><option value="SPECIFIC_ITEM_ID">Specific Item ID</option></select></div>
                    {formState.action.config?.updateTargetItemIdMode === 'SPECIFIC_ITEM_ID' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Item ID <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config.updateTargetItemId || ''} onChange={e => handleFormChange('action.config.updateTargetItemId', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>)}
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Analysis Key to Update* (e.g., SUMMARIZE or custom_key) <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config?.analysisKeyToUpdate || ''} onChange={e => handleFormChange('action.config.analysisKeyToUpdate', e.target.value)} required className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>
                    <div><label className="block text-[var(--theme-text-secondary)] mb-0.5">New Value Mode</label><select value={formState.action.config?.newValueMode || 'STATIC_TEXT'} onChange={e => handleFormChange('action.config.newValueMode', e.target.value)} className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"><option value="STATIC_TEXT">Static Text</option><option value="PREVIOUS_ACTION_RESULT">Previous Action Result</option></select></div>
                    {formState.action.config?.newValueMode === 'STATIC_TEXT' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Static New Value* <span title="Variables available"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><textarea value={Array.isArray(formState.action.config?.staticNewValue) ? formState.action.config.staticNewValue.join(', ') : formState.action.config?.staticNewValue || ''} onChange={e => handleFormChange('action.config.staticNewValue', e.target.value)} rows={2} required className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>)}
                    {formState.action.config?.newValueMode === 'PREVIOUS_ACTION_RESULT' && (<div><label className="block text-[var(--theme-text-secondary)] mb-0.5">Previous Action Result Key* <span title="Key from {{actionData}} to use, e.g., mySummaryOutput"><VariableIconInternal className="w-3 h-3 inline text-pink-400"/></span></label><input type="text" value={formState.action.config.previousActionResultKey || ''} onChange={e => handleFormChange('action.config.previousActionResultKey', e.target.value)} required className="w-full p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded"/></div>)}
                </div>)}
            </fieldset>

            {/* AI Generation Section */}
            {!editingAutomation && (
                <fieldset className="border border-dashed border-pink-500/50 p-3 rounded-lg space-y-3 text-xs">
                    <legend className="text-sm font-semibold text-pink-400 px-1 flex items-center"><SparklesIconInternal className="w-4 h-4 mr-1.5"/>Generate with Redline AI</legend>
                    <div><label htmlFor="aiDesc" className="block text-[var(--theme-text-secondary)] mb-1">Describe your desired automation</label><textarea id="aiDesc" value={aiDescription} onChange={e => setAiDescription(e.target.value)} rows={3} placeholder="e.g., When a new text file is added to RAG, summarize it and send a notification." className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)]" /></div>
                    <button type="button" onClick={handleGenerateWithAI} className="px-3 py-1.5 bg-pink-600 hover:bg-pink-700 text-white text-xs rounded flex items-center"><SparklesIconInternal className="w-3.5 h-3.5 mr-1"/>Generate Suggestion</button>
                    {aiSuggestedConfig && (
                        <div className="mt-2">
                            <p className="font-medium text-[var(--theme-text-secondary)] mb-0.5">AI Suggested Configuration (Read-only):</p>
                            <pre className="p-2 bg-[var(--theme-input-bg)]/50 border border-[var(--theme-input-border)] rounded max-h-32 overflow-y-auto text-[10px]">{aiSuggestedConfig}</pre>
                            <p className="italic text-slate-500 text-[10px] mt-0.5">Copy relevant parts to the fields above if desired. Full AI-to-form population is under development.</p>
                        </div>
                    )}
                </fieldset>
            )}
            
            <div className="flex justify-end space-x-3 pt-3">
              <button type="button" onClick={() => { setShowModal(false); resetForm(); }} className="px-4 py-2 text-[var(--theme-text-secondary)] hover:bg-[var(--theme-bg-accent)]/70 rounded-md transition-colors">Cancel</button>
              <button type="submit" className="px-5 py-2.5 bg-pink-600 hover:bg-pink-700 text-white font-semibold rounded-md shadow-md transition-transform hover:scale-105">{editingAutomation ? 'Save Changes' : 'Create Automation'}</button>
            </div>
          </form>
        </div>
      )}
      {jsonImportModalOpen && (
        <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-[101] p-4" onClick={() => setJsonImportModalOpen(false)}>
          <div className="bg-[var(--theme-modal-bg)] p-5 rounded-xl shadow-2xl w-full max-w-xl space-y-3 border border-sky-500/50" onClick={e => e.stopPropagation()}>
            <h3 className="text-xl font-semibold text-sky-300">Import Automations (JSON)</h3>
            <p className="text-xs text-amber-300">Warning: Importing will replace ALL existing automations.</p>
            <textarea
              value={jsonToImport}
              onChange={e => setJsonToImport(e.target.value)}
              rows={10}
              placeholder="Paste JSON content of automations here..."
              className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded text-[var(--theme-input-text)] font-mono text-xs"
            />
            <div className="flex justify-end space-x-2">
              <button onClick={() => setJsonImportModalOpen(false)} className="px-3 py-1.5 text-slate-300 hover:bg-slate-700 rounded text-xs">Cancel</button>
              <button onClick={handleConfirmImport} className="px-3 py-1.5 bg-sky-600 hover:bg-sky-700 text-white text-xs rounded">Import and Replace</button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default AutomationServiceFeature;
