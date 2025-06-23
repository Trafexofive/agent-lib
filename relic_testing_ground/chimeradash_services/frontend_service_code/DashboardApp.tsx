













import React, { useState, useCallback, useEffect, useRef } from 'react';
import SidebarNav from './components/SidebarNav';
import TopBar from './components/TopBar'; 
import NotificationsPanel from './components/NotificationsPanel'; 
import CommandPalette from './components/CommandPalette';
import RAGAgentFeature from './features/RAGAgentFeature'; 
import ShortcutsFeature from './features/ShortcutsFeature';
import StdUtilsFeature from './features/StdUtilsFeature';
import StdMetricsFeature from './features/StdMetricsFeature';
import AiDataProcessingFeature from './features/AiDataProcessingFeature';
import AutomationServiceFeature from './features/AutomationServiceFeature'; 
import SettingsFeature from './features/SettingsFeature';
import ProductivitySuiteFeature from './features/ProductivitySuiteFeature';
import AlertsContainer from './components/AlertsContainer'; 
import KnowledgeGraphExplorerFeature from './features/KnowledgeGraphExplorerFeature';
import SystemMonitorFeature from './features/SystemMonitorFeature';
import FinancialTrackerFeature from './features/FinancialTrackerFeature';
import ContentPlannerFeature from './features/ContentPlannerFeature';
import SecurityCenterFeature from './features/SecurityCenterFeature';
import { 
    ViewType, 
    AlertMessage, 
    SidebarNavItem, 
    TaskItem, 
    CalendarEvent, 
    NotificationItem, 
    ProductivitySubViewType,
    CommandPaletteAction,
    Theme,
    ThemeColors,
    DashboardSettings,
    HarmCategory,
    HarmBlockThreshold,
    ProductivitySuiteDeepLinkProps 
} from './types';

const baseDefaultInitialSidebarItems: Omit<SidebarNavItem, 'id'>[] = [
  { label: 'OmniHoarder Agent', type: 'VIEW', view: ViewType.RAG_AGENT, iconName: 'ChatBubbleLeftRightIcon' }, 
  { label: 'Productivity Suite', type: 'VIEW', view: ViewType.PRODUCTIVITY_SUITE, iconName: 'RectangleStackIcon' },
  { label: 'Automation Service', type: 'VIEW', view: ViewType.AUTOMATION_SERVICE, iconName: 'BoltIcon'}, 
  { label: 'Shortcuts', type: 'VIEW', view: ViewType.SHORTCUTS, iconName: 'LinkIcon' },
  { label: 'Std::Utils', type: 'VIEW', view: ViewType.STD_UTILS, iconName: 'WrenchScrewdriverIcon' },
  { label: 'Std::Metrics', type: 'VIEW', view: ViewType.STD_METRICS, iconName: 'ChartPieIcon' },
  { label: 'AI Tools', type: 'VIEW', view: ViewType.AI_DATA_PROCESSING, iconName: 'SparklesIcon' }, 
  { label: 'Knowledge Graph', type: 'VIEW', view: ViewType.KNOWLEDGE_GRAPH_EXPLORER, iconName: 'ShareIcon' },
  { label: 'System Monitor', type: 'VIEW', view: ViewType.SYSTEM_MONITOR, iconName: 'CpuChipIcon' },
  { label: 'Finance Tracker', type: 'VIEW', view: ViewType.FINANCIAL_TRACKER, iconName: 'BanknotesIcon' },
  { label: 'Content Planner', type: 'VIEW', view: ViewType.CONTENT_PLANNER, iconName: 'MapIcon' },
  { label: 'Security Center', type: 'VIEW', view: ViewType.SECURITY_CENTER, iconName: 'ShieldCheckIcon' },
  { label: 'Settings', type: 'VIEW', view: ViewType.SETTINGS, iconName: 'CogIcon' },
];

const generateDefaultSidebarItems = (): SidebarNavItem[] =>
    baseDefaultInitialSidebarItems.map(item => ({
        ...item,
        id: item.view ? `view-${item.view}` : crypto.randomUUID()
    }));

const defaultChimeraDarkColors: ThemeColors = {
  bgPrimary: '#0F172A',        
  bgSecondary: '#1E293B',      
  bgAccent: '#334155',         
  textPrimary: '#F1F5F9',      
  textSecondary: '#94A3B8',    
  accentPrimary: '#0EA5E9',    
  accentSecondary: '#EC4899',  
  borderPrimary: '#334155',    
  success: '#22C55E',          
  error: '#EF4444',            
  info: '#3B82F6',             
  buttonPrimaryBg: '#0EA5E9',
  buttonPrimaryText: '#FFFFFF',
  sidebarBg: '#1E293B',
  sidebarText: '#F1F5F9',
  sidebarAccent: '#0EA5E9',
  sidebarFontIconSize: '1.25rem', 
  sidebarFontItemTextSize: '0.875rem', 
  topbarBg: '#1E293B', 
  cardBg: '#1E293B',
  modalBg: '#334155cc', 
  inputBg: '#334155',
  inputBorder: '#475569', 
  inputText: '#F1F5F9',
  jsonKey: '#0EA5E9',        
  jsonString: '#EC4899',     
  jsonNumber: '#A78BFA',     
  jsonBoolean: '#F59E0B',    
  jsonNull: '#64748B',       
  jsonBrackets: '#94A3B8',   
  jsonComma: '#94A3B8',      
  jsonColon: '#94A3B8',      
};

const defaultCrimsonForgeColors: ThemeColors = {
  bgPrimary: '#171717',        
  bgSecondary: '#262626',      
  bgAccent: '#404040',         
  textPrimary: '#F5F5F5',      
  textSecondary: '#A3A3A3',    
  accentPrimary: '#DC2626',    
  accentSecondary: '#F97316',  
  borderPrimary: '#525252',    
  success: '#16A34A',          
  error: '#E11D48',            
  info: '#2563EB',             
  buttonPrimaryBg: '#DC2626',
  buttonPrimaryText: '#FFFFFF',
  sidebarBg: '#262626',
  sidebarText: '#F5F5F5',
  sidebarAccent: '#DC2626',
  sidebarFontIconSize: '1.2rem',
  sidebarFontItemTextSize: '0.85rem',
  topbarBg: '#262626',
  cardBg: '#262626',
  modalBg: '#404040cc',
  inputBg: '#404040',
  inputBorder: '#525252',
  inputText: '#F5F5F5',
  jsonKey: '#DC2626',        
  jsonString: '#F97316',     
  jsonNumber: '#FACC15',     
  jsonBoolean: '#A3A3A3',    
  jsonNull: '#737373',       
  jsonBrackets: '#F5F5F5',   
  jsonComma: '#A3A3A3',      
  jsonColon: '#F5F5F5',      
};

const defaultOceanicAbyssColors: ThemeColors = {
  bgPrimary: '#0B1C33',        
  bgSecondary: '#122B47',      
  bgAccent: '#1A3A5E',         
  textPrimary: '#E0F2FE',      
  textSecondary: '#7DD3FC',    
  accentPrimary: '#38BDF8',    
  accentSecondary: '#2DD4BF',  
  borderPrimary: '#294A70',    
  success: '#34D399',          
  error: '#F87171',            
  info: '#60A5FA',             
  buttonPrimaryBg: '#38BDF8',
  buttonPrimaryText: '#0B1C33',
  sidebarBg: '#122B47',
  sidebarText: '#E0F2FE',
  sidebarAccent: '#38BDF8',
  sidebarFontIconSize: '1.3rem',
  sidebarFontItemTextSize: '0.9rem',
  topbarBg: '#122B47',
  cardBg: '#122B47',
  modalBg: '#1A3A5Ecc',
  inputBg: '#1A3A5E',
  inputBorder: '#294A70',
  inputText: '#E0F2FE',
  jsonKey: '#2DD4BF',        
  jsonString: '#38BDF8',     
  jsonNumber: '#A5F3FC',     
  jsonBoolean: '#F472B6',    
  jsonNull: '#67E8F9',       
  jsonBrackets: '#E0F2FE',   
  jsonComma: '#7DD3FC',      
  jsonColon: '#E0F2FE',      
};


const defaultThemes: Theme[] = [
  { name: "Chimera Dark Default", colors: defaultChimeraDarkColors },
  { name: "Crimson Forge", colors: defaultCrimsonForgeColors },
  { name: "Oceanic Abyss", colors: defaultOceanicAbyssColors }
];

const defaultDashboardSettings: DashboardSettings = {
  dashboardTitle: "ChimeraDash",
  logoBase64: null,
  showLogo: true,
  currentThemeName: defaultThemes[0].name,
  fontFamilyBody: 'Inter, sans-serif', 
  fontFamilyTitle: 'Orbitron, sans-serif',
  customThemes: [],
  recentViews: [],
  defaultViewOnStartup: null, 
  logoMaxWidth: 32, 
  logoMaxHeight: 32,
  defaultTextModel: 'gemini-2.5-flash-preview-04-17',
  defaultImageModel: 'imagen-3.0-generate-002',
  globalSystemInstruction: '',
  aiSafetySettings: [
    { category: HarmCategory.HARM_CATEGORY_HARASSMENT, threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
    { category: HarmCategory.HARM_CATEGORY_HATE_SPEECH, threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
    { category: HarmCategory.HARM_CATEGORY_SEXUALLY_EXPLICIT, threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
    { category: HarmCategory.HARM_CATEGORY_DANGEROUS_CONTENT, threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE },
  ],
  aiThinkingEnabled: true, 
};

const DashboardApp: React.FC = () => {
  const [sidebarNavItems, setSidebarNavItems] = useState<SidebarNavItem[]>([]);
  const [activeView, setActiveView] = useState<ViewType | null>(null);
  
  const [notifications, setNotifications] = useState<NotificationItem[]>(() => {
    const saved = localStorage.getItem('dashboard-notifications');
    try { return saved ? JSON.parse(saved) : []; } catch { return []; }
  });
  const [showNotificationsPanel, setShowNotificationsPanel] = useState(false);
  const [toastAlerts, setToastAlerts] = useState<AlertMessage[]>([]); 
  const [firedReminderIds, setFiredReminderIds] = useState<Set<string>>(new Set());
  
  const [deepLinkProdSuiteSubView, setDeepLinkProdSuiteSubView] = useState<ProductivitySubViewType | undefined>(undefined);
  const [deepLinkProdSuiteItemId, setDeepLinkProdSuiteItemId] = useState<string | undefined>(undefined);

  const [showCommandPalette, setShowCommandPalette] = useState(false);
  const [commandPaletteActions, setCommandPaletteActions] = useState<CommandPaletteAction[]>([]);
  const [commandToExecuteInProdSuite, setCommandToExecuteInProdSuite] = useState<{ actionType: string; payload?: any } | null>(null);
  
  const topBarRef = useRef<HTMLDivElement>(null);
  const [topBarHeight, setTopBarHeight] = useState(64); 

  const [isSidebarOpen, setIsSidebarOpen] = useState<boolean>(() => {
    const savedState = localStorage.getItem('dashboard-sidebarOpen');
    return savedState ? JSON.parse(savedState) : true;
  });
  
  const [dashboardSettings, setDashboardSettings] = useState<DashboardSettings>(() => {
    const savedSettings = localStorage.getItem('dashboard-appSettings');
    let loadedSettings = defaultDashboardSettings;
    if (savedSettings) {
      try {
        const parsed = JSON.parse(savedSettings);
        loadedSettings = { 
            ...defaultDashboardSettings, 
            ...parsed, 
            customThemes: Array.isArray(parsed.customThemes) ? parsed.customThemes : defaultDashboardSettings.customThemes,
            recentViews: Array.isArray(parsed.recentViews) ? parsed.recentViews.filter((view: any) => Object.values(ViewType).includes(view as ViewType)) : defaultDashboardSettings.recentViews,
            aiSafetySettings: Array.isArray(parsed.aiSafetySettings) && parsed.aiSafetySettings.length > 0 ? parsed.aiSafetySettings : defaultDashboardSettings.aiSafetySettings,
            aiThinkingEnabled: parsed.aiThinkingEnabled !== undefined ? parsed.aiThinkingEnabled : defaultDashboardSettings.aiThinkingEnabled,
            defaultTextModel: parsed.defaultTextModel || defaultDashboardSettings.defaultTextModel,
            defaultImageModel: parsed.defaultImageModel || defaultDashboardSettings.defaultImageModel,
        };
        loadedSettings.customThemes = loadedSettings.customThemes?.map((customTheme: Theme) => {
            const baseDefault = defaultChimeraDarkColors; 
            return {
                ...customTheme,
                colors: { ...baseDefault, ...customTheme.colors }
            };
        });

      } catch (e) { console.error("Failed to parse dashboard settings, using defaults.", e); }
    }
    return loadedSettings;
  });

  const toggleSidebar = useCallback(() => {
    setIsSidebarOpen(prev => !prev);
  }, []);

  useEffect(() => {
    localStorage.setItem('dashboard-sidebarOpen', JSON.stringify(isSidebarOpen));
  }, [isSidebarOpen]);

  useEffect(() => {
    localStorage.setItem('dashboard-appSettings', JSON.stringify(dashboardSettings));
    
    const allThemes = [...defaultThemes, ...(dashboardSettings.customThemes || [])];
    const selectedTheme = allThemes.find(t => t.name === dashboardSettings.currentThemeName) || defaultThemes[0];
    
    const themeStyleTag = document.getElementById('dynamic-theme-styles');
    if (themeStyleTag) {
      const colors = selectedTheme.colors;
      const fallbackColors = defaultChimeraDarkColors; 

      themeStyleTag.innerHTML = `
        :root {
          --theme-bg-primary: ${colors.bgPrimary || fallbackColors.bgPrimary};
          --theme-bg-secondary: ${colors.bgSecondary || fallbackColors.bgSecondary};
          --theme-bg-accent: ${colors.bgAccent || fallbackColors.bgAccent};
          --theme-text-primary: ${colors.textPrimary || fallbackColors.textPrimary};
          --theme-text-secondary: ${colors.textSecondary || fallbackColors.textSecondary};
          --theme-accent-primary: ${colors.accentPrimary || fallbackColors.accentPrimary};
          --theme-accent-secondary: ${colors.accentSecondary || fallbackColors.accentSecondary};
          --theme-border-primary: ${colors.borderPrimary || fallbackColors.borderPrimary};
          --theme-success: ${colors.success || fallbackColors.success};
          --theme-error: ${colors.error || fallbackColors.error};
          --theme-info: ${colors.info || fallbackColors.info};

          --theme-button-primary-bg: ${colors.buttonPrimaryBg || colors.accentPrimary || fallbackColors.accentPrimary};
          --theme-button-primary-text: ${colors.buttonPrimaryText || colors.textPrimary || fallbackColors.textPrimary};
          
          --theme-sidebar-bg: ${colors.sidebarBg || colors.bgSecondary || fallbackColors.bgSecondary};
          --theme-sidebar-text: ${colors.sidebarText || colors.textPrimary || fallbackColors.textPrimary};
          --theme-sidebar-accent: ${colors.sidebarAccent || colors.accentPrimary || fallbackColors.accentPrimary};
          --theme-sidebar-font-icon-size: ${colors.sidebarFontIconSize || fallbackColors.sidebarFontIconSize};
          --theme-sidebar-font-item-text-size: ${colors.sidebarFontItemTextSize || fallbackColors.sidebarFontItemTextSize};
          
          --theme-topbar-bg: ${colors.topbarBg || colors.bgSecondary || fallbackColors.bgSecondary};
          --theme-card-bg: ${colors.cardBg || colors.bgSecondary || fallbackColors.bgSecondary};
          --theme-modal-bg: ${colors.modalBg || colors.bgAccent || fallbackColors.bgAccent};
          --theme-input-bg: ${colors.inputBg || colors.bgAccent || fallbackColors.bgAccent};
          --theme-input-border: ${colors.inputBorder || colors.borderPrimary || fallbackColors.borderPrimary};
          --theme-input-text: ${colors.inputText || colors.textPrimary || fallbackColors.textPrimary};

          --theme-json-key: ${colors.jsonKey || fallbackColors.jsonKey};
          --theme-json-string: ${colors.jsonString || fallbackColors.jsonString};
          --theme-json-number: ${colors.jsonNumber || fallbackColors.jsonNumber};
          --theme-json-boolean: ${colors.jsonBoolean || fallbackColors.jsonBoolean};
          --theme-json-null: ${colors.jsonNull || fallbackColors.jsonNull};
          --theme-json-brackets: ${colors.jsonBrackets || fallbackColors.jsonBrackets};
          --theme-json-comma: ${colors.jsonComma || fallbackColors.jsonComma};
          --theme-json-colon: ${colors.jsonColon || fallbackColors.jsonColon};

          --theme-font-family-body: ${dashboardSettings.fontFamilyBody || 'sans-serif'};
          --theme-font-family-title: ${dashboardSettings.fontFamilyTitle || 'sans-serif'};
        }
        body {
          font-family: var(--theme-font-family-body);
        }
        h1, h2, h3, h4, h5, h6 {
          font-family: var(--theme-font-family-title);
        }
      `;
    }
  }, [dashboardSettings]);


  useEffect(() => {
    if (topBarRef.current) {
      setTopBarHeight(topBarRef.current.offsetHeight);
      const resizeObserver = new ResizeObserver(() => {
        if (topBarRef.current) setTopBarHeight(topBarRef.current.offsetHeight);
      });
      resizeObserver.observe(topBarRef.current);
      return () => resizeObserver.disconnect();
    }
  }, []);

  useEffect(() => {
    localStorage.setItem('dashboard-notifications', JSON.stringify(notifications));
  }, [notifications]);

  const addToastAlert = useCallback((type: AlertMessage['type'], message: string) => {
    const id = crypto.randomUUID();
    setToastAlerts(prevAlerts => [...prevAlerts, { id, type, message }].slice(-3)); 
    setTimeout(() => {
      setToastAlerts(prevAlerts => prevAlerts.filter(alert => alert.id !== id));
    }, 5000); 
  }, []);

  const removeToastAlert = useCallback((id: string) => {
    setToastAlerts(prevAlerts => prevAlerts.filter(alert => alert.id !== id));
  }, []);

  const addNotification = useCallback((
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
      sourceSubView?: ProductivitySubViewType; 
      sourceItemId?: string;
    }
  ) => {
    const newNotification: NotificationItem = {
      id: crypto.randomUUID(), type, message,
      timestamp: new Date().toISOString(), read: false, ...sourceDetails,
    };
    setNotifications(prev => [newNotification, ...prev].slice(0, 100)); 

    if (!showNotificationsPanel) { 
      addToastAlert(type === 'reminder' ? 'info' : type, message);
    }
  }, [showNotificationsPanel, addToastAlert]);

  const handleMarkNotificationAsReadToggle = useCallback((notificationId: string, currentReadStatus: boolean) => {
    setNotifications(prev => 
      prev.map(n => n.id === notificationId ? { ...n, read: !currentReadStatus } : n)
    );
  }, []);

  const handleMarkAllNotificationsAsRead = useCallback(() => {
    setNotifications(prev => prev.map(n => ({ ...n, read: true })));
  }, []);

  const handleDismissNotification = useCallback((notificationId: string) => {
    setNotifications(prev => prev.filter(n => n.id !== notificationId));
  }, []);

  const handleDismissAllNotifications = useCallback(() => {
    setNotifications([]);
  }, []);

  const toggleNotificationsPanel = useCallback(() => {
    setShowNotificationsPanel(prev => !prev);
  }, []);

  const handleNotificationClick = useCallback((notification: NotificationItem) => {
    handleMarkNotificationAsReadToggle(notification.id, false); 
    if (notification.sourceView) {
      setActiveView(notification.sourceView);
      if (notification.sourceView === ViewType.PRODUCTIVITY_SUITE) {
        setDeepLinkProdSuiteSubView(notification.sourceSubView);
        setDeepLinkProdSuiteItemId(notification.sourceItemId);
      } else {
        setDeepLinkProdSuiteSubView(undefined);
        setDeepLinkProdSuiteItemId(undefined); 
      }
      setTimeout(() => {
        setDeepLinkProdSuiteSubView(undefined);
        setDeepLinkProdSuiteItemId(undefined);
      }, 100); 
    }
    setShowNotificationsPanel(false); 
  }, [handleMarkNotificationAsReadToggle]);

  useEffect(() => {
    // Initialize sidebarNavItems and attempt to set initial activeView only once,
    // or if critical startup settings change.
    const defaultViewItems = generateDefaultSidebarItems();
    let customUrlItems: SidebarNavItem[] = [];
    const loadedNavItemsString = localStorage.getItem('dashboard-sidebarNavItems');

    if (loadedNavItemsString) {
        try {
            const itemsFromStorage = JSON.parse(loadedNavItemsString) as SidebarNavItem[];
            if (Array.isArray(itemsFromStorage)) {
                customUrlItems = itemsFromStorage.filter(
                    item => item.type === 'URL' && item.id && item.label && item.url && item.iconName
                );
            }
        } catch (e) {
            console.error("Error processing custom sidebar URL items from localStorage:", e);
        }
    }

    const combinedItemsMap = new Map<string, SidebarNavItem>();
    defaultViewItems.forEach(item => combinedItemsMap.set(item.id, item));
    customUrlItems.forEach(item => { 
        combinedItemsMap.set(item.id, item);
    });

    let newSidebarNavItems = Array.from(combinedItemsMap.values());

    newSidebarNavItems.sort((a, b) => {
        const orderA = baseDefaultInitialSidebarItems.findIndex(d => d.view === a.view);
        const orderB = baseDefaultInitialSidebarItems.findIndex(d => d.view === b.view);

        if (a.type === 'VIEW' && b.type === 'URL') return -1;
        if (a.type === 'URL' && b.type === 'VIEW') return 1;
        
        if (a.type === 'VIEW' && b.type === 'VIEW') {
            return (orderA === -1 ? Infinity : orderA) - (orderB === -1 ? Infinity : orderB);
        }
        return a.label.localeCompare(b.label);
    });

    setSidebarNavItems(newSidebarNavItems);

    // Initialize activeView only if it's not already set (i.e., on initial load or after settings reset)
    if (activeView === null && newSidebarNavItems.length > 0) {
        let initialActiveViewCandidate: ViewType | null = null;
        const savedActiveView = localStorage.getItem('dashboard-activeView') as ViewType | null;
        const defaultStartupView = dashboardSettings.defaultViewOnStartup;
        
        // Use currentRecentViews from state, not directly from dashboardSettings to avoid loop if this effect had recentViews in deps
        const currentRecentViews = dashboardSettings.recentViews || [];

        if (defaultStartupView && newSidebarNavItems.some(item => item.type === 'VIEW' && item.view === defaultStartupView)) {
            initialActiveViewCandidate = defaultStartupView;
        } else if (savedActiveView && newSidebarNavItems.some(item => item.type === 'VIEW' && item.view === savedActiveView)) {
            initialActiveViewCandidate = savedActiveView;
        } else if (currentRecentViews.length > 0) {
            const validRecentView = currentRecentViews.find(rv => 
                newSidebarNavItems.some(item => item.type === 'VIEW' && item.view === rv)
            );
            if (validRecentView) initialActiveViewCandidate = validRecentView;
        }
        
        if (!initialActiveViewCandidate) {
            const ragAgentViewItem = newSidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.RAG_AGENT);
            const firstInternalViewItem = newSidebarNavItems.find(item => item.type === 'VIEW');
            initialActiveViewCandidate = ragAgentViewItem?.view || firstInternalViewItem?.view || null;
        }
        
        if (initialActiveViewCandidate) {
            setActiveView(initialActiveViewCandidate);
        }
    }
  // This effect should run if defaultViewOnStartup changes, as it influences the *initial* active view.
  // Changes to custom sidebar URLs are handled by SettingsFeature calling setSidebarNavItems, which triggers re-render and this effect.
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dashboardSettings.defaultViewOnStartup]); 


  useEffect(() => {
    if (sidebarNavItems.length > 0) {
        // Persist only custom URL items, as default items are generated.
        localStorage.setItem('dashboard-sidebarNavItems', JSON.stringify(sidebarNavItems.filter(item => item.type === 'URL')));
    }
  }, [sidebarNavItems]);

  useEffect(() => {
    if (activeView) {
      localStorage.setItem('dashboard-activeView', activeView);
      setDashboardSettings(prevSettings => {
        const currentRecents = prevSettings.recentViews || [];
        const updatedRecents = [activeView, ...currentRecents.filter(v => v !== activeView)].slice(0, 5); 
        return { ...prevSettings, recentViews: updatedRecents };
      });
      if (activeView !== ViewType.PRODUCTIVITY_SUITE) {
        setDeepLinkProdSuiteSubView(undefined);
        setDeepLinkProdSuiteItemId(undefined);
      }
    } else { // activeView is null (e.g., current active view was removed or during initial load before one is set)
      // This block tries to set a default active view if none is set and sidebar items are available.
      // This might be redundant if the previous effect handles initial activeView setting completely.
      // However, it also handles cases where activeView becomes null later (e.g., if a view is removed by settings).
      if (sidebarNavItems.length > 0 && sidebarNavItems.some(item => item.type === 'VIEW')) {
        const defaultViewCandidate = dashboardSettings.defaultViewOnStartup || 
                                  sidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.RAG_AGENT)?.view ||
                                  (sidebarNavItems.find(item => item.type === 'VIEW')?.view); 
        if (defaultViewCandidate && activeView !== defaultViewCandidate) { // Avoid setting if already set to this
             setActiveView(defaultViewCandidate);
        }
      } else { // No views available at all
        localStorage.removeItem('dashboard-activeView');
      }
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [activeView, sidebarNavItems]); 

  useEffect(() => {
    const apiKeyExists = typeof process !== 'undefined' && process.env && process.env.API_KEY;
    if (!apiKeyExists) {
        addNotification('error', 'Gemini API Key is not configured. AI features may not work.');
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []); 

  useEffect(() => {
    const checkReminders = () => {
      const now = new Date();
      const tasksString = localStorage.getItem('dashboard-tasks');
      if (tasksString) {
        try {
          const tasks: TaskItem[] = JSON.parse(tasksString).map((t: any) => ({...t, createdAt: new Date(t.createdAt)}));
          tasks.forEach(task => {
            if (!task.completed && task.dueDate && task.reminderMinutesBefore && task.reminderMinutesBefore > 0) {
              if (firedReminderIds.has(`task-${task.id}`)) return;
              let dueDateTimeStr = task.dueDate;
              if (task.dueTime) {
                dueDateTimeStr += `T${task.dueTime}`;
              } else { 
                dueDateTimeStr += `T23:59:59`;
              }
              try {
                  const dueDateTime = new Date(dueDateTimeStr);
                  const reminderTime = new Date(dueDateTime.getTime() - task.reminderMinutesBefore * 60000);
                  if (now >= reminderTime) {
                    addNotification('reminder', `Task "${task.text.substring(0, 30)}..." is due soon!`, {
                      sourceView: ViewType.PRODUCTIVITY_SUITE,
                      sourceSubView: ProductivitySubViewType.TASKS,
                      sourceItemId: task.id,
                    });
                    setFiredReminderIds(prev => new Set(prev).add(`task-${task.id}`));
                  }
              } catch (dateParseError) { console.error("Error parsing task due date for reminder:", task.id, dateParseError); }
            }
          });
        } catch (e) { console.error("Error parsing tasks for reminders:", e); }
      }

      const eventsString = localStorage.getItem('dashboard-calendarEvents');
      if (eventsString) {
        try {
          const events: CalendarEvent[] = JSON.parse(eventsString);
          events.forEach(event => {
            if (event.reminderMinutesBefore && event.reminderMinutesBefore > 0) {
              if (firedReminderIds.has(`event-${event.id}`)) return;
              let eventStartDateTimeStr = event.date;
              if (event.allDay || !event.startTime) {
                eventStartDateTimeStr += `T00:00:00`; 
              } else {
                eventStartDateTimeStr += `T${event.startTime}`;
              }
              try {
                const eventStartDateTime = new Date(eventStartDateTimeStr);
                const reminderTime = new Date(eventStartDateTime.getTime() - event.reminderMinutesBefore * 60000);
                if (now >= reminderTime) {
                  addNotification('reminder', `Event "${event.title.substring(0, 30)}..." is starting soon!`, {
                    sourceView: ViewType.PRODUCTIVITY_SUITE,
                    sourceSubView: ProductivitySubViewType.CALENDAR,
                    sourceItemId: event.id,
                  });
                  setFiredReminderIds(prev => new Set(prev).add(`event-${event.id}`));
                }
              } catch (dateParseError) { console.error("Error parsing event start date for reminder:", event.id, dateParseError); }
            }
          });
        } catch (e) { console.error("Error parsing calendar events for reminders:", e); }
      }
    };

    const intervalId = setInterval(checkReminders, 30 * 1000); 
    checkReminders(); 
    return () => clearInterval(intervalId);
  }, [addNotification, firedReminderIds]);

  const toggleCommandPalette = useCallback(() => {
    setShowCommandPalette(prev => !prev);
  }, []);

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key === 'k') {
        event.preventDefault();
        toggleCommandPalette();
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [toggleCommandPalette]);

  useEffect(() => {
    const navActions: CommandPaletteAction[] = sidebarNavItems.map(item => ({
      id: item.id,
      label: item.type === 'VIEW' ? `Go to ${item.label}` : `Open ${item.label}`,
      type: 'navigate',
      iconName: item.iconName,
      section: 'Navigation',
      keywords: [item.label, ...(item.view ? [item.view] : [])],
      actionDetail: {
        view: item.view,
        url: item.url,
      },
    }));

    const prodSuiteActions: CommandPaletteAction[] = [
      { id: 'timer-start-work', label: 'Timer: Start Work Session', type: 'action', iconName: 'ClockIcon', section: 'Productivity Suite', keywords: ['pomodoro', 'work', 'focus', 'timer'], actionDetail: { subView: ProductivitySubViewType.TIMER, subViewAction: 'TIMER_START_WORK' } },
      { id: 'timer-start-short-break', label: 'Timer: Start Short Break', type: 'action', iconName: 'ClockIcon', section: 'Productivity Suite', keywords: ['pomodoro', 'short', 'break', 'timer'], actionDetail: { subView: ProductivitySubViewType.TIMER, subViewAction: 'TIMER_START_SHORT_BREAK' } },
      { id: 'timer-start-long-break', label: 'Timer: Start Long Break', type: 'action', iconName: 'ClockIcon', section: 'Productivity Suite', keywords: ['pomodoro', 'long', 'break', 'timer'], actionDetail: { subView: ProductivitySubViewType.TIMER, subViewAction: 'TIMER_START_LONG_BREAK' } },
      { id: 'timer-pause-resume', label: 'Timer: Pause/Resume', type: 'action', iconName: 'ClockIcon', section: 'Productivity Suite', keywords: ['pomodoro', 'pause', 'resume', 'timer'], actionDetail: { subView: ProductivitySubViewType.TIMER, subViewAction: 'TIMER_PAUSE_RESUME' } },
      { id: 'timer-reset', label: 'Timer: Reset', type: 'action', iconName: 'ClockIcon', section: 'Productivity Suite', keywords: ['pomodoro', 'reset', 'timer'], actionDetail: { subView: ProductivitySubViewType.TIMER, subViewAction: 'TIMER_RESET' } },
      { id: 'tasks-focus-new', label: 'Tasks: Add New Task', type: 'action', iconName: 'ClipboardDocumentCheckIcon', section: 'Productivity Suite', keywords: ['task', 'todo', 'new', 'add'], actionDetail: { subView: ProductivitySubViewType.TASKS, subViewAction: 'TASKS_FOCUS_NEW_INPUT' } },
    ];
    
    setCommandPaletteActions([...navActions, ...prodSuiteActions]);
  }, [sidebarNavItems]);

  const executeCommandPaletteAction = useCallback((action: CommandPaletteAction) => {
    if (action.type === 'navigate') {
      if (action.actionDetail?.view) {
        setActiveView(action.actionDetail.view);
      } else if (action.actionDetail?.url) {
        window.open(action.actionDetail.url, '_blank', 'noopener,noreferrer');
      }
    } else if (action.type === 'action' && action.actionDetail?.subViewAction) {
        if (activeView !== ViewType.PRODUCTIVITY_SUITE) { 
            setActiveView(ViewType.PRODUCTIVITY_SUITE);
        }
        if (action.actionDetail.subView) { 
            setDeepLinkProdSuiteSubView(action.actionDetail.subView);
        }
        setCommandToExecuteInProdSuite({ 
            actionType: action.actionDetail.subViewAction, 
            payload: action.actionDetail.payload 
        });
        setTimeout(() => {
          setCommandToExecuteInProdSuite(null);
           if(action.actionDetail.subView) setDeepLinkProdSuiteSubView(undefined);
        }, 50);
    }
    setShowCommandPalette(false); 
  }, [activeView]);

  const activeViewItem = sidebarNavItems.find(item => item.type === 'VIEW' && item.view === activeView);
  const activeViewLabel = activeViewItem ? activeViewItem.label : "Dashboard";
  const unreadNotificationsCount = notifications.filter(n => !n.read).length;

  const renderView = () => {
    if (!activeView && sidebarNavItems.length > 0 && sidebarNavItems.some(item => item.type === 'VIEW')) {
        const defaultCandidate = dashboardSettings.defaultViewOnStartup || 
                                (dashboardSettings.recentViews && dashboardSettings.recentViews.length > 0 && sidebarNavItems.some(item => item.type === 'VIEW' && item.view === dashboardSettings.recentViews![0])
                                    ? dashboardSettings.recentViews![0]
                                    : sidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.RAG_AGENT)?.view 
                                      || (sidebarNavItems.find(item => item.type === 'VIEW')?.view)); 
        
        if(defaultCandidate) {
            setActiveView(defaultCandidate); 
            return <div className="p-10 text-center text-[var(--theme-text-secondary)]">Loading default view...</div>;
        }
    }
     if (!activeView) { 
        return (
            <div className="text-center p-10 text-[var(--theme-text-secondary)]">
                <h2 className="text-2xl font-semibold mb-4">Welcome to your {dashboardSettings.dashboardTitle}!</h2>
                <p>Please select a feature from the sidebar or use Ctrl+K to get started.</p>
                {(sidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.SETTINGS)) && (
                     <button 
                        onClick={() => setActiveView(ViewType.SETTINGS)} 
                        className="mt-4 px-4 py-2 bg-[var(--theme-accent-primary)] hover:opacity-80 text-[var(--theme-button-primary-text)] font-semibold rounded-md"
                     >
                        Go to Settings
                     </button>
                )}
            </div>
        );
    }

    const commonProps = { addNotification, dashboardSettings }; 
    const productivityProps: ProductivitySuiteDeepLinkProps & typeof commonProps & { commandToExecuteFromPalette?: any } = { 
        ...commonProps, 
        deepLinkSubView: deepLinkProdSuiteSubView, 
        deepLinkItemId: deepLinkProdSuiteItemId,
        commandToExecuteFromPalette: commandToExecuteInProdSuite,
    };

    switch (activeView) {
      case ViewType.RAG_AGENT: return <RAGAgentFeature {...commonProps} />; 
      case ViewType.PRODUCTIVITY_SUITE: return <ProductivitySuiteFeature {...productivityProps} />;
      case ViewType.AUTOMATION_SERVICE: return <AutomationServiceFeature {...commonProps} />; 
      case ViewType.SHORTCUTS: return <ShortcutsFeature {...commonProps} />;
      case ViewType.STD_UTILS: return <StdUtilsFeature {...commonProps} />;
      case ViewType.STD_METRICS: return <StdMetricsFeature {...commonProps} />;
      case ViewType.AI_DATA_PROCESSING: return <AiDataProcessingFeature {...commonProps} />;
      case ViewType.KNOWLEDGE_GRAPH_EXPLORER: return <KnowledgeGraphExplorerFeature {...commonProps} />;
      case ViewType.SYSTEM_MONITOR: return <SystemMonitorFeature {...commonProps} />;
      case ViewType.FINANCIAL_TRACKER: return <FinancialTrackerFeature {...commonProps} />;
      case ViewType.CONTENT_PLANNER: return <ContentPlannerFeature {...commonProps} />;
      case ViewType.SECURITY_CENTER: return <SecurityCenterFeature {...commonProps} />;
      case ViewType.SETTINGS: return <SettingsFeature sidebarNavItems={sidebarNavItems} setSidebarNavItems={setSidebarNavItems} dashboardSettings={dashboardSettings} setDashboardSettings={setDashboardSettings} availableThemes={defaultThemes} addNotification={addNotification} />;
      default:
        const defaultViewCandidate = dashboardSettings.defaultViewOnStartup ||
                                    (dashboardSettings.recentViews && dashboardSettings.recentViews.length > 0 && sidebarNavItems.some(item => item.type === 'VIEW' && item.view === dashboardSettings.recentViews![0])
                                        ? dashboardSettings.recentViews![0]
                                        : sidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.RAG_AGENT)?.view 
                                          || sidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.SETTINGS)?.view  
                                          || (sidebarNavItems.find(item => item.type === 'VIEW')?.view)); 
        
        addNotification('error', `Invalid view type: ${activeView}. Defaulting to ${defaultViewCandidate || 'Settings'}.`);
        
        if (defaultViewCandidate) setActiveView(defaultViewCandidate);
        else if (sidebarNavItems.find(item => item.type === 'VIEW' && item.view === ViewType.SETTINGS)) setActiveView(ViewType.SETTINGS);
        else if (sidebarNavItems.length > 0 && sidebarNavItems.find(item => item.type === 'VIEW')?.view) setActiveView(sidebarNavItems.find(item => item.type === 'VIEW')!.view!);

        return <div className="p-10 text-center text-[var(--theme-text-secondary)]">Invalid view detected. Resetting...</div>; 
    }
  };

  const sidebarWidthClass = isSidebarOpen ? 'md:ml-64' : 'md:ml-20'; 

  return (
    <div className="flex h-screen bg-[var(--theme-bg-primary)] text-[var(--theme-text-primary)]">
      <SidebarNav 
        navItems={sidebarNavItems} 
        activeView={activeView} 
        setActiveView={setActiveView} 
        isSidebarOpen={isSidebarOpen}
        toggleSidebar={toggleSidebar}
        dashboardSettings={dashboardSettings}
      />
      <div 
        className={`flex-1 flex flex-col transition-all duration-300 ease-in-out ${sidebarWidthClass}`}
      >
        <div ref={topBarRef}>
          <TopBar 
            activeViewLabel={activeViewLabel} 
            unreadNotificationsCount={unreadNotificationsCount} 
            onToggleNotificationsPanel={toggleNotificationsPanel}
            onToggleCommandPalette={toggleCommandPalette}
            isSidebarOpen={isSidebarOpen}
          />
        </div>
        <main 
          className="flex-1 overflow-y-auto p-4 sm:p-6 lg:p-8" 
          style={{ height: `calc(100vh - ${topBarHeight}px)`}}
        >
          {renderView()}
        </main>
      </div>
      <AlertsContainer alerts={toastAlerts} removeAlert={removeToastAlert} />
      <NotificationsPanel 
        notifications={notifications} 
        showPanel={showNotificationsPanel} 
        onClose={toggleNotificationsPanel} 
        onNotificationClick={handleNotificationClick}
        onMarkReadToggle={handleMarkNotificationAsReadToggle}
        onDismissNotification={handleDismissNotification}
        onMarkAllRead={handleMarkAllNotificationsAsRead}
        onDismissAll={handleDismissAllNotifications}
      />
      {showCommandPalette && (
        <CommandPalette 
          show={showCommandPalette} 
          onClose={toggleCommandPalette} 
          actions={commandPaletteActions}
          onExecuteAction={executeCommandPaletteAction}
        />
      )}
    </div>
  );
};

export default DashboardApp;
