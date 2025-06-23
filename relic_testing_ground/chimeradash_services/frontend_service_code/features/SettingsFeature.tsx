
import React, { useState, useCallback, FC, ChangeEvent, useEffect } from 'react';
import { 
    SidebarNavItem, 
    ViewType, 
    IconName, 
    NotificationItem, 
    DashboardSettings, 
    Theme, 
    ThemeColors,
    FullExportedSettings,
    HarmCategory,      
    HarmBlockThreshold,
    FullExportedAutomations, 
    Automation,
    AggregatedItem, 
    FullExportedRAGItems, 
    ItemType 
} from '../types'; 
import { iconMap } from '../components/SidebarNav';


// --- Internal Icons ---
const PlusIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 4.5v15m7.5-7.5h-15" /></svg>;
const TrashIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12.56 0c1.153 0 2.24.032 3.223.094M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const ArrowDownTrayIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5M16.5 12L12 16.5m0 0L7.5 12m4.5 0V3" /></svg>;
const ArrowUpTrayIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5m-13.5-9L12 3m0 0l4.5 4.5M12 3v13.5" /></svg>;
const InformationCircleIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M11.25 11.25l.041-.02a.75.75 0 011.063.852l-.708 2.836a.75.75 0 001.063.853l.041-.021M21 12a9 9 0 11-18 0 9 9 0 0118 0zm-9-3.75h.008v.008H12V8.25z" /></svg>;
const Cog8ToothIcon = (props: React.SVGProps<SVGSVGElement>) => ( <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M10.343 3.94c.09-.542.56-.94 1.11-.94h1.093c.55 0 1.02.398 1.11.94l.149.894c.07.424.384.764.78.93s.89-.093 1.24-.39l.658-.658c.432-.432 1.143-.432 1.574 0l.775.775a1.113 1.113 0 010 1.574l-.658.658c-.302.301-.52.727-.392 1.24s.504.71.93.78l.893.15c.542.09.94.56.94 1.11v1.093c0 .55-.398 1.02-.94 1.11l-.893.149c-.424.07-.764.383-.93.78s.093.89.392 1.24l.658.658c.432.432.432 1.142 0 1.574l-.775.775a1.113 1.113 0 01-1.574 0l-.658-.658c-.302-.3-.727-.52-.1.24-.39s-.71.504-.78.93l-.15.892c-.09.542-.56.94-1.11.94h-1.093c-.55 0-1.02-.398-1.11-.94l-.149-.894c-.07-.424-.384-.764-.78-.93s-.89.093-1.24.39l-.658.658c-.432-.432-1.143-.432-1.574 0l-.775-.775a1.113 1.113 0 010-1.574l.658-.658c.302-.301-.52-.727-.392-1.24s-.504-.71-.93-.78l-.893-.15c-.542-.09-.94-.56-.94-1.11V9.423c0-.55.398-1.02.94-1.11l.893-.149c.424-.07.764-.383.93-.78s-.093-.89-.392-1.24l-.658-.658c-.432-.432-.432-1.142 0 1.574l.775-.775A1.113 1.113 0 018.128 3l.658.658c.3.302.727.52 1.24.39s.71-.504.78-.93l.15-.892z" /><path strokeLinecap="round" strokeLinejoin="round" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" /></svg>);
const PaintBrushIcon = (props: React.SVGProps<SVGSVGElement>) => (<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9.53 16.122a3 3 0 00-5.78 1.128 2.25 2.25 0 01-2.4 2.245 4.5 4.5 0 008.4-2.245c0-.399-.078-.78-.22-1.128zm0 0a15.998 15.998 0 003.388-1.62m-5.043-.025a15.994 15.994 0 011.622-3.395m3.42 3.42a15.995 15.995 0 004.764-4.648l3.876-5.814a1.151 1.151 0 00-1.597-1.597L14.146 6.32a15.996 15.996 0 00-4.649 4.763m3.42 3.42a6.776 6.776 0 00-3.42-3.42" /></svg>);
const ListBulletIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 6.75h7.5M8.25 12h7.5m-7.5 5.25h7.5M3.75 6.75h.007v.008H3.75V6.75zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 12h.007v.008H3.75V12zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 17.25h.007v.008H3.75v-.008zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0z" /></svg>;
const ServerStackIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6 12L3.269 3.126A59.768 59.768 0 0121.485 12 59.77 59.77 0 013.27 20.876L5.999 12zm0 0h7.5" /></svg>;
const SparklesIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M9.813 15.904L9 18.75l-.813-2.846a4.5 4.5 0 00-3.09-3.09L1.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L9 5.25l.813 2.846a4.5 4.5 0 003.09 3.09L15.75 12l-2.846.813a4.5 4.5 0 00-3.09 3.09zM18.25 12l2.846-.813a4.5 4.5 0 003.09-3.09L24 5.25l-.813 2.846a4.5 4.5 0 00-3.09 3.09L18.25 12zm0 0l-2.846.813a4.5 4.5 0 00-3.09 3.09L12 18.75l.813-2.846a4.5 4.5 0 003.09-3.09L18.25 12z" /></svg>;
const LinkIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M13.19 8.688a4.5 4.5 0 011.242 7.244l-4.5 4.5a4.5 4.5 0 01-6.364-6.364l1.757-1.757m13.35-.622l1.757-1.757a4.5 4.5 0 00-6.364-6.364l-4.5 4.5a4.5 4.5 0 001.242 7.244" /></svg>;

interface SettingsFeatureProps {
  sidebarNavItems: SidebarNavItem[];
  setSidebarNavItems: React.Dispatch<React.SetStateAction<SidebarNavItem[]>>;
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
  dashboardSettings: DashboardSettings;
  setDashboardSettings: React.Dispatch<React.SetStateAction<DashboardSettings>>;
  availableThemes: Theme[]; 
}

type SettingsTab = 'general' | 'appearance' | 'sidebar' | 'ai_config' | 'data' | 'integrations' | 'about';

// LocalStorage keys for granular data clearing
const RAG_ITEMS_KEY = 'rag-agent-items';
const RAG_MOCK_DATA_KEY = 'rag-agent-mock-data-loaded';
const PRODUCTIVITY_TASKS_KEY = 'dashboard-tasks';
const PRODUCTIVITY_CALENDAR_KEY = 'dashboard-calendarEvents';
const PRODUCTIVITY_POMODORO_KEY = 'dashboard-pomodoroSessions';
const NOTIFICATIONS_HISTORY_KEY = 'dashboard-notifications';
const AUTOMATION_STORAGE_KEY = 'dashboard-automations-redline';


const SettingsFeature: FC<SettingsFeatureProps> = ({ 
    sidebarNavItems, 
    setSidebarNavItems, 
    addNotification,
    dashboardSettings,
    setDashboardSettings,
    availableThemes
}) => {
  const [activeTab, setActiveTab] = useState<SettingsTab>('general');
  
  const [newItemLabel, setNewItemLabel] = useState('');
  const [newItemUrl, setNewItemUrl] = useState('');
  const [newItemIconName, setNewItemIconName] = useState<IconName>('ExternalLinkIcon'); 
  const [editingSidebarItemId, setEditingSidebarItemId] = useState<string | null>(null);


  const [showThemeEditor, setShowThemeEditor] = useState(false);
  const [editingThemeName, setEditingThemeName] = useState('');
  const [editingThemeColors, setEditingThemeColors] = useState<Partial<ThemeColors>>({});
  const [isCreatingNewTheme, setIsCreatingNewTheme] = useState(true); 
  const [baseThemeForEditor, setBaseThemeForEditor] = useState<ThemeColors | null>(null);

  const handleSettingsChange = useCallback((key: keyof DashboardSettings, value: any) => {
    setDashboardSettings(prev => ({ ...prev, [key]: value }));
  }, [setDashboardSettings]);

  const handleLogoUpload = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      if (file.size > 500 * 1024) { 
        addNotification('error', 'Logo file size exceeds 500KB limit.', {sourceView: ViewType.SETTINGS});
        return;
      }
      const reader = new FileReader();
      reader.onloadend = () => {
        handleSettingsChange('logoBase64', reader.result as string);
        addNotification('success', 'Logo updated!', {sourceView: ViewType.SETTINGS});
      };
      reader.readAsDataURL(file);
    }
  };

  const handleClearRecentViews = () => {
    if (confirm("Are you sure you want to clear recent views history?")) {
        handleSettingsChange('recentViews', []);
        addNotification('info', 'Recent views history cleared.', {sourceView: ViewType.SETTINGS});
    }
  };

  const allThemes = [...availableThemes, ...(dashboardSettings.customThemes || [])];

  const handleThemeColorChange = (colorKey: keyof ThemeColors, value: string) => {
    setEditingThemeColors(prev => ({...prev, [colorKey]: value }));
  };

  const handleOpenThemeEditor = (themeToEditName?: string) => {
    let baseTheme: Theme | undefined;
    if (themeToEditName) { 
        const themeToEdit = (dashboardSettings.customThemes || []).find(t => t.name === themeToEditName);
        if (!themeToEdit) {
            addNotification('error', "Could not find theme to edit.", {sourceView: ViewType.SETTINGS});
            return;
        }
        baseTheme = themeToEdit;
        setIsCreatingNewTheme(false);
        setEditingThemeName(themeToEdit.name.replace(" (Custom)", "")); 
        setEditingThemeColors(JSON.parse(JSON.stringify(themeToEdit.colors))); 
    } else { 
        baseTheme = allThemes.find(t => t.name === dashboardSettings.currentThemeName) || availableThemes[0];
        setIsCreatingNewTheme(true);
        setEditingThemeName('');
        setEditingThemeColors(JSON.parse(JSON.stringify(baseTheme.colors))); 
    }
    setBaseThemeForEditor(JSON.parse(JSON.stringify(baseTheme.colors)));
    setShowThemeEditor(true);
  };

  const handleSaveTheme = () => {
    if (!editingThemeName.trim()) {
      addNotification('error', "Theme name cannot be empty.", {sourceView: ViewType.SETTINGS});
      return;
    }
    const finalThemeName = `${editingThemeName.trim()} (Custom)`;

    if (isCreatingNewTheme && allThemes.some(t => t.name === finalThemeName)) {
        addNotification('error', `A theme named "${finalThemeName}" already exists. Please choose a different name.`, {sourceView: ViewType.SETTINGS});
        return;
    }
    
    const newCustomTheme: Theme = {
      name: finalThemeName,
      colors: { ...editingThemeColors } as ThemeColors, 
    };

    setDashboardSettings(prev => {
      let updatedCustomThemes = [...(prev.customThemes || [])];
      if (isCreatingNewTheme) {
        updatedCustomThemes.push(newCustomTheme);
      } else {
        updatedCustomThemes = updatedCustomThemes.map(t => t.name === finalThemeName ? newCustomTheme : t);
      }
      return {
        ...prev,
        customThemes: updatedCustomThemes,
        currentThemeName: newCustomTheme.name, 
      };
    });
    addNotification('success', `Theme "${newCustomTheme.name}" ${isCreatingNewTheme ? 'created' : 'updated'} and applied!`, {sourceView: ViewType.SETTINGS});
    setShowThemeEditor(false);
  };

   const handleResetThemeEditor = () => {
    if (baseThemeForEditor) {
        setEditingThemeColors(JSON.parse(JSON.stringify(baseThemeForEditor)));
        addNotification('info', 'Theme editor fields reset to their initial state for this editing session.', {sourceView: ViewType.SETTINGS});
    }
  };

  const handleDeleteCustomTheme = (themeName: string) => {
    if(confirm(`Are you sure you want to delete the custom theme "${themeName}"? This cannot be undone.`)) {
        setDashboardSettings(prev => {
            const updatedCustomThemes = (prev.customThemes || []).filter(t => t.name !== themeName);
            let newCurrentTheme = prev.currentThemeName;
            if(prev.currentThemeName === themeName) { 
                newCurrentTheme = availableThemes[0].name;
            }
            return { ...prev, customThemes: updatedCustomThemes, currentThemeName: newCurrentTheme };
        });
        addNotification('info', `Custom theme "${themeName}" deleted.`, {sourceView: ViewType.SETTINGS});
    }
  };
  
  const handleSidebarItemSubmit = useCallback((e: React.FormEvent) => {
    e.preventDefault();
    if (!newItemLabel.trim() || !newItemUrl.trim()) {
      addNotification('error', 'Label and URL are required.', {sourceView: ViewType.SETTINGS});
      return;
    }
    if (!newItemUrl.match(/^https?:\/\/[^\s/$.?#].[^\s]*$/i)) { 
        addNotification('error', 'Please enter a valid URL (e.g., http://example.com).', {sourceView: ViewType.SETTINGS});
        return;
    }
    if (editingSidebarItemId) { // Editing existing item
        setSidebarNavItems(prevItems => 
            prevItems.map(item => item.id === editingSidebarItemId 
                ? { ...item, label: newItemLabel.trim(), url: newItemUrl.trim(), iconName: newItemIconName } 
                : item
            )
        );
        addNotification('success', `Sidebar link "${newItemLabel.trim()}" updated!`, {sourceView: ViewType.SETTINGS});
    } else { // Adding new item
        const newItem: SidebarNavItem = {
          id: crypto.randomUUID(), label: newItemLabel.trim(), type: 'URL',
          url: newItemUrl.trim(), iconName: newItemIconName, 
        };
        setSidebarNavItems(prevItems => [...prevItems, newItem]);
        addNotification('success', `Sidebar link "${newItem.label}" added!`, {sourceView: ViewType.SETTINGS});
    }
    setNewItemLabel(''); setNewItemUrl(''); setNewItemIconName('ExternalLinkIcon'); setEditingSidebarItemId(null);
  }, [newItemLabel, newItemUrl, newItemIconName, editingSidebarItemId, setSidebarNavItems, addNotification]);

  const handleEditSidebarItem = (itemToEdit: SidebarNavItem) => {
    if (itemToEdit.type === 'URL') {
        setEditingSidebarItemId(itemToEdit.id);
        setNewItemLabel(itemToEdit.label);
        setNewItemUrl(itemToEdit.url || '');
        setNewItemIconName(itemToEdit.iconName);
    } else {
        addNotification('info', 'Built-in view items cannot be edited here.', {sourceView: ViewType.SETTINGS});
    }
  };

  const handleDeleteSidebarItem = useCallback((itemId: string) => {
    const itemToDelete = sidebarNavItems.find(item => item.id === itemId);
    if (itemToDelete) {
      if (itemToDelete.type === 'VIEW') { 
        addNotification('error', 'Built-in view items cannot be removed here.', {sourceView: ViewType.SETTINGS});
        return;
      }
      setSidebarNavItems(prevItems => prevItems.filter(item => item.id !== itemId));
      addNotification('info', `Sidebar link "${itemToDelete.label}" removed.`, {sourceView: ViewType.SETTINGS});
      if (editingSidebarItemId === itemId) { // If deleting the item being edited, reset form
        setNewItemLabel(''); setNewItemUrl(''); setNewItemIconName('ExternalLinkIcon'); setEditingSidebarItemId(null);
      }
    }
  }, [sidebarNavItems, setSidebarNavItems, addNotification, editingSidebarItemId]);

  const handleExportSettings = () => {
    const customURLItems = sidebarNavItems.filter(item => item.type === 'URL');
    const exportData: FullExportedSettings = {
        dashboardSettings: dashboardSettings,
        sidebarNavItems: customURLItems
    };
    const jsonString = `data:text/json;charset=utf-8,${encodeURIComponent(JSON.stringify(exportData, null, 2))}`;
    const link = document.createElement("a");
    link.href = jsonString;
    link.download = `chimeradash_settings_${new Date().toISOString().split('T')[0]}.json`;
    link.click();
    addNotification('success', 'Settings exported!', {sourceView: ViewType.SETTINGS});
  };

  const handleImportSettings = (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        try {
          const importedData = JSON.parse(e.target?.result as string) as FullExportedSettings;
          if (importedData.dashboardSettings && importedData.sidebarNavItems) {
            if (confirm("Importing settings will overwrite your current configuration. Are you sure?")) {
              setDashboardSettings(importedData.dashboardSettings);
              const currentViewItems = sidebarNavItems.filter(item => item.type === 'VIEW');
              setSidebarNavItems([...currentViewItems, ...importedData.sidebarNavItems]);
              addNotification('success', 'Settings imported successfully! The page might reload or require a refresh to apply all changes.', {sourceView: ViewType.SETTINGS});
            }
          } else {
            addNotification('error', 'Invalid settings file format.', {sourceView: ViewType.SETTINGS});
          }
        } catch (err) {
          addNotification('error', 'Failed to parse settings file.', {sourceView: ViewType.SETTINGS});
          console.error("Settings import error:", err);
        }
      };
      reader.readAsText(file);
      if(event.target) event.target.value = ""; 
    }
  };
  
  const clearLocalStorageItem = (key: string, itemName: string, additionalKeys?: string[]) => {
    if (confirm(`Are you sure you want to clear all ${itemName}? This action cannot be undone.`)) {
        localStorage.removeItem(key);
        if (additionalKeys) {
            additionalKeys.forEach(addKey => localStorage.removeItem(addKey));
        }
        addNotification('info', `${itemName} cleared. Please refresh relevant views if needed.`, {sourceView: ViewType.SETTINGS});
    }
  };
  
  const handleFullReset = () => {
    if (confirm("DANGER ZONE! This will reset ALL ChimeraDash settings, data, and custom links to their defaults. Are you absolutely sure?")) {
        localStorage.clear(); // Clears everything for the domain
        addNotification('success', 'Full dashboard reset complete. Please refresh the page.', {sourceView: ViewType.SETTINGS});
        setTimeout(() => window.location.reload(), 1000);
    }
  };
  
  const [apiKeyStatus, setApiKeyStatus] = useState("Checking...");
  useEffect(() => {
    const key = (typeof process !== 'undefined' && process.env && process.env.API_KEY) ? process.env.API_KEY : null;
    setApiKeyStatus(key ? "VALID (Environment Variable)" : "NOT SET / NOT ACCESSIBLE CLIENT-SIDE");
  }, []);

  const handleSafetySettingChange = (category: HarmCategory, threshold: HarmBlockThreshold) => {
    setDashboardSettings(prev => {
        const existingSettings = prev.aiSafetySettings || [];
        const settingIndex = existingSettings.findIndex(s => s.category === category);
        let newSettings;
        if (settingIndex > -1) {
            newSettings = existingSettings.map((s, i) => i === settingIndex ? { ...s, threshold } : s);
        } else {
            newSettings = [...existingSettings, { category, threshold }];
        }
        return { ...prev, aiSafetySettings: newSettings };
    });
  };

  const handleExportAutomations = () => {
    const automationsString = localStorage.getItem(AUTOMATION_STORAGE_KEY);
    if (!automationsString) {
      addNotification('info', 'No automations to export.');
      return;
    }
    try {
      const automationsData: Automation[] = JSON.parse(automationsString);
      const exportData: FullExportedAutomations = { automations: automationsData };
      const jsonString = `data:text/json;charset=utf-8,${encodeURIComponent(JSON.stringify(exportData, null, 2))}`;
      const link = document.createElement("a");
      link.href = jsonString;
      link.download = `chimeradash_automations_${new Date().toISOString().split('T')[0]}.json`;
      link.click();
      addNotification('success', 'Automations exported!', { sourceView: ViewType.SETTINGS });
    } catch (e) {
      addNotification('error', 'Failed to prepare automations for export.');
      console.error("Automation export error:", e);
    }
  };

  const handleImportAutomations = (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        try {
          const importedData = JSON.parse(e.target?.result as string) as FullExportedAutomations;
          if (importedData.automations && Array.isArray(importedData.automations)) {
            if (confirm("Importing automations will REPLACE all current automations. Are you sure?")) {
              localStorage.setItem(AUTOMATION_STORAGE_KEY, JSON.stringify(importedData.automations));
              addNotification('success', 'Automations imported successfully! Refresh Automation Service view if open.', { sourceView: ViewType.SETTINGS });
            }
          } else {
            addNotification('error', 'Invalid automations file format. Root "automations" array missing.');
          }
        } catch (err) {
          addNotification('error', `Failed to parse automations file: ${err instanceof Error ? err.message : 'Unknown error'}.`);
          console.error("Automations import error:", err);
        }
      };
      reader.readAsText(file);
      if(event.target) event.target.value = ""; 
    }
  };
  
  const handleExportRAGItems = () => {
    const ragItemsString = localStorage.getItem(RAG_ITEMS_KEY);
    if (!ragItemsString) {
        addNotification('info', 'No RAG items to export.');
        return;
    }
    try {
        const ragItemsData: AggregatedItem[] = JSON.parse(ragItemsString);
        // Simple export: includes all content, even base64. Could be large.
        // For a more advanced version, consider options for how to handle large content.
        const exportData: FullExportedRAGItems = { ragItems: ragItemsData };
        const jsonString = `data:text/json;charset=utf-8,${encodeURIComponent(JSON.stringify(exportData, null, 2))}`;
        const link = document.createElement("a");
        link.href = jsonString;
        link.download = `chimeradash_rag_items_${new Date().toISOString().split('T')[0]}.json`;
        link.click();
        addNotification('success', 'RAG items exported!', { sourceView: ViewType.SETTINGS });
    } catch (e) {
        addNotification('error', 'Failed to prepare RAG items for export.');
        console.error("RAG export error:", e);
    }
  };

  const handleImportRAGItems = (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
        const reader = new FileReader();
        reader.onload = (e) => {
            try {
                const importedData = JSON.parse(e.target?.result as string) as FullExportedRAGItems;
                if (importedData.ragItems && Array.isArray(importedData.ragItems)) {
                    if (confirm("Importing RAG items will ADD them to your existing vault. Duplicates (by ID) might be overwritten. Continue?")) {
                        const existingItemsString = localStorage.getItem(RAG_ITEMS_KEY);
                        let existingItems: AggregatedItem[] = [];
                        if (existingItemsString) {
                            try { existingItems = JSON.parse(existingItemsString); } catch { /* ignore error, start fresh */ }
                        }
                        
                        const importedItemsMap = new Map(importedData.ragItems.map(item => [item.id, item]));
                        const finalItems = [
                            ...existingItems.filter(item => !importedItemsMap.has(item.id)), // Keep existing items not in import
                            ...importedData.ragItems // Add/overwrite with imported items
                        ];

                        localStorage.setItem(RAG_ITEMS_KEY, JSON.stringify(finalItems));
                        addNotification('success', 'RAG items imported successfully! Refresh RAG Agent view if open.', { sourceView: ViewType.SETTINGS });
                    }
                } else {
                    addNotification('error', 'Invalid RAG items file format. Root "ragItems" array missing.');
                }
            } catch (err) {
                addNotification('error', `Failed to parse RAG items file: ${err instanceof Error ? err.message : 'Unknown error'}.`);
                console.error("RAG import error:", err);
            }
        };
        reader.readAsText(file);
        if (event.target) event.target.value = "";
    }
  };


  const SettingSection: FC<{title: string, children: React.ReactNode}> = ({title, children}) => (
    <section className="bg-[var(--theme-card-bg)] p-5 rounded-xl shadow-lg border border-[var(--theme-border-primary)]">
      <h2 className="text-xl font-semibold text-sky-400 mb-4">{title}</h2>
      <div className="space-y-4">{children}</div>
    </section>
  );
  
  const tabs: { id: SettingsTab; label: string; icon: React.ReactElement }[] = [
    { id: 'general', label: 'General', icon: <Cog8ToothIcon className="w-5 h-5 mr-2"/> },
    { id: 'appearance', label: 'Appearance', icon: <PaintBrushIcon className="w-5 h-5 mr-2"/> },
    { id: 'sidebar', label: 'Sidebar Links', icon: <ListBulletIcon className="w-5 h-5 mr-2"/> },
    { id: 'ai_config', label: 'AI Config', icon: <SparklesIcon className="w-5 h-5 mr-2" /> },
    { id: 'data', label: 'Data Management', icon: <ServerStackIcon className="w-5 h-5 mr-2"/> },
    { id: 'integrations', label: 'Integrations & API', icon: <LinkIcon className="w-5 h-5 mr-2"/> },
    { id: 'about', label: 'About', icon: <InformationCircleIcon className="w-5 h-5 mr-2"/> },
  ];


  const colorFields: (keyof ThemeColors)[] = [
    'bgPrimary', 'bgSecondary', 'bgAccent', 'textPrimary', 'textSecondary', 
    'accentPrimary', 'accentSecondary', 'borderPrimary', 'success', 'error', 'info',
    'buttonPrimaryBg', 'buttonPrimaryText', 'sidebarBg', 'sidebarText', 'sidebarAccent',
    'sidebarFontIconSize', 'sidebarFontItemTextSize', 'topbarBg', 'cardBg', 'modalBg', 
    'inputBg', 'inputBorder', 'inputText',
    'jsonKey', 'jsonString', 'jsonNumber', 'jsonBoolean', 'jsonNull', 
    'jsonBrackets', 'jsonComma', 'jsonColon'
  ];
  
  const availableTextModels = ['gemini-2.5-flash-preview-04-17'];
  const availableImageModels = ['imagen-3.0-generate-002'];
  const allIconNames = Object.keys(iconMap) as IconName[];


  return (
    <div className="space-y-6">
      <header className="text-center">
        <h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-slate-400 via-slate-300 to-slate-200">
          System Configuration
        </h1>
        <p className="text-slate-400 mt-1 text-md">Fine-tune your ChimeraDash experience.</p>
      </header>

      <div className="flex border-b border-[var(--theme-border-primary)] mb-6 overflow-x-auto">
        {tabs.map(tab => (
          <button
            key={tab.id}
            onClick={() => setActiveTab(tab.id)}
            className={`flex items-center px-3 sm:px-4 py-3 text-sm font-medium focus:outline-none transition-colors duration-150 whitespace-nowrap
                        ${activeTab === tab.id 
                            ? 'border-b-2 border-pink-500 text-pink-400' 
                            : 'border-b-2 border-transparent text-slate-400 hover:text-slate-200 hover:border-slate-500'}`}
          >
            {tab.icon} {tab.label}
          </button>
        ))}
      </div>

      {activeTab === 'general' && (
        <SettingSection title="General Settings">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6 text-sm">
            <div>
              <label htmlFor="dashboardTitle" className="block font-medium text-slate-300 mb-1">Dashboard Title</label>
              <input type="text" id="dashboardTitle" value={dashboardSettings.dashboardTitle} onChange={e => handleSettingsChange('dashboardTitle', e.target.value)} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md focus:ring-1 focus:ring-sky-500 text-[var(--theme-input-text)]"/>
            </div>
            <div>
              <label htmlFor="defaultViewOnStartup" className="block font-medium text-slate-300 mb-1">Default View on Startup</label>
              <select id="defaultViewOnStartup" value={dashboardSettings.defaultViewOnStartup || ''} onChange={e => handleSettingsChange('defaultViewOnStartup', e.target.value || null)} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md focus:ring-1 focus:ring-sky-500 text-[var(--theme-input-text)]">
                <option value="">Last Active (or Default)</option>
                {Object.values(ViewType).map(view => <option key={view} value={view}>{view.replace(/_/g, ' ').replace(/\b\w/g, l => l.toUpperCase())}</option>)}
              </select>
            </div>
             <div className="md:col-span-2">
                <label className="block font-medium text-slate-300 mb-1">Logo</label>
                <div className="flex items-center space-x-3 mt-1">
                    <input type="checkbox" id="showLogo" checked={dashboardSettings.showLogo} onChange={e => handleSettingsChange('showLogo', e.target.checked)} className="h-4 w-4 text-sky-500 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded focus:ring-sky-400"/>
                    <label htmlFor="showLogo" className="text-slate-300">Show Logo in Sidebar</label>
                </div>
                {dashboardSettings.showLogo && (
                    <div className="mt-3">
                        <div className="flex items-center space-x-2">
                            <input type="file" id="logoUpload" accept="image/png, image/jpeg, image/svg+xml, image/gif" onChange={handleLogoUpload} className="block w-full text-xs text-slate-400 file:mr-2 file:py-1.5 file:px-2.5 file:rounded-md file:border-0 file:text-xs file:font-semibold file:bg-sky-600 file:text-white hover:file:bg-sky-700"/>
                            {dashboardSettings.logoBase64 && <img src={dashboardSettings.logoBase64} alt="logo preview" className="h-8 w-auto bg-slate-600 p-1 rounded"/>}
                        </div>
                         {dashboardSettings.logoBase64 && <button onClick={() => handleSettingsChange('logoBase64', null)} className="mt-1.5 text-[10px] text-red-400 hover:text-red-300">Remove Logo</button>}
                    </div>
                )}
            </div>
             <div className="md:col-span-2">
                <label className="block font-medium text-slate-300 mb-1">Recent Views History</label>
                 <p className="text-xs text-slate-400 mb-1">Currently showing: {(dashboardSettings.recentViews || []).map(v => v.replace(/_/g, ' ')).join(', ') || 'None'}</p>
                <button onClick={handleClearRecentViews} className="px-3 py-1.5 bg-amber-600 hover:bg-amber-700 text-white text-xs rounded-md">Clear Recent Views</button>
            </div>
          </div>
        </SettingSection>
      )}

      {activeTab === 'appearance' && (
        <SettingSection title="Appearance & Themes">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6 text-sm">
            <div>
              <label htmlFor="currentTheme" className="block font-medium text-slate-300 mb-1">Current Theme</label>
              <select id="currentTheme" value={dashboardSettings.currentThemeName} onChange={e => handleSettingsChange('currentThemeName', e.target.value)} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md focus:ring-1 focus:ring-sky-500 text-[var(--theme-input-text)]">
                {allThemes.map(theme => <option key={theme.name} value={theme.name}>{theme.name}</option>)}
              </select>
            </div>
             <div>
              <label htmlFor="fontFamilyBody" className="block font-medium text-slate-300 mb-1">Body Font Family</label>
              <input type="text" id="fontFamilyBody" value={dashboardSettings.fontFamilyBody} onChange={e => handleSettingsChange('fontFamilyBody', e.target.value)} placeholder="e.g., Inter, sans-serif" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md"/>
             <div className="mt-1 p-2 bg-slate-700/50 rounded text-slate-300" style={{fontFamily: dashboardSettings.fontFamilyBody}}>Font Preview: The quick brown fox jumps over the lazy dog. 1234567890</div>
            </div>
            <div>
              <label htmlFor="fontFamilyTitle" className="block font-medium text-slate-300 mb-1">Title Font Family</label>
              <input type="text" id="fontFamilyTitle" value={dashboardSettings.fontFamilyTitle} onChange={e => handleSettingsChange('fontFamilyTitle', e.target.value)} placeholder="e.g., Orbitron, sans-serif" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md"/>
              <div className="mt-1 p-2 bg-slate-700/50 rounded text-slate-300 text-lg" style={{fontFamily: dashboardSettings.fontFamilyTitle}}>Title Font Preview</div>
            </div>
            <div>
                <label htmlFor="logoMaxWidth" className="block font-medium text-slate-300 mb-1">Logo Max Width (px in Sidebar)</label>
                <input type="number" id="logoMaxWidth" value={dashboardSettings.logoMaxWidth || ''} onChange={e => handleSettingsChange('logoMaxWidth', e.target.value ? parseInt(e.target.value) : undefined)} placeholder="e.g., 32" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md"/>
            </div>
            <div>
                <label htmlFor="logoMaxHeight" className="block font-medium text-slate-300 mb-1">Logo Max Height (px in Sidebar)</label>
                <input type="number" id="logoMaxHeight" value={dashboardSettings.logoMaxHeight || ''} onChange={e => handleSettingsChange('logoMaxHeight', e.target.value ? parseInt(e.target.value) : undefined)} placeholder="e.g., 32" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md"/>
            </div>
            <div>
                <label htmlFor="sidebarFontIconSize" className="block font-medium text-slate-300 mb-1">Sidebar Icon Font Size (CSS)</label>
                <input type="text" id="sidebarFontIconSize" value={dashboardSettings.customThemes?.find(t=>t.name === dashboardSettings.currentThemeName)?.colors.sidebarFontIconSize || editingThemeColors.sidebarFontIconSize || ''} 
                 onChange={e => {
                    const value = e.target.value;
                    // Update the live theme being edited, or the current theme in settings
                    if (showThemeEditor) {
                        handleThemeColorChange('sidebarFontIconSize', value);
                    } else {
                        const currentAppliedTheme = allThemes.find(t => t.name === dashboardSettings.currentThemeName);
                        if(currentAppliedTheme && currentAppliedTheme.name.includes("(Custom)")) {
                            handleSettingsChange('customThemes', dashboardSettings.customThemes?.map(t => t.name === currentAppliedTheme.name ? {...t, colors: {...t.colors, sidebarFontIconSize: value}} : t));
                        } else {
                            addNotification('info', "To customize this, create a new theme based on the current one.", {sourceView: ViewType.SETTINGS});
                        }
                    }
                 }}
                 placeholder="e.g., 1.25rem or 16px" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md"/>
            </div>
             <div>
                <label htmlFor="sidebarFontItemTextSize" className="block font-medium text-slate-300 mb-1">Sidebar Item Text Font Size (CSS)</label>
                <input type="text" id="sidebarFontItemTextSize" value={dashboardSettings.customThemes?.find(t=>t.name === dashboardSettings.currentThemeName)?.colors.sidebarFontItemTextSize || editingThemeColors.sidebarFontItemTextSize || ''} 
                 onChange={e => {
                    const value = e.target.value;
                     if (showThemeEditor) {
                        handleThemeColorChange('sidebarFontItemTextSize', value);
                    } else {
                        const currentAppliedTheme = allThemes.find(t => t.name === dashboardSettings.currentThemeName);
                         if(currentAppliedTheme && currentAppliedTheme.name.includes("(Custom)")) {
                            handleSettingsChange('customThemes', dashboardSettings.customThemes?.map(t => t.name === currentAppliedTheme.name ? {...t, colors: {...t.colors, sidebarFontItemTextSize: value}} : t));
                        } else {
                             addNotification('info', "To customize this, create a new theme based on the current one.", {sourceView: ViewType.SETTINGS});
                        }
                    }
                 }}
                 placeholder="e.g., 0.875rem or 14px" className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md"/>
            </div>

          </div>
          <div className="mt-6">
            <button onClick={() => handleOpenThemeEditor()} className="px-3 py-1.5 bg-teal-600 hover:bg-teal-700 text-white text-xs rounded-md mr-2">Create New Theme</button>
            {(dashboardSettings.customThemes || []).length > 0 && (
                 <p className="text-sm text-slate-300 mt-4 mb-1">Manage Custom Themes:</p>
            )}
            <div className="space-y-1">
                {(dashboardSettings.customThemes || []).map(theme => (
                    <div key={theme.name} className="flex justify-between items-center p-1.5 bg-[var(--theme-input-bg)]/50 rounded text-xs">
                        <span className="text-slate-300">{theme.name}</span> 
                        <div>
                             <button onClick={() => handleOpenThemeEditor(theme.name)} className="text-sky-400 hover:text-sky-300 px-1.5">Edit</button>
                             <button onClick={() => handleDeleteCustomTheme(theme.name)} className="text-red-500 hover:text-red-400 px-1.5"><TrashIcon className="w-3 h-3 inline"/> Delete</button>
                        </div>
                    </div>
                ))}
            </div>
          </div>
            {showThemeEditor && (
                <div className="fixed inset-0 bg-slate-900/80 backdrop-blur-sm flex items-center justify-center z-[101] p-4" onClick={() => setShowThemeEditor(false)}>
                    <div className="bg-[var(--theme-modal-bg)] p-5 rounded-xl shadow-2xl w-full max-w-3xl max-h-[90vh] flex flex-col border border-teal-500/50" onClick={e => e.stopPropagation()}>
                        <h3 className="text-lg font-semibold text-teal-300 mb-3">{isCreatingNewTheme ? 'Create Custom Theme' : `Edit: ${editingThemeName} (Custom)`}</h3>
                        {!isCreatingNewTheme && <p className="text-xs text-amber-300 mb-2">Note: You are editing the theme <strong className="font-medium">"{editingThemeName} (Custom)"</strong>.</p>}
                        <div><label htmlFor="editingThemeNameInput" className="block text-xs font-medium text-slate-300 mb-0.5">Theme Name</label><input type="text" id="editingThemeNameInput" value={editingThemeName} onChange={e => setEditingThemeName(e.target.value)} placeholder="My Awesome Theme" className="w-full p-1.5 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded text-xs"/></div>
                        
                        <div className="mt-3 mb-2 p-3 bg-slate-800/50 border border-slate-700 rounded">
                            <h4 className="text-sm text-slate-300 mb-2">Live Preview Sample:</h4>
                            <button style={{
                                backgroundColor: editingThemeColors.buttonPrimaryBg || 'var(--theme-button-primary-bg)',
                                color: editingThemeColors.buttonPrimaryText || 'var(--theme-button-primary-text)',
                                padding: '6px 12px', borderRadius: '6px', fontSize: '12px'
                            }}>Sample Button</button>
                            <div style={{
                                backgroundColor: editingThemeColors.cardBg || 'var(--theme-card-bg)',
                                color: editingThemeColors.textPrimary || 'var(--theme-text-primary)',
                                border: `1px solid ${editingThemeColors.borderPrimary || 'var(--theme-border-primary)'}`,
                                padding: '8px', borderRadius: '8px', marginTop: '8px', fontSize: '11px'
                            }}>
                                <h5 style={{fontFamily: dashboardSettings.fontFamilyTitle, color: editingThemeColors.accentPrimary || 'var(--theme-accent-primary)'}}>Card Title</h5>
                                <p style={{color: editingThemeColors.textSecondary || 'var(--theme-text-secondary)'}}>Some card content here.</p>
                            </div>
                        </div>
                        
                        <div className="grid grid-cols-2 sm:grid-cols-3 gap-x-4 gap-y-2 overflow-y-auto pr-2 py-2 flex-grow" style={{maxHeight: 'calc(90vh - 250px)'}}>
                        {colorFields.map(key => (
                            <div key={key}>
                                <label htmlFor={`theme-color-${key}`} className="block text-[10px] font-medium text-slate-400 mb-0.5 capitalize">{key.replace(/([A-Z])/g, ' $1').trim()}</label>
                                <input 
                                    type={key.toLowerCase().includes('size') ? "text" : "color"} 
                                    id={`theme-color-${key}`} 
                                    value={editingThemeColors[key] || ''} 
                                    onChange={e => handleThemeColorChange(key, e.target.value)}
                                    className="w-full h-7 p-0.5 text-xs rounded-md border bg-transparent"
                                    style={{
                                        borderColor: editingThemeColors.borderPrimary || 'var(--theme-input-border)',
                                        backgroundColor: key.toLowerCase().includes('size') ? (editingThemeColors.inputBg || 'var(--theme-input-bg)') : (editingThemeColors[key] || 'var(--theme-input-bg)'), 
                                        color: key.toLowerCase().includes('size') ? (editingThemeColors.inputText || 'var(--theme-input-text)') : 'transparent' 
                                    }}
                                    placeholder={key.toLowerCase().includes('size') ? "e.g., 1rem" : ""}
                                />
                            </div>
                        ))}
                        </div>
                        <div className="flex justify-end space-x-2 mt-4 pt-3 border-t border-slate-700">
                            <button type="button" onClick={handleResetThemeEditor} className="px-3 py-1.5 text-slate-300 hover:bg-slate-700 rounded text-xs">Reset Fields</button>
                            <button type="button" onClick={() => setShowThemeEditor(false)} className="px-3 py-1.5 text-slate-300 hover:bg-slate-700 rounded text-xs">Cancel</button>
                            <button type="button" onClick={handleSaveTheme} className="px-4 py-1.5 bg-teal-600 hover:bg-teal-700 text-white text-xs rounded">Save Theme</button>
                        </div>
                    </div>
                </div>
            )}
        </SettingSection>
      )}

      {activeTab === 'sidebar' && (
        <SettingSection title="Customize Sidebar Links (Custom URLs Only)">
            <form onSubmit={handleSidebarItemSubmit} className="space-y-3 p-3 bg-[var(--theme-input-bg)]/30 rounded-lg border border-[var(--theme-border-primary)]/50">
                <h4 className="text-md font-semibold text-sky-300">{editingSidebarItemId ? 'Edit Link' : 'Add New Link'}</h4>
                <div><label htmlFor="newItemLabel" className="block text-xs text-slate-300 mb-0.5">Label*</label><input type="text" id="newItemLabel" value={newItemLabel} onChange={e => setNewItemLabel(e.target.value)} required className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-xs"/></div>
                <div><label htmlFor="newItemUrl" className="block text-xs text-slate-300 mb-0.5">URL*</label><input type="url" id="newItemUrl" value={newItemUrl} onChange={e => setNewItemUrl(e.target.value)} required placeholder="https://example.com" className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-xs"/></div>
                <div><label htmlFor="newItemIcon" className="block text-xs text-slate-300 mb-0.5">Icon</label>
                    <select id="newItemIcon" value={newItemIconName} onChange={e => setNewItemIconName(e.target.value as IconName)} className="w-full p-1.5 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded text-xs">
                        {allIconNames.map(name => <option key={name} value={name}>{name}</option>)}
                    </select>
                </div>
                <div className="flex space-x-2">
                    <button type="submit" className="px-3 py-1.5 bg-sky-600 hover:bg-sky-700 text-white text-xs rounded">{editingSidebarItemId ? 'Save Changes' : 'Add Link'}</button>
                    {editingSidebarItemId && <button type="button" onClick={() => {setEditingSidebarItemId(null); setNewItemLabel(''); setNewItemUrl(''); setNewItemIconName('ExternalLinkIcon');}} className="px-3 py-1.5 bg-slate-600 hover:bg-slate-500 text-white text-xs rounded">Cancel Edit</button>}
                </div>
            </form>
            <div className="mt-4">
                <h4 className="text-md font-semibold text-sky-300 mb-1">Current Custom Links:</h4>
                <ul className="space-y-1 text-xs">
                    {sidebarNavItems.filter(item => item.type === 'URL').map(item => {
                        const IconComp = iconMap[item.iconName] || iconMap.ExternalLinkIcon;
                        return (
                            <li key={item.id} className="flex justify-between items-center p-1.5 bg-[var(--theme-input-bg)]/50 rounded">
                                <div className="flex items-center space-x-2">
                                    <IconComp className="w-4 h-4 text-slate-400"/>
                                    <span className="text-slate-300">{item.label} ({item.url})</span>
                                </div>
                                <div>
                                    <button onClick={() => handleEditSidebarItem(item)} className="text-yellow-400 hover:text-yellow-300 px-1">Edit</button>
                                    <button onClick={() => handleDeleteSidebarItem(item.id)} className="text-red-500 hover:text-red-400 px-1"><TrashIcon className="w-3 h-3 inline"/> Delete</button>
                                </div>
                            </li>
                        );
                    })}
                    {sidebarNavItems.filter(item => item.type === 'URL').length === 0 && <p className="text-slate-400 italic">No custom links added yet.</p>}
                </ul>
            </div>
        </SettingSection>
      )}

      {activeTab === 'ai_config' && (
        <SettingSection title="Global AI Configuration">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-6 text-sm">
                <div>
                    <label htmlFor="defaultTextModel" className="block font-medium text-slate-300 mb-1">Default Text Model</label>
                    <select id="defaultTextModel" value={dashboardSettings.defaultTextModel} onChange={e => handleSettingsChange('defaultTextModel', e.target.value)} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md">
                        {availableTextModels.map(model => <option key={model} value={model}>{model}</option>)}
                    </select>
                    <p className="text-xs text-slate-400 mt-1">Primary model for text generation and analysis.</p>
                </div>
                 <div>
                    <label htmlFor="defaultImageModel" className="block font-medium text-slate-300 mb-1">Default Image Model</label>
                     <select id="defaultImageModel" value={dashboardSettings.defaultImageModel} onChange={e => handleSettingsChange('defaultImageModel', e.target.value)} className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md">
                        {availableImageModels.map(model => <option key={model} value={model}>{model}</option>)}
                    </select>
                    <p className="text-xs text-slate-400 mt-1">Primary model for image generation tasks (if applicable).</p>
                </div>
                <div className="md:col-span-2">
                    <label htmlFor="globalSystemInstruction" className="block font-medium text-slate-300 mb-1">Global System Instruction</label>
                    <textarea id="globalSystemInstruction" value={dashboardSettings.globalSystemInstruction || ''} onChange={e => handleSettingsChange('globalSystemInstruction', e.target.value)} rows={3} placeholder="e.g., You are a helpful assistant specializing in software development." className="w-full p-2 bg-[var(--theme-input-bg)] border border-[var(--theme-input-border)] rounded-md text-sm"/>
                    <p className="text-xs text-slate-400 mt-1">This instruction is prepended to many AI interactions. Use it to define a global persona or context.</p>
                </div>
                <div className="md:col-span-2">
                    <label className="block font-medium text-slate-300 mb-1">AI Thinking Mode <span className="text-xs text-slate-500">(for gemini-2.5-flash-preview-04-17)</span></label>
                    <div className="flex items-center space-x-3 mt-1">
                        <input type="checkbox" id="aiThinkingEnabled" checked={dashboardSettings.aiThinkingEnabled} onChange={e => handleSettingsChange('aiThinkingEnabled', e.target.checked)} className="h-4 w-4 text-sky-500 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded focus:ring-sky-400"/>
                        <label htmlFor="aiThinkingEnabled" className="text-slate-300">Enable Thinking (Higher Quality)</label>
                    </div>
                    <p className="text-xs text-slate-400 mt-1">Disable for lower latency (e.g., game AI opponents). Default is enabled.</p>
                </div>
                 <div className="md:col-span-2 space-y-3">
                    <h4 className="text-md font-medium text-slate-300 border-b border-[var(--theme-border-primary)] pb-1">Content Safety Settings</h4>
                     {Object.values(HarmCategory)
                        .filter(cat => cat !== HarmCategory.HARM_CATEGORY_UNSPECIFIED)
                        .map(category => (
                        <div key={category} className="grid grid-cols-5 gap-2 items-center text-xs">
                            <label htmlFor={`safety-${category}`} className="col-span-2 text-slate-300 capitalize">{category.replace('HARM_CATEGORY_', '').replace(/_/g, ' ').toLowerCase()}</label>
                            <select 
                                id={`safety-${category}`}
                                value={(dashboardSettings.aiSafetySettings.find(s => s.category === category)?.threshold) || HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE}
                                onChange={e => handleSafetySettingChange(category, e.target.value as HarmBlockThreshold)}
                                className="col-span-3 p-1 bg-[var(--theme-input-bg)] border-[var(--theme-input-border)] rounded-md text-slate-200"
                                title={`Threshold for ${category.replace('HARM_CATEGORY_', '').toLowerCase()}`}
                            >
                                {Object.values(HarmBlockThreshold).map(threshold => (
                                    <option key={threshold} value={threshold}>{threshold.replace(/_/g, ' ')}</option>
                                ))}
                            </select>
                        </div>
                     ))}
                     <p className="text-xs text-slate-500 mt-1">Configure how strictly the AI should block potentially harmful content. "Block None" is most permissive.</p>
                </div>
            </div>
        </SettingSection>
      )}
      
      {activeTab === 'data' && (
        <SettingSection title="Data Management">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4 text-sm">
                 <div>
                    <h4 className="text-md font-medium text-slate-300 mb-2">Export Data</h4>
                    <div className="space-y-2">
                        <button onClick={handleExportSettings} className="w-full text-left px-3 py-2 bg-sky-700 hover:bg-sky-800 text-white rounded-md text-xs flex items-center"><ArrowDownTrayIcon className="w-4 h-4 mr-2"/>Export All Settings</button>
                        <button onClick={handleExportAutomations} className="w-full text-left px-3 py-2 bg-sky-700 hover:bg-sky-800 text-white rounded-md text-xs flex items-center"><ArrowDownTrayIcon className="w-4 h-4 mr-2"/>Export Automations</button>
                        <button onClick={handleExportRAGItems} className="w-full text-left px-3 py-2 bg-sky-700 hover:bg-sky-800 text-white rounded-md text-xs flex items-center"><ArrowDownTrayIcon className="w-4 h-4 mr-2"/>Export RAG Items</button>
                    </div>
                </div>
                <div>
                    <h4 className="text-md font-medium text-slate-300 mb-2">Import Data</h4>
                    <div className="space-y-2">
                        <div><label htmlFor="importSettingsFile" className="block w-full text-left px-3 py-2 bg-blue-700 hover:bg-blue-800 text-white rounded-md text-xs cursor-pointer flex items-center"><ArrowUpTrayIcon className="w-4 h-4 mr-2"/>Import Settings <input type="file" id="importSettingsFile" accept=".json" onChange={handleImportSettings} className="hidden"/></label></div>
                        <div><label htmlFor="importAutomationsFile" className="block w-full text-left px-3 py-2 bg-blue-700 hover:bg-blue-800 text-white rounded-md text-xs cursor-pointer flex items-center"><ArrowUpTrayIcon className="w-4 h-4 mr-2"/>Import Automations <input type="file" id="importAutomationsFile" accept=".json" onChange={handleImportAutomations} className="hidden"/></label></div>
                        <div><label htmlFor="importRAGFile" className="block w-full text-left px-3 py-2 bg-blue-700 hover:bg-blue-800 text-white rounded-md text-xs cursor-pointer flex items-center"><ArrowUpTrayIcon className="w-4 h-4 mr-2"/>Import RAG Items <input type="file" id="importRAGFile" accept=".json" onChange={handleImportRAGItems} className="hidden"/></label></div>
                    </div>
                </div>
            </div>
            <div className="mt-6 pt-4 border-t border-dashed border-[var(--theme-border-primary)]">
                <h4 className="text-md font-medium text-amber-400 mb-2">Clear Specific Data</h4>
                <div className="grid grid-cols-1 sm:grid-cols-2 gap-2 text-xs">
                    <button onClick={() => clearLocalStorageItem(RAG_ITEMS_KEY, 'RAG Agent Items', [RAG_MOCK_DATA_KEY])} className="px-3 py-1.5 bg-amber-600 hover:bg-amber-700 text-white rounded-md">Clear RAG Items</button>
                    <button onClick={() => clearLocalStorageItem(AUTOMATION_STORAGE_KEY, 'Automations')} className="px-3 py-1.5 bg-amber-600 hover:bg-amber-700 text-white rounded-md">Clear Automations</button>
                    <button onClick={() => clearLocalStorageItem(PRODUCTIVITY_TASKS_KEY, 'Tasks Data', [PRODUCTIVITY_CALENDAR_KEY, PRODUCTIVITY_POMODORO_KEY])} className="px-3 py-1.5 bg-amber-600 hover:bg-amber-700 text-white rounded-md">Clear Productivity Data</button>
                    <button onClick={() => clearLocalStorageItem(NOTIFICATIONS_HISTORY_KEY, 'Notifications History')} className="px-3 py-1.5 bg-amber-600 hover:bg-amber-700 text-white rounded-md">Clear Notifications</button>
                </div>
            </div>
            <div className="mt-6 pt-4 border-t border-dashed border-red-500/50">
                 <h4 className="text-md font-medium text-red-400 mb-2">Danger Zone</h4>
                <button onClick={handleFullReset} className="w-full px-4 py-2 bg-red-700 hover:bg-red-800 text-white font-bold rounded-md shadow-md border-2 border-red-500 hover:border-red-400 text-sm">FULL DASHBOARD RESET (Delete All Data & Settings)</button>
                <p className="text-xs text-red-300/80 mt-1 text-center">This action is irreversible and will clear all locally stored data for ChimeraDash.</p>
            </div>
        </SettingSection>
      )}

      {activeTab === 'integrations' && (
         <SettingSection title="Integrations & API Management">
            <div className="space-y-3 text-sm">
                <h4 className="text-md font-medium text-slate-300">Gemini API Key Status</h4>
                <p className={`p-2 rounded-md text-xs ${apiKeyStatus.startsWith('VALID') ? 'bg-green-700/30 text-green-300' : 'bg-red-700/30 text-red-300'}`}>
                    Status: <strong>{apiKeyStatus}</strong>
                </p>
                <p className="text-xs text-slate-400">
                    The Gemini API Key is managed via an environment variable (`API_KEY`) and is not configurable through this UI. 
                    Ensure it is correctly set in the environment where ChimeraDash is running for AI features to function.
                </p>
                 <p className="text-xs text-slate-500 mt-4">
                    Future third-party integrations and API access controls will be managed here.
                </p>
            </div>
         </SettingSection>
      )}

      {activeTab === 'about' && (
        <SettingSection title="About ChimeraDash">
            <div className="space-y-3 text-sm text-slate-300">
                <p className="text-xl font-semibold text-sky-300">{dashboardSettings.dashboardTitle} - <span className="text-lg text-pink-400">Ver. Himothy Prime</span></p>
                <p>Your intelligent command center, built with the <strong className="text-pink-400">Himothy Covenant</strong> in mind.</p>
                
                <div className="mt-4 pt-3 border-t border-slate-700">
                    <h4 className="font-medium text-slate-200 mb-1">Core Pillars:</h4>
                    <ul className="list-disc list-inside text-xs text-slate-400 space-y-0.5">
                        <li>AGGREGATE: Consolidate diverse information.</li>
                        <li>ANALYZE: Extract insights with advanced AI.</li>
                        <li>AUGMENT: Enhance productivity and decision-making.</li>
                        <li>AUTOMATE: Orchestrate complex workflows.</li>
                        <li>INTEGRATE: Ensure seamless data and tool flow.</li>
                        <li>DOMINATE: Achieve superior operational awareness.</li>
                    </ul>
                </div>
                <div className="mt-3">
                    {/* In a real app, these would link to actual pages or modals displaying the markdown content */}
                    <button onClick={() => addNotification('info', "Displaying 'Project Vision' (Placeholder Action).")} className="text-sky-400 hover:underline text-xs mr-3">View Project Vision</button>
                    <button onClick={() => addNotification('info', "Displaying 'Features Roadmap' (Placeholder Action).")} className="text-sky-400 hover:underline text-xs">View Features Roadmap</button>
                </div>
                 <div className="mt-4">
                    <button 
                        onClick={() => addNotification('success', "ChimeraDash is up-to-date with the latest Himothy Protocol enhancements!")}
                        className="px-3 py-1.5 bg-teal-600 hover:bg-teal-700 text-white text-xs rounded-md"
                    >
                        Check for Updates
                    </button>
                </div>
                <p className="text-xs text-slate-500 pt-3 border-t border-slate-700 mt-4">
                    &copy; {new Date().getFullYear()} Himothy Command Systems. All rights reserved under the Covenant.
                </p>
            </div>
        </SettingSection>
      )}

    </div>
  );
};

export default SettingsFeature;
