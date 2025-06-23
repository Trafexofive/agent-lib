
import React, { useState, useEffect, useCallback, useRef } from 'react';
import {
    ProductivitySubViewType,
    TaskItem,
    CalendarEvent,
    PomodoroSession,
    TimerMode, 
    ViewType, 
    NotificationItem,
    ProductivitySuiteDeepLinkProps, // For props from DashboardApp
    ProductivityTaskPrefillData    // For prefilling tasks
} from '../types';
import TasksFeature from './TasksFeature';

// --- Pomodoro Timer Constants ---
const WORK_DURATION_SECONDS = 25 * 60;
const SHORT_BREAK_DURATION_SECONDS = 5 * 60;
const LONG_BREAK_DURATION_SECONDS = 15 * 60;
const SESSIONS_BEFORE_LONG_BREAK = 4;

interface ProductivitySuiteFeatureProps extends ProductivitySuiteDeepLinkProps {
  addNotification: (
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
      sourceSubView?: ProductivitySubViewType; 
      sourceItemId?: string;
    }
  ) => void;
  commandToExecuteFromPalette?: { actionType: string; payload?: any } | null;
}

const reminderOptions = [
  { label: 'No reminder', value: 0 }, { label: '5 mins before', value: 5 }, { label: '15 mins before', value: 15 },
  { label: '30 mins before', value: 30 }, { label: '1 hour before', value: 60 }, { label: '1 day before', value: 1440 }, 
];

const getDaysInMonth = (month: number, year: number) => new Date(year, month + 1, 0).getDate();
const getFirstDayOfMonth = (month: number, year: number) => new Date(year, month, 1).getDay(); // 0 for Sunday

type TimelineTaskItem = TaskItem & { isTask: true; date: string };
type TimelineDisplayableItem = CalendarEvent | TimelineTaskItem;


const ProductivitySuiteFeature: React.FC<ProductivitySuiteFeatureProps> = ({ 
    addNotification, 
    deepLinkSubView, 
    deepLinkItemId,
    commandToExecuteFromPalette
}) => {
  const [activeSubView, setActiveSubView] = useState<ProductivitySubViewType>(deepLinkSubView || ProductivitySubViewType.TIMELINE); 

  // Internal navigation/context states
  const [internalNavigateToItemId, setInternalNavigateToItemId] = useState<string | undefined>(deepLinkItemId);
  const [internalNavigateToDateString, setInternalNavigateToDateString] = useState<string | undefined>();
  const [taskDataToPrefill, setTaskDataToPrefill] = useState<ProductivityTaskPrefillData | null>(null);


  // --- Timer State & Logic ---
  const [timeLeft, setTimeLeft] = useState(WORK_DURATION_SECONDS);
  const [isTimerActive, setIsTimerActive] = useState(false);
  const [currentTimerMode, setCurrentTimerMode] = useState<TimerMode>('WORK');
  const [workSessionCount, setWorkSessionCount] = useState(0);
  const [pomodoroSessions, setPomodoroSessions] = useState<PomodoroSession[]>(() => {
    const saved = localStorage.getItem('dashboard-pomodoroSessions');
    try { return saved ? JSON.parse(saved) : []; } catch { return []; }
  });
  const [linkedTaskId, setLinkedTaskId] = useState<string | null>(null);
  const [tasksForLinking, setTasksForLinking] = useState<TaskItem[]>([]);
  const [showNewTaskInputForLink, setShowNewTaskInputForLink] = useState(false);
  const [newTaskForLinkText, setNewTaskForLinkText] = useState('');
  const [scheduleNewLinkedTask, setScheduleNewLinkedTask] = useState(true);
  const timerAudioRef = useRef<HTMLAudioElement | null>(null);
  const timerIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null);
  const timeHiddenRef = useRef<number | null>(null);
  
  // --- Handle deep linking props from DashboardApp ---
  useEffect(() => {
    if (deepLinkSubView) {
      setActiveSubView(deepLinkSubView);
    }
    // This allows DashboardApp to set the item ID for focus, e.g., from a notification click
    if (deepLinkItemId !== undefined) { // Check for undefined to allow clearing
        setInternalNavigateToItemId(deepLinkItemId);
    }
  }, [deepLinkSubView, deepLinkItemId]);


  // --- Command Palette Effect ---
  useEffect(() => {
    if (commandToExecuteFromPalette) {
      const { actionType } = commandToExecuteFromPalette;
      switch (actionType) {
        case 'TIMER_START_WORK': selectTimerModeUI('WORK', true); break;
        case 'TIMER_START_SHORT_BREAK': selectTimerModeUI('SHORT_BREAK', true); break;
        case 'TIMER_START_LONG_BREAK': selectTimerModeUI('LONG_BREAK', true); break;
        case 'TIMER_PAUSE_RESUME': toggleTimer(); break;
        case 'TIMER_RESET': resetTimer(); break;
        case 'TASKS_FOCUS_NEW_INPUT':
          setActiveSubView(ProductivitySubViewType.TASKS);
          // The focusNewTaskInputSignal prop in TasksFeature will handle the focus
          break;
        default:
          console.warn("Unknown command palette action for Productivity Suite:", actionType);
      }
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [commandToExecuteFromPalette]);

  // --- Timer Logic ---
  useEffect(() => { 
    localStorage.setItem('dashboard-pomodoroSessions', JSON.stringify(pomodoroSessions));
  }, [pomodoroSessions]);
  
  useEffect(() => { 
    if (activeSubView === ProductivitySubViewType.TIMER) {
      const savedTasks = localStorage.getItem('dashboard-tasks');
      if (savedTasks) {
        try {
          const allTasks: TaskItem[] = JSON.parse(savedTasks).map((t:any) => ({...t, createdAt: new Date(t.createdAt)}));
          setTasksForLinking(allTasks.filter(task => !task.completed && !task.parentId).sort((a,b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime())); 
        } catch (e) { console.error("Error parsing tasks for linking", e); setTasksForLinking([]);}
      } else {
        setTasksForLinking([]);
      }
    }
  }, [activeSubView, linkedTaskId]); // Re-fetch tasks if linkedTaskId changes or timer view becomes active

  const playNotificationSound = useCallback(() => {
    // The addNotification will be more specific below
    if (timerAudioRef.current) {
        const beepSound = "data:audio/wav;base64,UklGRl9vT19XQVZFZm10IBAAAAABAAEAQB8AAEAfAAABAAgAZGF0YU"+
                          "LjvT9vAP////////////////////////////////////////////////////////////////"+
                          "////////////////////////////////////////////////////////////////"+
                          "////////////////////////////////////////////////////////////////"+
                          "////////////////////////////////////////////////////////////////"+
                          "//8AAADhAAామ//////AAAA//8EADDABAA4AAkAPSX/AD364AA9VKMAPZOrAD0VswA8"+
                          "yrMAPAO7ADsSyQA6mboAOfXNADnwggA50YkAN7nPADbCvQA1n7cAMzhaAC5QnQArVOMA"+
  						"JlNALwh3ACoS0gAkMFwAIghSACH+7AAgDM0AHftxAB0EwwAcGEMAGpGqABc4TwATQWAA"+
  						"EBBhAAwIWgAJNTxABMggAANAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        timerAudioRef.current.src = beepSound;
        timerAudioRef.current.volume = 0.2;
        timerAudioRef.current.play().catch(error => console.warn("Timer audio play failed:", error));
    }
  }, []);

useEffect(() => {
  if (timerIntervalRef.current) {
    clearInterval(timerIntervalRef.current);
    timerIntervalRef.current = null;
  }

  if (isTimerActive && timeLeft > 0 && document.visibilityState === 'visible') {
    timerIntervalRef.current = setInterval(() => {
      setTimeLeft((prevTime) => {
        if (prevTime <= 1) {
          if(timerIntervalRef.current) clearInterval(timerIntervalRef.current);
          timerIntervalRef.current = null;
          return 0;
        }
        return prevTime - 1;
      });
    }, 1000);
  }

  return () => {
    if (timerIntervalRef.current) {
      clearInterval(timerIntervalRef.current);
      timerIntervalRef.current = null;
    }
  };
}, [isTimerActive, timeLeft]); // Removed visibilityState, handled separately


useEffect(() => {
  if (timeLeft === 0 && isTimerActive) {
    playNotificationSound();

    const newSessionLog: PomodoroSession = {
      id: crypto.randomUUID(), date: new Date().toISOString(), type: currentTimerMode,
      duration: currentTimerMode === 'WORK' ? WORK_DURATION_SECONDS : (currentTimerMode === 'SHORT_BREAK' ? SHORT_BREAK_DURATION_SECONDS : LONG_BREAK_DURATION_SECONDS),
    };
    setPomodoroSessions(prev => [newSessionLog, ...prev].slice(0, 200));

    let notificationMessage = "";
    if (linkedTaskId && currentTimerMode === 'WORK') {
      let linkedTaskName = "Linked Task";
      const savedTasks = localStorage.getItem('dashboard-tasks');
      if (savedTasks) { try { const task = JSON.parse(savedTasks).find((t: TaskItem) => t.id === linkedTaskId); if (task) linkedTaskName = task.text; } catch {} }
      notificationMessage = `Work session for "${linkedTaskName.substring(0,30)}..." ended. Consider updating its status.`;
      addNotification('reminder', notificationMessage, { sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TASKS, sourceItemId: linkedTaskId });
    } else if (currentTimerMode !== 'WORK') {
      notificationMessage = `${currentTimerMode.replace('_', ' ')} session ended! Time for work.`;
      addNotification('info', notificationMessage, { sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TIMER });
    } else {
      notificationMessage = `Work session ended! Time for a break.`;
      addNotification('info', notificationMessage, { sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TIMER });
    }
    
    let nextMode: TimerMode;
    let nextTimeLeft: number;

    if (currentTimerMode === 'WORK') {
      const newWorkSessionCount = workSessionCount + 1;
      setWorkSessionCount(newWorkSessionCount);
      nextMode = newWorkSessionCount % SESSIONS_BEFORE_LONG_BREAK === 0 ? 'LONG_BREAK' : 'SHORT_BREAK';
      nextTimeLeft = nextMode === 'LONG_BREAK' ? LONG_BREAK_DURATION_SECONDS : SHORT_BREAK_DURATION_SECONDS;
    } else {
      nextMode = 'WORK';
      nextTimeLeft = WORK_DURATION_SECONDS;
    }
    
    setIsTimerActive(false); // Stop timer for the ended session
    setCurrentTimerMode(nextMode);
    setTimeLeft(nextTimeLeft);
  }
}, [timeLeft, isTimerActive, currentTimerMode, workSessionCount, playNotificationSound, addNotification, linkedTaskId]);

useEffect(() => {
  const handleVisibilityChange = () => {
    if (document.visibilityState === 'hidden') {
      if (isTimerActive && timerIntervalRef.current) { // Timer is running
        clearInterval(timerIntervalRef.current);
        timerIntervalRef.current = null;
        timeHiddenRef.current = Date.now();
      }
    } else if (document.visibilityState === 'visible') {
      if (isTimerActive && timeHiddenRef.current) { // Timer was active and was hidden
        const elapsedWhileHidden = Math.round((Date.now() - timeHiddenRef.current) / 1000);
        timeHiddenRef.current = null;
        
        setTimeLeft(prevTimeLeft => {
          const newTime = Math.max(0, prevTimeLeft - elapsedWhileHidden);
          return newTime;
        });
      }
    }
  };

  document.addEventListener('visibilitychange', handleVisibilityChange);
  return () => {
    document.removeEventListener('visibilitychange', handleVisibilityChange);
    if (timerIntervalRef.current) { 
      clearInterval(timerIntervalRef.current);
      timerIntervalRef.current = null;
    }
  };
}, [isTimerActive]);


  const toggleTimer = () => setIsTimerActive(!isTimerActive);
  const resetTimer = () => { setIsTimerActive(false); setCurrentTimerMode('WORK'); setTimeLeft(WORK_DURATION_SECONDS); setWorkSessionCount(0); setLinkedTaskId(null); setShowNewTaskInputForLink(false); setNewTaskForLinkText(''); };
  const selectTimerModeUI = (newMode: TimerMode, autoStart = false) => { 
    setIsTimerActive(false); // Stop current timer before switching
    setCurrentTimerMode(newMode); 
    setWorkSessionCount(newMode === 'WORK' ? workSessionCount : workSessionCount); 
    switch (newMode) {
      case 'WORK': setTimeLeft(WORK_DURATION_SECONDS); break;
      case 'SHORT_BREAK': setTimeLeft(SHORT_BREAK_DURATION_SECONDS); break;
      case 'LONG_BREAK': setTimeLeft(LONG_BREAK_DURATION_SECONDS); break;
    }
    if (autoStart) setIsTimerActive(true);
  };
  const formatTime = (seconds: number) => `${String(Math.floor(seconds / 60)).padStart(2, '0')}:${String(seconds % 60).padStart(2, '0')}`;
  const getTimerProgress = () => {
    const total = currentTimerMode === 'WORK' ? WORK_DURATION_SECONDS : (currentTimerMode === 'SHORT_BREAK' ? SHORT_BREAK_DURATION_SECONDS : LONG_BREAK_DURATION_SECONDS);
    return total > 0 ? ((total - timeLeft) / total) * 100 : 0;
  };

  const handleCreateAndLinkTask = () => {
    if (!newTaskForLinkText.trim()) {
        addNotification('error', 'New task name cannot be empty.', {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TIMER });
        return;
    }
    const newTaskId = crypto.randomUUID();
    const newTaskItem: TaskItem = {
        id: newTaskId, text: newTaskForLinkText.trim(), completed: false,
        createdAt: new Date(), reminderMinutesBefore: 0,
        scheduledDate: scheduleNewLinkedTask ? new Date().toISOString().split('T')[0] : undefined,
    };
    const savedTasksString = localStorage.getItem('dashboard-tasks');
    let currentTasks: TaskItem[] = [];
    if (savedTasksString) { try { currentTasks = JSON.parse(savedTasksString).map((t:any) => ({...t, createdAt: new Date(t.createdAt)})); } catch {} }
    const updatedTasks = [newTaskItem, ...currentTasks];
    localStorage.setItem('dashboard-tasks', JSON.stringify(updatedTasks.map(t => ({...t, createdAt: t.createdAt.toISOString()}))));

    setLinkedTaskId(newTaskId);
    setTasksForLinking(prev => [newTaskItem, ...prev].sort((a,b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime())); 
    addNotification('success', `Task "${newTaskItem.text.substring(0,20)}..." created & linked. ${scheduleNewLinkedTask ? 'Scheduled for today.' : ''}`, {
        sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TASKS, sourceItemId: newTaskId
    });
    setNewTaskForLinkText(''); setShowNewTaskInputForLink(false); setScheduleNewLinkedTask(true);
  };

  // --- Calendar State & Logic ---
  const [calendarEvents, setCalendarEvents] = useState<CalendarEvent[]>(() => {
    const saved = localStorage.getItem('dashboard-calendarEvents');
    try { 
        const parsed = saved ? JSON.parse(saved) : [];
        return parsed.map((event: any) => ({ ...event, reminderMinutesBefore: typeof event.reminderMinutesBefore === 'number' ? event.reminderMinutesBefore : 0, }));
    } catch { return []; }
  });
  const [currentDisplayMonthYear, setCurrentDisplayMonthYear] = useState({ month: new Date().getMonth(), year: new Date().getFullYear() }); 
  const [selectedCalDateString, setSelectedCalDateString] = useState<string | null>(internalNavigateToDateString || null);
  const [showEventForm, setShowEventForm] = useState(false);
  const [eventFormDetails, setEventFormDetails] = useState<Partial<CalendarEvent>>({ 
      allDay: true, date: new Date().toISOString().split('T')[0], startTime: '09:00', endTime: '10:00',
      reminderMinutesBefore: 0, title: '', description: ''
  });

  useEffect(() => { 
    localStorage.setItem('dashboard-calendarEvents', JSON.stringify(calendarEvents.map(event => ({...event, reminderMinutesBefore: event.reminderMinutesBefore || 0, }))));
  }, [calendarEvents]);
  
  useEffect(() => {
    if (activeSubView === ProductivitySubViewType.CALENDAR && internalNavigateToItemId) {
      const eventToFocus = calendarEvents.find(e => e.id === internalNavigateToItemId);
      if (eventToFocus) {
        setSelectedCalDateString(eventToFocus.date);
        setCurrentDisplayMonthYear({ month: new Date(eventToFocus.date + "T00:00:00").getMonth(), year: new Date(eventToFocus.date + "T00:00:00").getFullYear() });
        setEventFormDetails({...eventToFocus, reminderMinutesBefore: eventToFocus.reminderMinutesBefore || 0});
        setShowEventForm(true);
         setTimeout(() => {
             const eventElement = document.getElementById(`event-${internalNavigateToItemId}`);
             if (eventElement) {
                 eventElement.scrollIntoView({ behavior: 'smooth', block: 'center' });
                 eventElement.classList.add('ring-2', 'ring-sky-400', 'transition-all', 'duration-1000');
                 setTimeout(() => eventElement.classList.remove('ring-2', 'ring-sky-400', 'transition-all', 'duration-1000'), 2000);
             }
         }, 100);
      }
    } else if (activeSubView === ProductivitySubViewType.CALENDAR && internalNavigateToDateString) {
        setSelectedCalDateString(internalNavigateToDateString);
        setCurrentDisplayMonthYear({ month: new Date(internalNavigateToDateString + "T00:00:00").getMonth(), year: new Date(internalNavigateToDateString + "T00:00:00").getFullYear() });
        setShowEventForm(false); // Don't open form if just navigating to date
    }
  }, [activeSubView, internalNavigateToItemId, internalNavigateToDateString, calendarEvents]);


  const handleMonthChange = (offset: number) => {
    setCurrentDisplayMonthYear(prev => {
      let newMonth = prev.month + offset; let newYear = prev.year;
      if (newMonth < 0) { newMonth = 11; newYear--; }
      else if (newMonth > 11) { newMonth = 0; newYear++; }
      return { month: newMonth, year: newYear };
    });
  };

  const handleCalDayClick = (day: number) => {
    const dateStr = `${currentDisplayMonthYear.year}-${String(currentDisplayMonthYear.month + 1).padStart(2, '0')}-${String(day).padStart(2, '0')}`;
    setSelectedCalDateString(dateStr);
    setInternalNavigateToDateString(dateStr); // Update internal state for consistency
    setInternalNavigateToItemId(undefined); // Clear item ID when clicking a day
    setEventFormDetails({ allDay: true, date: dateStr, startTime: '09:00', endTime: '10:00', reminderMinutesBefore: 0, title: '', description: '' }); 
    setShowEventForm(true); 
  };
  
  const handleEventFormChange = (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement>) => {
    const { name, value, type } = e.target;
    const checked = type === 'checkbox' ? (e.target as HTMLInputElement).checked : undefined;
    const val = name === 'reminderMinutesBefore' ? Number(value) : value;
    setEventFormDetails(prev => ({ ...prev, [name]: type === 'checkbox' ? checked : val }));
  };

  const handleSaveEvent = (e: React.FormEvent) => {
    e.preventDefault();
    if (!eventFormDetails.title || !eventFormDetails.date) {
      addNotification('error', 'Event title and date are required.', {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.CALENDAR });
      return;
    }
    const fullEvent: CalendarEvent = {
      id: eventFormDetails.id || crypto.randomUUID(), title: eventFormDetails.title, date: eventFormDetails.date, 
      startTime: eventFormDetails.allDay ? '' : (eventFormDetails.startTime || '00:00'),
      endTime: eventFormDetails.allDay ? '' : (eventFormDetails.endTime || '00:00'),
      description: eventFormDetails.description || '', allDay: eventFormDetails.allDay || false,
      createdAt: eventFormDetails.createdAt || new Date().toISOString(),
      reminderMinutesBefore: eventFormDetails.reminderMinutesBefore || 0,
    };
    const sourceDetails = { sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.CALENDAR, sourceItemId: fullEvent.id };
    if (eventFormDetails.id) { 
      setCalendarEvents(prev => prev.map(ev => ev.id === fullEvent.id ? fullEvent : ev));
      addNotification('success', 'Event updated!', sourceDetails);
    } else { 
      setCalendarEvents(prev => [fullEvent, ...prev]);
      addNotification('success', 'Event added!', sourceDetails);
    }
    setShowEventForm(false);
    setEventFormDetails({ allDay: true, date: selectedCalDateString || new Date().toISOString().split('T')[0], startTime: '09:00', endTime: '10:00', reminderMinutesBefore: 0, title:'', description:'' });
  };
  
  const handleEditEventClick = (event: CalendarEvent) => {
      setSelectedCalDateString(event.date); 
      setInternalNavigateToDateString(event.date);
      setInternalNavigateToItemId(event.id);
      setCurrentDisplayMonthYear({month: new Date(event.date + "T00:00:00").getMonth(), year: new Date(event.date + "T00:00:00").getFullYear()});
      setEventFormDetails({...event, reminderMinutesBefore: event.reminderMinutesBefore || 0});
      setShowEventForm(true);
  };

  const handleDeleteEvent = (eventId: string) => {
    const eventToDelete = calendarEvents.find(ev => ev.id === eventId);
    setCalendarEvents(prev => prev.filter(ev => ev.id !== eventId));
    if (eventToDelete) {
        addNotification('info', `Event "${eventToDelete.title.substring(0,20)}..." deleted.`, {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.CALENDAR});
    }
    if (eventFormDetails.id === eventId) setShowEventForm(false); 
  };
  
  const handleCreateTaskFromEvent = useCallback((eventDetails: Partial<CalendarEvent>) => {
    if (!eventDetails.title || !eventDetails.date) return;
    setTaskDataToPrefill({
        text: `Follow up: ${eventDetails.title}`,
        scheduledDate: eventDetails.date,
    });
    setActiveSubView(ProductivitySubViewType.TASKS);
    setInternalNavigateToItemId(undefined); // Clear item ID for tasks view, focus on new task
    setShowEventForm(false); // Close event form
  }, []);

  const handleGoToCalendarDateFromTask = useCallback((date: string) => {
    setActiveSubView(ProductivitySubViewType.CALENDAR);
    setInternalNavigateToDateString(date);
    setInternalNavigateToItemId(undefined); // Clear item ID when just focusing a date
    setSelectedCalDateString(date); // Ensure the date is selected in calendar
    setCurrentDisplayMonthYear({ month: new Date(date + "T00:00:00").getMonth(), year: new Date(date + "T00:00:00").getFullYear() });
    setShowEventForm(false); // Don't auto-open event form
  }, []);


  const renderCalendarGrid = () => {
    const daysInMonth = getDaysInMonth(currentDisplayMonthYear.month, currentDisplayMonthYear.year);
    const firstDayIndex = getFirstDayOfMonth(currentDisplayMonthYear.month, currentDisplayMonthYear.year); 
    const today = new Date();
    const todayStr = `${today.getFullYear()}-${String(today.getMonth() + 1).padStart(2, '0')}-${String(today.getDate()).padStart(2, '0')}`;

    const blanks = Array(firstDayIndex).fill(null);
    const daysArray = Array.from({ length: daysInMonth }, (_, i) => i + 1);
    
    const itemsOnDate = (dateStr: string) => {
        const eventsCount = calendarEvents.filter(ev => ev.date === dateStr).length;
        const savedTasks = localStorage.getItem('dashboard-tasks');
        let tasksCount = 0;
        if (savedTasks) {
            try {
                const allTasks: TaskItem[] = JSON.parse(savedTasks);
                tasksCount = allTasks.filter(task => task.scheduledDate === dateStr).length;
            } catch {}
        }
        return { eventsCount, tasksCount };
    };

    return (
      <div className="grid grid-cols-7 gap-px text-center text-xs sm:text-sm bg-slate-700 border border-slate-700">
        {['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'].map(d => <div key={d} className="font-medium text-slate-300 py-1.5 bg-slate-750">{d.substring(0,3)}</div>)}
        {blanks.map((_, i) => <div key={`blank-${i}`} className="bg-slate-800 h-16 sm:h-20 md:h-24"></div>)}
        {daysArray.map(day => {
          const dateStr = `${currentDisplayMonthYear.year}-${String(currentDisplayMonthYear.month + 1).padStart(2, '0')}-${String(day).padStart(2, '0')}`;
          const { eventsCount, tasksCount } = itemsOnDate(dateStr);
          return (
            <div key={day} onClick={() => handleCalDayClick(day)}
              className={`p-1 h-16 sm:h-20 md:h-24 cursor-pointer hover:bg-slate-600 transition-colors flex flex-col items-start justify-start relative
                ${dateStr === todayStr ? 'bg-sky-800/50' : 'bg-slate-800'}
                ${dateStr === selectedCalDateString ? 'ring-2 ring-sky-500 inset-0 z-10' : ''}`}>
              <span className={`p-0.5 rounded text-xs ${dateStr === todayStr ? 'text-sky-300 font-bold' : 'text-slate-300'}`}>{day}</span>
              <div className="flex space-x-1 mt-auto self-end">
                {eventsCount > 0 && <div className="w-1.5 h-1.5 sm:w-2 sm:h-2 bg-emerald-500 rounded-full" title={`${eventsCount} event(s)`}></div>}
                {tasksCount > 0 && <div className="w-1.5 h-1.5 sm:w-2 sm:h-2 bg-purple-500 rounded-full" title={`${tasksCount} task(s) scheduled`}></div>}
              </div>
            </div>
          );
        })}
      </div>
    );
  };

  // --- Graphs & Data State & Logic ---
  const [tasksForGraph, setTasksForGraph] = useState<TaskItem[]>([]);
  useEffect(() => {
    if (activeSubView === ProductivitySubViewType.GRAPHS_DATA) {
      const savedTasks = localStorage.getItem('dashboard-tasks');
      try { setTasksForGraph(savedTasks ? JSON.parse(savedTasks).map((t:any) => ({...t, createdAt: new Date(t.createdAt)})) : []); } catch { setTasksForGraph([]); }
    }
  }, [activeSubView]); 

  const taskStats = (() => {
    const total = tasksForGraph.length;
    const completed = tasksForGraph.filter(t => t.completed).length;
    return { total, completed, pending: total - completed, completionPercent: total > 0 ? (completed / total) * 100 : 0 };
  })();

  const pomodoroGraphStats = (() => {
    const todayISO = new Date().toISOString().split('T')[0];
    const todaySessions = pomodoroSessions.filter(s => s.date.startsWith(todayISO));
    const oneWeekAgo = new Date(); oneWeekAgo.setDate(oneWeekAgo.getDate() - 6); oneWeekAgo.setHours(0,0,0,0);
    const sessionsThisWeek = pomodoroSessions.filter(s => new Date(s.date) >= oneWeekAgo && new Date(s.date) <= new Date()); 
    const dailyWorkSessionsLast7Days: { date: string, count: number }[] = [];
    for (let i = 0; i < 7; i++) { 
        const d = new Date(); d.setDate(d.getDate() - (6-i) ); 
        const dateStr = d.toISOString().split('T')[0];
        const count = pomodoroSessions.filter(s => s.date.startsWith(dateStr) && s.type === 'WORK').length;
        dailyWorkSessionsLast7Days.push({ date: `${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`, count });
    }
    return { 
      sessionsTodayCount: todaySessions.length, 
      workTimeTodaySeconds: todaySessions.filter(s => s.type === 'WORK').reduce((sum, s) => sum + s.duration, 0),
      sessionsThisWeekCount: sessionsThisWeek.length,
      workTimeThisWeekSeconds: sessionsThisWeek.filter(s => s.type === 'WORK').reduce((sum, s) => sum + s.duration, 0),
      dailyWorkSessionsLast7Days 
    };
  })();

  // --- Search State & Logic ---
  const [searchTerm, setSearchTerm] = useState('');
  const [taskSearchResults, setTaskSearchResults] = useState<TaskItem[]>([]);
  const [calendarSearchResults, setCalendarSearchResults] = useState<CalendarEvent[]>([]);
  const searchDebounceTimeout = useRef<ReturnType<typeof setTimeout> | null>(null);

  useEffect(() => {
    if (searchDebounceTimeout.current) clearTimeout(searchDebounceTimeout.current);
    searchDebounceTimeout.current = setTimeout(() => {
        if (!searchTerm.trim()) { setTaskSearchResults([]); setCalendarSearchResults([]); return; }
        const lowerSearchTerm = searchTerm.toLowerCase();
        const savedTasks = localStorage.getItem('dashboard-tasks');
        if (savedTasks) {
            try {
                const allTasks: TaskItem[] = JSON.parse(savedTasks).map((t:any) => ({...t, createdAt: new Date(t.createdAt)}));
                setTaskSearchResults(allTasks.filter(task => task.text.toLowerCase().includes(lowerSearchTerm)));
            } catch { setTaskSearchResults([]); }
        }
        setCalendarSearchResults(calendarEvents.filter(event =>
            event.title.toLowerCase().includes(lowerSearchTerm) ||
            (event.description && event.description.toLowerCase().includes(lowerSearchTerm))
        ));
    }, 300); 
    return () => { if (searchDebounceTimeout.current) clearTimeout(searchDebounceTimeout.current); };
  }, [searchTerm, calendarEvents]); 

  // --- Timeline State & Logic ---
  const [timelineItems, setTimelineItems] = useState<TimelineDisplayableItem[]>([]);
  
  useEffect(() => {
    if (activeSubView === ProductivitySubViewType.TIMELINE || activeSubView === ProductivitySubViewType.CALENDAR) { 
        const today = new Date(); today.setHours(0,0,0,0);
        const next7DaysBoundary = new Date(today); next7DaysBoundary.setDate(today.getDate() + 7); next7DaysBoundary.setHours(23,59,59,999);

        const relevantEvents: CalendarEvent[] = calendarEvents.filter(event => {
            const eventDate = new Date(event.date + "T00:00:00"); 
            return eventDate >= today && eventDate <= next7DaysBoundary;
        });
        
        let scheduledTasksForTimeline: TimelineTaskItem[] = [];
        const savedTasks = localStorage.getItem('dashboard-tasks');
        if (savedTasks) {
            try {
                const allTasks: TaskItem[] = JSON.parse(savedTasks).map((t:any) => ({...t, createdAt: new Date(t.createdAt)}));
                scheduledTasksForTimeline = allTasks
                    .filter(task => task.scheduledDate && new Date(task.scheduledDate + "T00:00:00") >= today && new Date(task.scheduledDate + "T00:00:00") <= next7DaysBoundary && !task.completed) 
                    .map(task => ({ ...task, isTask: true as const, date: task.scheduledDate! })); 
            } catch { /* ignore error */ }
        }
        
        const combinedItemsForTimeline: TimelineDisplayableItem[] = [...relevantEvents, ...scheduledTasksForTimeline];
        const groupedByDate: Record<string, TimelineDisplayableItem[]> = {};

        combinedItemsForTimeline.forEach((item: TimelineDisplayableItem) => {
            const itemDate = item.date; 
            if (!groupedByDate[itemDate]) groupedByDate[itemDate] = [];
            groupedByDate[itemDate].push(item);
        });

        const finalSortedTimeline: TimelineDisplayableItem[] = [];
        Object.keys(groupedByDate).sort((a,b) => new Date(a).getTime() - new Date(b).getTime()).forEach(dateKey => {
            groupedByDate[dateKey].sort((a,b) => {
                const aIsTask = 'isTask' in a && a.isTask === true;
                const bIsTask = 'isTask' in b && b.isTask === true;
                
                if (!aIsTask && !bIsTask) { 
                    const eventA = a as CalendarEvent; const eventB = b as CalendarEvent; 
                    if (eventA.allDay && !eventB.allDay) return -1; if (!eventA.allDay && eventB.allDay) return 1;
                    if (eventA.allDay && eventB.allDay) return eventA.title.localeCompare(eventB.title); 
                    return (eventA.startTime || "00:00").localeCompare(eventB.startTime || "00:00");
                }
                if (aIsTask && bIsTask) { return new Date(a.createdAt).getTime() - new Date(b.createdAt).getTime(); } 
                
                if (!aIsTask && (a as CalendarEvent).allDay) return -1; 
                if (!bIsTask && (b as CalendarEvent).allDay) return 1;  
                
                if (!aIsTask && !bIsTask) return (a as CalendarEvent).startTime.localeCompare((b as CalendarEvent).startTime); 
                if (!aIsTask && bIsTask) return -1; 
                if (aIsTask && !bIsTask) return 1;  

                return 0;
            });
            finalSortedTimeline.push(...groupedByDate[dateKey]);
        });
        setTimelineItems(finalSortedTimeline);
    }
  }, [activeSubView, calendarEvents]);

  const subViewNavItems = [
    { type: ProductivitySubViewType.TIMELINE, label: 'Timeline', icon: <ListBulletIcon className="w-4 h-4 sm:w-5 sm:h-5 mr-1 sm:mr-2"/> },
    { type: ProductivitySubViewType.CALENDAR, label: 'Calendar', icon: <CalendarDaysIcon className="w-4 h-4 sm:w-5 sm:h-5 mr-1 sm:mr-2"/> },
    { type: ProductivitySubViewType.TASKS, label: 'Tasks', icon: <ClipboardDocumentCheckIcon className="w-4 h-4 sm:w-5 sm:h-5 mr-1 sm:mr-2"/> },
    { type: ProductivitySubViewType.TIMER, label: 'Timer', icon: <ClockIcon className="w-4 h-4 sm:w-5 sm:h-5 mr-1 sm:mr-2"/> },
    { type: ProductivitySubViewType.GRAPHS_DATA, label: 'Graphs', icon: <ChartBarIcon className="w-4 h-4 sm:w-5 sm:h-5 mr-1 sm:mr-2"/> },
    { type: ProductivitySubViewType.SEARCH, label: 'Search', icon: <MagnifyingGlassIcon className="w-4 h-4 sm:w-5 sm:h-5 mr-1 sm:mr-2"/> },
  ];
  
  const handleTimelineItemClick = (item: TimelineDisplayableItem) => {
    const isTimelineTaskItem = 'isTask' in item && item.isTask === true;
    const sourceItemId = item.id;

    if (isTimelineTaskItem) {
        const task = item as TimelineTaskItem;
        setActiveSubView(ProductivitySubViewType.TASKS);
        setInternalNavigateToItemId(task.id); 
        setTaskDataToPrefill(null); // Clear prefill if navigating this way
    } else {
        const event = item as CalendarEvent;
        setActiveSubView(ProductivitySubViewType.CALENDAR);
        setInternalNavigateToDateString(event.date);
        setInternalNavigateToItemId(event.id);  
        setSelectedCalDateString(event.date); // Also update selectedCalDateString directly
        setCurrentDisplayMonthYear({month: new Date(event.date + "T00:00:00").getMonth(), year: new Date(event.date + "T00:00:00").getFullYear()});
        setEventFormDetails({...event, reminderMinutesBefore: event.reminderMinutesBefore || 0 });
        setShowEventForm(true); 
        setTaskDataToPrefill(null); // Clear prefill
    }
  };


  const RenderTimelineItem: React.FC<{item: TimelineDisplayableItem, onClick: () => void}> = ({ item, onClick }) => {
    const isTaskItem = 'isTask' in item && item.isTask === true;
    const itemDate = new Date(item.date + "T00:00:00");
    let timeDisplay = "";
    let descriptionSnippet = "";

    if (isTaskItem) {
        const task = item as TimelineTaskItem;
        descriptionSnippet = task.text.substring(0, 100) + (task.text.length > 100 ? "..." : "");
        if (task.dueDate) {
            timeDisplay = `Due: ${task.dueDate}`;
            if (task.dueTime) timeDisplay += ` ${task.dueTime}`;
        } else if (task.scheduledDate) {
            timeDisplay = `Scheduled: ${task.scheduledDate}`;
        }
    } else {
        const event = item as CalendarEvent;
        descriptionSnippet = (event.description || "").substring(0, 100) + ((event.description || "").length > 100 ? "..." : "");
        if (event.allDay) {
            timeDisplay = "All Day";
        } else {
            const formatEventTime = (timeStr: string) => {
                if (!timeStr) return "";
                const [hours, minutes] = timeStr.split(':');
                const date = new Date();
                date.setHours(parseInt(hours,10), parseInt(minutes,10));
                return date.toLocaleTimeString([], { hour: 'numeric', minute: '2-digit', hour12: true });
            };
            timeDisplay = `${formatEventTime(event.startTime)} - ${formatEventTime(event.endTime)}`;
        }
    }

    const itemBaseColor = isTaskItem ? 'var(--theme-info)' : 'var(--theme-success)';
    const ItemIcon = isTaskItem ? ClipboardDocumentCheckIcon : CalendarDaysIcon;

    return (
        <div 
            onClick={onClick}
            className={`flex items-start p-3 rounded-lg shadow-md hover:shadow-lg transition-all duration-150 ease-in-out cursor-pointer bg-[var(--theme-card-bg)]/70 hover:bg-[var(--theme-card-bg)] border-l-4`}
            style={{ borderColor: itemBaseColor }}
            role="button"
            tabIndex={0}
            onKeyDown={(e) => { if (e.key === 'Enter' || e.key === ' ') onClick(); }}
            aria-label={`View details for ${isTaskItem ? 'task' : 'event'}: ${isTaskItem ? (item as TimelineTaskItem).text.substring(0,30) : (item as CalendarEvent).title.substring(0,30)}`}
        >
            <ItemIcon className="w-5 h-5 sm:w-6 sm:h-6 mr-3 mt-1 flex-shrink-0" style={{ color: itemBaseColor }} />
            <div className="flex-grow min-w-0">
                <p className="font-semibold text-sm sm:text-base text-[var(--theme-text-primary)] truncate" title={isTaskItem ? (item as TimelineTaskItem).text : (item as CalendarEvent).title}>
                    {isTaskItem ? (item as TimelineTaskItem).text : (item as CalendarEvent).title}
                </p>
                {timeDisplay && <p className="text-xs sm:text-sm text-[var(--theme-text-secondary)] mt-0.5">{timeDisplay}</p>}
                {descriptionSnippet && <p className="text-xs text-slate-500 mt-1 truncate">{descriptionSnippet}</p>}
            </div>
        </div>
    );
  };


  // Main render logic for sub-views
  const renderSubView = () => {
    switch (activeSubView) {
      case ProductivitySubViewType.TIMER:
        const linkedTaskForDisplay = linkedTaskId ? tasksForLinking.find(t => t.id === linkedTaskId) : null;
        return (
          <div className="bg-slate-800/80 p-5 rounded-xl shadow-2xl max-w-md mx-auto text-center border border-slate-700/50">
            <div className="mb-4">
                {['WORK', 'SHORT_BREAK', 'LONG_BREAK'].map(mode => (
                    <button key={mode} onClick={() => selectTimerModeUI(mode as TimerMode)}
                            className={`px-3 py-1.5 mx-1 text-xs sm:text-sm font-medium rounded-lg transition-all duration-200
                                        ${currentTimerMode === mode ? 'bg-sky-500 text-white shadow-lg ring-2 ring-sky-300 ring-offset-2 ring-offset-slate-800' 
                                                                    : 'bg-slate-700 hover:bg-slate-600 text-slate-300'}`}>
                        {mode.replace('_', ' ')}
                    </button>
                ))}
            </div>
            <div className="relative w-40 h-40 sm:w-48 sm:h-48 mx-auto mb-4">
                <svg className="w-full h-full" viewBox="0 0 36 36">
                    <path className="text-slate-700" strokeWidth="3" fill="none" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" />
                    <path className="text-sky-400" strokeWidth="3" strokeDasharray={`${getTimerProgress()}, 100`} strokeLinecap="round" fill="none" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" />
                </svg>
                <div className="absolute inset-0 flex items-center justify-center">
                    <span className="text-3xl sm:text-4xl font-mono font-bold text-sky-300">{formatTime(timeLeft)}</span>
                </div>
            </div>
            <div className="flex justify-center space-x-3 mb-5">
                <button onClick={toggleTimer} className={`px-5 py-2 rounded-lg font-semibold shadow-md transition-all duration-200 text-sm sm:text-base min-w-[100px] ${isTimerActive ? 'bg-amber-500 hover:bg-amber-600 text-white' : 'bg-green-500 hover:bg-green-600 text-white'}`}>
                    {isTimerActive ? 'Pause' : 'Start'}
                </button>
                <button onClick={resetTimer} className="px-5 py-2 bg-red-600 hover:bg-red-700 text-white font-semibold rounded-lg shadow-md transition-colors text-sm sm:text-base min-w-[100px]">Reset</button>
            </div>
            <div className="text-xs text-slate-400 mb-2">Work Sessions Completed: {workSessionCount} / {SESSIONS_BEFORE_LONG_BREAK}</div>
            
            <div className="mt-4 pt-4 border-t border-slate-700 text-left text-xs">
                <label htmlFor="linkedTask" className="block text-sm font-medium text-slate-300 mb-1">Link to Task:</label>
                {linkedTaskForDisplay ? (
                    <div className="flex items-center justify-between p-2 bg-slate-700 rounded">
                        <span className="text-slate-200 truncate" title={linkedTaskForDisplay.text}>{linkedTaskForDisplay.text}</span>
                        <button onClick={() => setLinkedTaskId(null)} className="text-xs text-red-400 hover:text-red-300 ml-2">Unlink</button>
                    </div>
                ) : (
                  <>
                    <select id="linkedTask" value={linkedTaskId || ''} onChange={e => setLinkedTaskId(e.target.value || null)}
                            className="w-full p-1.5 bg-slate-600 border-slate-500 rounded text-slate-200 mb-2 appearance-none focus:ring-1 focus:ring-sky-500">
                        <option value="">No task linked</option>
                        {tasksForLinking.map(task => <option key={task.id} value={task.id}>{task.text.substring(0,40)}{task.text.length > 40 ? '...' : ''}</option>)}
                    </select>
                    <button onClick={() => setShowNewTaskInputForLink(!showNewTaskInputForLink)} className="text-sky-400 hover:text-sky-300 text-xs">
                        {showNewTaskInputForLink ? 'Cancel New Task' : '+ Create New Task to Link'}
                    </button>
                    {showNewTaskInputForLink && (
                        <div className="mt-2 space-y-1.5">
                            <input type="text" value={newTaskForLinkText} onChange={e => setNewTaskForLinkText(e.target.value)} placeholder="New task name..." className="w-full p-1.5 bg-slate-600 border-slate-500 rounded"/>
                            <label className="flex items-center text-slate-300"><input type="checkbox" checked={scheduleNewLinkedTask} onChange={e => setScheduleNewLinkedTask(e.target.checked)} className="mr-1.5 h-3 w-3 text-sky-500 bg-slate-500 border-slate-400 rounded focus:ring-sky-400"/>Schedule for today</label>
                            <button onClick={handleCreateAndLinkTask} className="w-full px-3 py-1 bg-emerald-600 hover:bg-emerald-700 text-white text-xs rounded">Create & Link</button>
                        </div>
                    )}
                  </>
                )}
            </div>
            <audio ref={timerAudioRef} preload="auto" />
          </div>
        );
      case ProductivitySubViewType.TASKS:
        return <TasksFeature 
                    addNotification={addNotification} 
                    navigateToItemId={internalNavigateToItemId} 
                    focusNewTaskInputSignal={commandToExecuteFromPalette?.actionType === 'TASKS_FOCUS_NEW_INPUT'}
                    initialPrefillData={taskDataToPrefill}
                    onGoToCalendarDate={handleGoToCalendarDateFromTask}
                />;
      
      case ProductivitySubViewType.CALENDAR:
        const eventsForSelectedDate = selectedCalDateString ? calendarEvents.filter(e => e.date === selectedCalDateString).sort((a, b) => (a.startTime || "00:00").localeCompare(b.startTime || "00:00")) : [];
        const tasksForSelectedDate = selectedCalDateString ? (localStorage.getItem('dashboard-tasks') ? JSON.parse(localStorage.getItem('dashboard-tasks')!).filter((t:TaskItem) => t.scheduledDate === selectedCalDateString && !t.completed) : []) : [];
        return (
            <div className="grid grid-cols-1 lg:grid-cols-3 gap-4 h-full">
                <div className="lg:col-span-2 bg-slate-800/80 p-4 rounded-xl shadow-lg border border-slate-700/50 overflow-hidden flex flex-col">
                    <div className="flex justify-between items-center mb-3">
                        <button onClick={() => handleMonthChange(-1)} className="p-1.5 hover:bg-slate-700 rounded-full text-slate-300"><ChevronLeftIcon className="w-5 h-5"/></button>
                        <h2 className="text-lg font-semibold text-sky-300">{new Date(currentDisplayMonthYear.year, currentDisplayMonthYear.month).toLocaleString('default', { month: 'long', year: 'numeric' })}</h2>
                        <button onClick={() => handleMonthChange(1)} className="p-1.5 hover:bg-slate-700 rounded-full text-slate-300"><ChevronRightIcon className="w-5 h-5"/></button>
                    </div>
                    <div className="flex-grow overflow-y-auto pr-1">{renderCalendarGrid()}</div>
                </div>
                <div className="bg-slate-800/80 p-4 rounded-xl shadow-lg border border-slate-700/50 overflow-y-auto flex flex-col">
                    {selectedCalDateString ? (
                        <>
                            <h3 className="text-md font-semibold text-sky-300 mb-2">Details for {new Date(selectedCalDateString + "T00:00:00").toLocaleDateString(undefined, { weekday: 'long', month: 'long', day: 'numeric' })}</h3>
                            {showEventForm ? (
                                <form onSubmit={handleSaveEvent} className="space-y-2.5 text-xs">
                                    <h4 className="text-sm text-emerald-400 mt-1 mb-1">{eventFormDetails.id ? 'Edit Event' : 'New Event'}</h4>
                                    <div><label className="block text-slate-400 mb-0.5">Title*</label><input type="text" name="title" value={eventFormDetails.title || ''} onChange={handleEventFormChange} required className="w-full p-1.5 bg-slate-700 border-slate-600 rounded"/></div>
                                    <div className="flex items-center"><input type="checkbox" name="allDay" checked={eventFormDetails.allDay || false} onChange={handleEventFormChange} className="h-3.5 w-3.5 mr-1.5 text-sky-500 bg-slate-600 border-slate-500 rounded focus:ring-sky-400"/><label className="text-slate-300">All day</label></div>
                                    {!eventFormDetails.allDay && (<div className="grid grid-cols-2 gap-2"><div><label className="block text-slate-400 mb-0.5">Start Time</label><input type="time" name="startTime" value={eventFormDetails.startTime || ''} onChange={handleEventFormChange} className="w-full p-1.5 bg-slate-700 border-slate-600 rounded"/></div><div><label className="block text-slate-400 mb-0.5">End Time</label><input type="time" name="endTime" value={eventFormDetails.endTime || ''} onChange={handleEventFormChange} className="w-full p-1.5 bg-slate-700 border-slate-600 rounded"/></div></div>)}
                                    <div><label className="block text-slate-400 mb-0.5">Description</label><textarea name="description" value={eventFormDetails.description || ''} onChange={handleEventFormChange} rows={2} className="w-full p-1.5 bg-slate-700 border-slate-600 rounded"/></div>
                                    <div><label className="block text-slate-400 mb-0.5">Reminder</label><select name="reminderMinutesBefore" value={eventFormDetails.reminderMinutesBefore || 0} onChange={handleEventFormChange} className="w-full p-1.5 bg-slate-700 border-slate-600 rounded appearance-none focus:ring-1 focus:ring-sky-400">{reminderOptions.map(o=><option key={o.value} value={o.value}>{o.label}</option>)}</select></div>
                                    <div className="flex justify-end space-x-2 pt-1">
                                        <button type="button" onClick={() => { setShowEventForm(false); setEventFormDetails({ allDay: true, date: selectedCalDateString!, startTime: '09:00', endTime: '10:00', reminderMinutesBefore: 0, title: '', description: '' }); }} className="px-3 py-1 bg-slate-600 hover:bg-slate-500 text-white rounded">Cancel</button>
                                        <button type="submit" className="px-3 py-1 bg-emerald-600 hover:bg-emerald-700 text-white rounded">{eventFormDetails.id ? 'Save Changes' : 'Add Event'}</button>
                                        {eventFormDetails.id && <button type="button" onClick={() => handleDeleteEvent(eventFormDetails.id!)} className="px-3 py-1 bg-red-600 hover:bg-red-700 text-white rounded">Delete</button>}
                                    </div>
                                     <button type="button" onClick={() => handleCreateTaskFromEvent(eventFormDetails)} className="w-full mt-2 px-3 py-1.5 bg-purple-600 hover:bg-purple-700 text-white text-xs rounded flex items-center justify-center"><PlusIcon className="w-3.5 h-3.5 mr-1"/> Create Related Task</button>
                                </form>
                            ) : (
                                <div>
                                    {eventsForSelectedDate.length === 0 && tasksForSelectedDate.length === 0 ? <p className="text-slate-400 italic text-sm text-center py-4">No events or tasks for this day.</p> : (
                                        <div className="space-y-2">
                                          {eventsForSelectedDate.map(event => (
                                                <div key={event.id} id={`event-${event.id}`} className="p-2 bg-slate-700/70 rounded-md text-xs cursor-pointer hover:bg-slate-700" onClick={() => handleEditEventClick(event)}>
                                                    <p className="font-semibold text-emerald-400">{event.title}</p>
                                                    {!event.allDay && <p className="text-slate-300">{event.startTime} - {event.endTime}</p>}
                                                    {event.description && <p className="text-slate-400 truncate">{event.description}</p>}
                                                </div>
                                            ))}
                                          {tasksForSelectedDate.map((task: TaskItem) => (
                                                <div key={task.id} id={`task-${task.id}`} className="p-2 bg-purple-700/40 rounded-md text-xs cursor-pointer hover:bg-purple-700/60" onClick={() => {setActiveSubView(ProductivitySubViewType.TASKS); setInternalNavigateToItemId(task.id);}}>
                                                    <p className="font-semibold text-purple-300 flex items-center"><ClipboardDocumentCheckIcon className="w-3 h-3 mr-1"/> {task.text}</p>
                                                    {task.dueDate && <p className="text-purple-400/80 text-xs">Due: {task.dueDate} {task.dueTime || ''}</p>}
                                                </div>
                                          ))}
                                        </div>
                                    )}
                                    <div className="mt-3 pt-3 border-t border-slate-700 flex flex-col space-y-2">
                                        <button onClick={() => { setEventFormDetails({ allDay: true, date: selectedCalDateString!, startTime: '09:00', endTime: '10:00', reminderMinutesBefore: 0, title:'', description:'' }); setShowEventForm(true);}} className="w-full px-3 py-1.5 bg-emerald-600 hover:bg-emerald-700 text-white text-xs rounded flex items-center justify-center"><PlusIcon className="w-3.5 h-3.5 mr-1"/> Add Event</button>
                                        <button onClick={() => handleCreateTaskFromEvent({date: selectedCalDateString, title: `Follow up for ${new Date(selectedCalDateString + "T00:00:00").toLocaleDateString()}`})} className="w-full px-3 py-1.5 bg-purple-600 hover:bg-purple-700 text-white text-xs rounded flex items-center justify-center"><PlusIcon className="w-3.5 h-3.5 mr-1"/> Schedule Task for this Day</button>
                                    </div>
                                </div>
                            )}
                        </>
                    ) : (
                        <p className="text-slate-400 italic text-sm text-center py-10">Select a day to view or add events & tasks.</p>
                    )}
                </div>
            </div>
        );
      case ProductivitySubViewType.GRAPHS_DATA:
        return (
            <div className="space-y-6">
                <div className="bg-slate-800/80 p-4 rounded-xl shadow-lg border border-slate-700/50">
                    <h3 className="text-lg font-semibold text-sky-300 mb-3">Task Completion</h3>
                    <div className="flex items-center space-x-4">
                        <div className="relative w-24 h-24">
                            <svg className="w-full h-full" viewBox="0 0 36 36"><path className="text-slate-700" strokeWidth="3.5" fill="none" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" /><path className="text-emerald-400" strokeWidth="3.5" strokeDasharray={`${taskStats.completionPercent}, 100`} strokeLinecap="round" fill="none" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" /></svg>
                            <div className="absolute inset-0 flex items-center justify-center text-xl font-bold text-emerald-300">{taskStats.completionPercent.toFixed(0)}%</div>
                        </div>
                        <div className="text-sm">
                            <p>Total Tasks: <span className="font-semibold text-slate-200">{taskStats.total}</span></p>
                            <p>Completed: <span className="font-semibold text-emerald-300">{taskStats.completed}</span></p>
                            <p>Pending: <span className="font-semibold text-amber-300">{taskStats.pending}</span></p>
                        </div>
                    </div>
                </div>
                <div className="bg-slate-800/80 p-4 rounded-xl shadow-lg border border-slate-700/50">
                    <h3 className="text-lg font-semibold text-sky-300 mb-3">Pomodoro Focus (Work Sessions)</h3>
                    <p className="text-sm mb-1">Today: <span className="font-semibold text-slate-200">{pomodoroGraphStats.sessionsTodayCount}</span> sessions ({formatTime(pomodoroGraphStats.workTimeTodaySeconds)} of work)</p>
                    <p className="text-sm mb-3">This Week: <span className="font-semibold text-slate-200">{pomodoroGraphStats.sessionsThisWeekCount}</span> sessions ({formatTime(pomodoroGraphStats.workTimeThisWeekSeconds)} of work)</p>
                    <h4 className="text-sm font-medium text-slate-300 mb-1">Work Sessions - Last 7 Days:</h4>
                    <div className="flex items-end space-x-1 h-24 bg-slate-700/50 p-2 rounded">
                        {pomodoroGraphStats.dailyWorkSessionsLast7Days.map((day, idx) => {
                            const maxCount = Math.max(...pomodoroGraphStats.dailyWorkSessionsLast7Days.map(d => d.count), 1); // Avoid div by zero
                            const barHeight = Math.max(5, (day.count / maxCount) * 90); // Min height 5%
                            return (
                                <div key={idx} className="flex-1 flex flex-col items-center justify-end group" title={`${day.date}: ${day.count} sessions`}>
                                    <div className="w-full bg-sky-500 hover:bg-sky-400 transition-all rounded-t-sm" style={{ height: `${barHeight}%` }}>
                                        <span className="text-xs text-white opacity-0 group-hover:opacity-100 absolute bottom-full left-1/2 transform -translate-x-1/2 mb-1 bg-slate-900 px-1 rounded">{day.count}</span>
                                    </div>
                                    <span className="text-[10px] text-slate-400 mt-0.5">{day.date.substring(3,5)}</span>
                                </div>
                            );
                        })}
                    </div>
                </div>
            </div>
        );
      case ProductivitySubViewType.SEARCH:
        return (
            <div className="space-y-4">
                <input type="search" value={searchTerm} onChange={e => setSearchTerm(e.target.value)} placeholder="Search tasks and events..." className="w-full p-2.5 bg-slate-700 border border-slate-600 rounded-lg focus:ring-1 focus:ring-sky-500 text-slate-100" />
                {searchTerm.trim() && (taskSearchResults.length > 0 || calendarSearchResults.length > 0) ? (
                    <div className="max-h-[calc(100vh-200px)] overflow-y-auto pr-1 text-sm">
                        {taskSearchResults.length > 0 && <h3 className="text-md font-semibold text-sky-300 my-2">Task Results ({taskSearchResults.length})</h3>}
                        {taskSearchResults.map(task => (
                            <div key={task.id} onClick={() => { setActiveSubView(ProductivitySubViewType.TASKS); setInternalNavigateToItemId(task.id);}} className="p-2 mb-1.5 bg-slate-700/70 hover:bg-slate-700 rounded-md cursor-pointer">
                                <p className={`font-medium ${task.completed ? 'line-through text-slate-400' : 'text-purple-300'}`}>{task.text}</p>
                                <p className="text-xs text-slate-400">Created: {new Date(task.createdAt).toLocaleDateString()}{task.dueDate && ` | Due: ${task.dueDate}`}</p>
                            </div>
                        ))}
                        {calendarSearchResults.length > 0 && <h3 className="text-md font-semibold text-sky-300 my-2 pt-2 border-t border-slate-700">Calendar Event Results ({calendarSearchResults.length})</h3>}
                        {calendarSearchResults.map(event => (
                            <div key={event.id} onClick={() => { setActiveSubView(ProductivitySubViewType.CALENDAR); setInternalNavigateToDateString(event.date); setInternalNavigateToItemId(event.id); setSelectedCalDateString(event.date); setCurrentDisplayMonthYear({month: new Date(event.date + "T00:00:00").getMonth(), year: new Date(event.date + "T00:00:00").getFullYear()}); setEventFormDetails({...event, reminderMinutesBefore: event.reminderMinutesBefore || 0 }); setShowEventForm(true); }} className="p-2 mb-1.5 bg-slate-700/70 hover:bg-slate-700 rounded-md cursor-pointer">
                                <p className="font-medium text-emerald-300">{event.title}</p>
                                <p className="text-xs text-slate-400">Date: {event.date} {event.allDay ? '(All Day)' : `${event.startTime}-${event.endTime}`}</p>
                                {event.description && <p className="text-xs text-slate-500 truncate">{event.description}</p>}
                            </div>
                        ))}
                    </div>
                ) : searchTerm.trim() ? (
                     <p className="text-slate-400 italic text-center py-6">No results found for "{searchTerm}".</p>
                ) : (
                    <p className="text-slate-400 italic text-center py-6">Type above to search your tasks and calendar events.</p>
                )}
            </div>
        );
      case ProductivitySubViewType.TIMELINE:
        const todayStrForTimeline = new Date().toISOString().split('T')[0];
        return (
            <div className="space-y-4 h-full overflow-y-auto pr-1 pb-4">
                <h2 className="text-xl sm:text-2xl font-semibold text-[var(--theme-accent-primary)] sticky top-0 bg-[var(--theme-bg-primary)]/80 backdrop-blur-sm py-3 z-10 px-1">
                    Timeline: Next 7 Days
                </h2>
                {timelineItems.length === 0 ? (
                    <div className="text-center py-10 text-slate-400">
                        <ListBulletIcon className="w-16 h-16 mx-auto text-slate-500 mb-4 opacity-50" />
                        <p className="text-lg">No scheduled items in the next 7 days.</p>
                        <p className="text-sm text-slate-500">Try adding some tasks or calendar events!</p>
                    </div>
                ) :
                timelineItems.reduce<React.ReactNode[]>((acc, item, index) => {
                    const itemDateStr = item.date;
                    const prevItemDateStr = index > 0 ? timelineItems[index - 1].date : null;
                    const showDateHeader = index === 0 || itemDateStr !== prevItemDateStr;
                    
                    if (showDateHeader) {
                        const isToday = itemDateStr === todayStrForTimeline;
                        acc.push(
                            <div key={`header-${itemDateStr}`} 
                                 className={`pt-3 pb-2 my-2 px-2 rounded-md
                                            ${isToday ? 'bg-sky-700/30 border-sky-500' : 'bg-slate-800/50 border-slate-700'} border-b-2 `}>
                                <h3 className={`text-md sm:text-lg font-semibold 
                                                ${isToday ? 'text-sky-300' : 'text-[var(--theme-text-secondary)]'}`}>
                                    {new Date(itemDateStr + "T00:00:00").toLocaleDateString(undefined, { weekday: 'long', month: 'long', day: 'numeric' })}
                                    {isToday && <span className="text-xs align-middle ml-2 px-1.5 py-0.5 bg-sky-500 text-white rounded-full">TODAY</span>}
                                </h3>
                            </div>
                        );
                    }
                    acc.push(
                        <RenderTimelineItem 
                            key={item.id} 
                            item={item} 
                            onClick={() => handleTimelineItemClick(item)} 
                        />
                    );
                    return acc;
                }, [])}
            </div>
        );
      default:
        return <p>Select a sub-view.</p>;
    }
  };

  return (
    <div className="h-full flex flex-col">
      <nav className="mb-4 pb-2 border-b border-[var(--theme-border-primary)] overflow-x-auto whitespace-nowrap">
        <div className="inline-flex space-x-1 sm:space-x-2 p-1 bg-[var(--theme-bg-accent)]/50 rounded-lg">
          {subViewNavItems.map(nav => (
            <button key={nav.type} onClick={() => { setActiveSubView(nav.type); setInternalNavigateToItemId(undefined); setInternalNavigateToDateString(undefined); setTaskDataToPrefill(null);}}
                    className={`px-2.5 py-1.5 sm:px-3 sm:py-2 text-xs sm:text-sm font-medium rounded-md transition-all duration-200 flex items-center
                                ${activeSubView === nav.type ? 'bg-[var(--theme-accent-primary)] text-white shadow-md' 
                                                            : 'text-[var(--theme-text-secondary)] hover:bg-[var(--theme-accent-primary)]/20 hover:text-[var(--theme-text-primary)]'}`}>
              {nav.icon} {nav.label}
            </button>
          ))}
        </div>
      </nav>
      <div className="flex-grow overflow-y-auto min-h-0">{renderSubView()}</div>
    </div>
  );
};

// --- Internal Icons (reduced set for brevity) ---
const ListBulletIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 6.75h7.5M8.25 12h7.5m-7.5 5.25h7.5M3.75 6.75h.007v.008H3.75V6.75zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 12h.007v.008H3.75V12zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0zM3.75 17.25h.007v.008H3.75v-.008zm.375 0a.375.375 0 11-.75 0 .375.375 0 01.75 0z" /></svg>;
const CalendarDaysIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6.75 3v2.25M17.25 3v2.25M3 18.75V7.5a2.25 2.25 0 012.25-2.25h13.5A2.25 2.25 0 0121 7.5v11.25m-18 0A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75m-18 0v-7.5A2.25 2.25 0 015.25 9h13.5A2.25 2.25 0 0121 11.25v7.5" /></svg>;
const ClipboardDocumentCheckIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M10.125 2.25h-4.5c-.621 0-1.125.504-1.125 1.125v17.25c0 .621.504 1.125 1.125 1.125h12.75c.621 0 1.125-.504 1.125-1.125v-9M10.125 2.25c.882 0 1.724.222 2.494.626M10.125 2.25a2.25 2.25 0 00-2.25 2.25M10.125 2.25v3.375c0 .621.504 1.125 1.125 1.125h3.375M9 15l2.25 2.25L15 12" /></svg>;
const ClockIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const ChartBarIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M3 13.125C3 12.504 3.504 12 4.125 12h2.25c.621 0 1.125.504 1.125 1.125v6.75C7.5 20.496 6.996 21 6.375 21h-2.25A1.125 1.125 0 013 19.875v-6.75zM9.75 8.625c0-.621.504-1.125 1.125-1.125h2.25c.621 0 1.125.504 1.125 1.125v11.25c0 .621-.504 1.125-1.125 1.125h-2.25a1.125 1.125 0 01-1.125-1.125V8.625zM16.5 4.125c0-.621.504-1.125 1.125-1.125h2.25C20.496 3 21 3.504 21 4.125v15.75c0 .621-.504 1.125-1.125 1.125h-2.25a1.125 1.125 0 01-1.125-1.125V4.125z" /></svg>;
const MagnifyingGlassIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M21 21l-5.197-5.197m0 0A7.5 7.5 0 105.196 5.196a7.5 7.5 0 0010.607 10.607z" /></svg>;
const ChevronLeftIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M15.75 19.5L8.25 12l7.5-7.5" /></svg>;
const ChevronRightIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M8.25 4.5l7.5 7.5-7.5 7.5" /></svg>;
const PlusIcon = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 4.5v15m7.5-7.5h-15" /></svg>;

export default ProductivitySuiteFeature;
