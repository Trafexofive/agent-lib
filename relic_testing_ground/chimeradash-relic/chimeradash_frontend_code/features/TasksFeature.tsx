
import React, { useState, useEffect, useCallback, useRef } from 'react';
import { TaskItem, NotificationItem, ViewType, ProductivitySubViewType, ProductivityTaskPrefillData } from '../types';

interface TasksFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
      sourceSubView?: ProductivitySubViewType; 
      sourceItemId?: string;
    }
  ) => void;
  navigateToItemId?: string; 
  focusNewTaskInputSignal?: boolean;
  initialPrefillData?: ProductivityTaskPrefillData | null; // For prefilling from Calendar
  onGoToCalendarDate?: (date: string) => void; // For navigating to Calendar
}

const initialDefaultTasks: Omit<TaskItem, 'id' | 'createdAt'>[] = [
  { text: "Command Palette: Implement full Vim-like navigation & advanced filtering", completed: false, reminderMinutesBefore: 0 },
  { text: "AI Tools: Design & Implement Custom AI Tool creation interface", completed: false, reminderMinutesBefore: 0 },
  { text: "AI Tools: Implement execution logic for user-defined Custom AI Tools", completed: false, reminderMinutesBefore: 0 },
  { text: "AI Tools: Research & Implement basic Prompt Chaining functionality", completed: false, reminderMinutesBefore: 0 },
  { text: "Dashboard: Explore further 'Vimification' across other features", completed: false, reminderMinutesBefore: 0 },
  { text: "Refine Notification System: Snooze functionality", completed: false, reminderMinutesBefore: 0 },
  { text: "Refine Notification System: Custom sound options", completed: false, reminderMinutesBefore: 0 },
  { text: "Refine Notification System: Enhance deep linking to scroll/highlight items", completed: false, reminderMinutesBefore: 0 },
  { text: "Command Palette: Add actions for Std::Utils tools (e.g., Base64 encode selected text)", completed: false, reminderMinutesBefore: 0 },
  { text: "CommandPalette: Implement fuzzy search for actions", completed: false, reminderMinutesBefore: 0 },
  { text: "CommandPalette: Contextual actions based on active view", completed: false, reminderMinutesBefore: 0 },
  { text: "AI Tools: Test Schema Extraction with JS, Python, JSON", completed: false, reminderMinutesBefore: 0 },
  { text: "AI Tools: Test Template Generation for Dockerfile, Python script, Markdown", completed: false, reminderMinutesBefore: 0 },
  { text: "AI Tools: Evaluate effectiveness of custom prompts for all AI tools", completed: false, reminderMinutesBefore: 0 },
  { text: "OmniClip: Verify custom prompt modal functionality for all AI actions", completed: false, reminderMinutesBefore: 0 },
  { text: "Productivity Suite: Calendar - Implement recurring events & ICS import/export", completed: false, reminderMinutesBefore: 0 },
  { text: "Productivity Suite: Graphs - Add Pomodoro focus vs break time chart", completed: false, reminderMinutesBefore: 0 },
  { text: "Productivity Suite: Search - Implement advanced filtering, sorting, and extend to other data sources", completed: false, reminderMinutesBefore: 0 },
  { text: "Productivity Suite: Timeline - Enhance Task integration with duration/time", completed: false, reminderMinutesBefore: 0 },
  { text: "Tasks: Parent task auto-completion when all subtasks are done", completed: false, reminderMinutesBefore: 0 },
  { text: "Tasks: Visual progress bar for parent tasks based on subtask completion", completed: false, reminderMinutesBefore: 0 },
  { text: "Productivity Suite: Write comprehensive unit tests for subtask logic", completed: false, reminderMinutesBefore: 0 },
  { text: "Productivity Suite: Implement end-to-end tests for Pomodoro-Task linking", completed: false, reminderMinutesBefore: 0 },
];

const TasksFeature: React.FC<TasksFeatureProps> = ({ 
    addNotification, 
    navigateToItemId, 
    focusNewTaskInputSignal,
    initialPrefillData,
    onGoToCalendarDate
}) => {
  const [tasks, setTasks] = useState<TaskItem[]>(() => {
    const savedTasks = localStorage.getItem('dashboard-tasks');
    if (savedTasks) {
      try {
        const parsed = JSON.parse(savedTasks).map((task: any) => {
            const createdAtDate = new Date(task.createdAt);
            return { 
                ...task, 
                createdAt: isNaN(createdAtDate.getTime()) ? new Date() : createdAtDate // Fallback for invalid dates
            };
        });
        if (parsed.length > 0) return parsed;
      } catch (e) { console.error("Failed to parse tasks from localStorage", e); }
    }
    return initialDefaultTasks.map(task => ({
      ...task,
      id: crypto.randomUUID(),
      createdAt: new Date(),
    })).sort((a,b) => b.createdAt.getTime() - a.createdAt.getTime());
  });

  const [newTaskText, setNewTaskText] = useState('');
  const [newTaskDueDate, setNewTaskDueDate] = useState('');
  const [newTaskDueTime, setNewTaskDueTime] = useState('');
  const [newTaskReminder, setNewTaskReminder] = useState(0);
  const [newTaskScheduledDate, setNewTaskScheduledDate] = useState('');
  
  const [editingSubtaskId, setEditingSubtaskId] = useState<string | null>(null); 
  const [newSubtaskText, setNewSubtaskText] = useState('');

  const taskItemRefs = useRef<Map<string, HTMLLIElement>>(new Map());
  const newTaskInputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (initialPrefillData) {
        setNewTaskText(initialPrefillData.text);
        if (initialPrefillData.scheduledDate) setNewTaskScheduledDate(initialPrefillData.scheduledDate);
        if (initialPrefillData.dueDate) setNewTaskDueDate(initialPrefillData.dueDate);
        if (newTaskInputRef.current) newTaskInputRef.current.focus();
    }
  }, [initialPrefillData]);


  useEffect(() => {
    try {
        localStorage.setItem('dashboard-tasks', JSON.stringify(tasks.map(task => {
            const createdAtStr = (task.createdAt instanceof Date && !isNaN(task.createdAt.getTime()))
                ? task.createdAt.toISOString()
                : new Date().toISOString(); // Fallback for invalid date objects in state
            return {...task, createdAt: createdAtStr };
        })));
    } catch (error) {
        console.error("Error saving tasks to localStorage:", error);
        addNotification('error', 'Could not save task data.');
    }
  }, [tasks, addNotification]);

  useEffect(() => {
    if (navigateToItemId) {
      const itemRef = taskItemRefs.current.get(navigateToItemId);
      if (itemRef) {
        itemRef.scrollIntoView({ behavior: 'smooth', block: 'center' });
        itemRef.classList.add('ring-2', 'ring-sky-400', 'transition-all', 'duration-1000');
        setTimeout(() => itemRef.classList.remove('ring-2', 'ring-sky-400', 'transition-all', 'duration-1000'), 2000);
      }
    }
  }, [navigateToItemId]);

  useEffect(() => {
    if (focusNewTaskInputSignal && newTaskInputRef.current) {
        newTaskInputRef.current.focus();
    }
  }, [focusNewTaskInputSignal]);

  const handleAddTask = useCallback((e?: React.FormEvent) => {
    if (e) e.preventDefault();
    if (newTaskText.trim()) {
      const newTask: TaskItem = {
        id: crypto.randomUUID(), text: newTaskText.trim(), completed: false, createdAt: new Date(),
        dueDate: newTaskDueDate || undefined, dueTime: newTaskDueTime || undefined,
        reminderMinutesBefore: Number(newTaskReminder) || 0,
        scheduledDate: newTaskScheduledDate || undefined,
      };
      setTasks(prevTasks => [newTask, ...prevTasks].sort((a,b) => b.createdAt.getTime() - a.createdAt.getTime()));
      addNotification('success', `Task "${newTask.text.substring(0,20)}..." added.`, {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TASKS, sourceItemId: newTask.id});
      setNewTaskText(''); setNewTaskDueDate(''); setNewTaskDueTime(''); setNewTaskReminder(0); setNewTaskScheduledDate('');
      if (newTaskInputRef.current) newTaskInputRef.current.focus();
    }
  }, [newTaskText, newTaskDueDate, newTaskDueTime, newTaskReminder, newTaskScheduledDate, addNotification]);
  
  const handleToggleComplete = useCallback((id: string) => {
    setTasks(prevTasks =>
      prevTasks.map(task =>
        task.id === id ? { ...task, completed: !task.completed } : task
      )
    );
    const task = tasks.find(t=> t.id === id);
    if (task) {
      addNotification('info', `Task "${task.text.substring(0,20)}..." marked ${task.completed ? 'active' : 'complete'}.`, {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TASKS, sourceItemId: task.id});
    }
  }, [addNotification, tasks]);

  const handleDeleteTask = useCallback((id: string) => {
    const taskToDelete = tasks.find(t => t.id === id);
    setTasks(prevTasks => prevTasks.filter(task => task.id !== id && task.parentId !== id)); 
    if (taskToDelete) {
      addNotification('info', `Task "${taskToDelete.text.substring(0,20)}..." and its subtasks removed.`, {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TASKS});
    }
  }, [addNotification, tasks]);

  const handleAddSubtask = useCallback((parentId: string) => {
    if (newSubtaskText.trim()) {
        const newSubtask: TaskItem = {
            id: crypto.randomUUID(), text: newSubtaskText.trim(), completed: false,
            createdAt: new Date(), parentId: parentId, reminderMinutesBefore: 0
        };
        setTasks(prevTasks => [...prevTasks, newSubtask].sort((a,b) => b.createdAt.getTime() - new Date(a.createdAt).getTime()));
        addNotification('success', `Subtask added.`, {sourceView: ViewType.PRODUCTIVITY_SUITE, sourceSubView: ProductivitySubViewType.TASKS, sourceItemId: newSubtask.id});
        setNewSubtaskText('');
        setEditingSubtaskId(null); 
    }
  }, [newSubtaskText, addNotification]);

  const reminderOptions = [
    { label: 'No reminder', value: 0 }, { label: '5 mins before', value: 5 }, { label: '15 mins before', value: 15 },
    { label: '30 mins before', value: 30 }, { label: '1 hour before', value: 60 }, { label: '1 day before', value: 1440 },
  ];

  const renderTaskItem = (task: TaskItem, isSubtask: boolean = false) => {
    const subtasks = tasks.filter(st => st.parentId === task.id).sort((a,b) => new Date(a.createdAt).getTime() - new Date(b.createdAt).getTime());
    const dateForCalendarLink = task.dueDate || task.scheduledDate;

    return (
      <li 
        key={task.id}
        ref={(el) => { if (el) taskItemRefs.current.set(task.id, el); else taskItemRefs.current.delete(task.id);}}
        className={`p-3 rounded-lg shadow-md group relative transition-all duration-150
                    ${isSubtask ? 'bg-slate-700/50 ml-6' : 'bg-slate-700'} 
                    ${task.completed ? 'opacity-60' : ''}`}
      >
        <div className="flex items-start space-x-3">
          <input type="checkbox" checked={task.completed} onChange={() => handleToggleComplete(task.id)}
            className="mt-1 h-5 w-5 text-sky-500 bg-slate-600 border-slate-500 rounded focus:ring-2 focus:ring-sky-400 focus:ring-offset-2 focus:ring-offset-slate-700 flex-shrink-0" 
            aria-labelledby={`task-label-${task.id}`}
            />
          <div className="flex-grow min-w-0">
            <span id={`task-label-${task.id}`} className={`block text-sm font-medium break-words ${task.completed ? 'line-through text-slate-400' : 'text-slate-100'}`}>{task.text}</span>
            {(task.dueDate || task.scheduledDate) && (
              <div className="text-xs mt-1 space-x-2 flex items-center">
                {task.dueDate && <span className="text-amber-400/80">Due: {new Date(task.dueDate + (task.dueTime ? `T${task.dueTime}` : 'T23:59:00')).toLocaleString(undefined, {month:'short', day:'numeric', year:'numeric', hour: task.dueTime ? 'numeric' : undefined, minute: task.dueTime ? 'numeric' : undefined})}</span>}
                {task.scheduledDate && !task.dueDate && <span className="text-purple-400/80">Scheduled: {new Date(task.scheduledDate + "T00:00:00").toLocaleDateString(undefined, {month:'short', day:'numeric', year:'numeric'})}</span>}
                {dateForCalendarLink && onGoToCalendarDate && (
                    <button onClick={() => onGoToCalendarDate(dateForCalendarLink)} title="View on Calendar" className="p-0.5 text-slate-400 hover:text-sky-400 rounded-full hover:bg-slate-600/50">
                        <CalendarDaysIconInternal className="w-3.5 h-3.5"/>
                    </button>
                )}
              </div>
            )}
          </div>
          <div className="flex-shrink-0 flex items-center space-x-1 opacity-0 group-hover:opacity-100 focus-within:opacity-100 transition-opacity">
            {!isSubtask && (
                <button onClick={() => setEditingSubtaskId(editingSubtaskId === task.id ? null : task.id)} title="Add subtask" className="p-1.5 text-slate-400 hover:text-sky-400 rounded-full hover:bg-slate-600">
                    <PlusCircleIconInternal className="w-4 h-4"/>
                </button>
            )}
            <button onClick={() => handleDeleteTask(task.id)} title="Delete task" className="p-1.5 text-slate-400 hover:text-red-400 rounded-full hover:bg-slate-600">
              <TrashIconInternal className="w-4 h-4"/>
            </button>
          </div>
        </div>
        {editingSubtaskId === task.id && !isSubtask && (
            <div className="ml-8 mt-2 flex gap-2">
                <input type="text" value={newSubtaskText} onChange={e => setNewSubtaskText(e.target.value)} placeholder="New subtask text..." 
                       className="flex-grow p-1.5 bg-slate-600 border-slate-500 rounded-md text-slate-100 text-xs focus:ring-1 focus:ring-sky-500" />
                <button onClick={() => handleAddSubtask(task.id)} className="px-2 py-1 bg-sky-600 hover:bg-sky-700 text-white text-xs rounded-md">Add</button>
            </div>
        )}
        {subtasks.length > 0 && (
            <ul className="mt-2 space-y-1.5 pl-2 border-l-2 border-slate-600">
                {subtasks.map(subtask => renderTaskItem(subtask, true))}
            </ul>
        )}
      </li>
    );
  };

  const topLevelTasks = tasks.filter(task => !task.parentId).sort((a,b) => new Date(b.createdAt).getTime() - new Date(a.createdAt).getTime());

  return (
    <div className="h-full flex flex-col space-y-4"> 
      <form onSubmit={handleAddTask} className="p-3 bg-slate-800 rounded-lg shadow-md space-y-3 sticky top-0 z-10">
        <input type="text" ref={newTaskInputRef} value={newTaskText} onChange={(e) => setNewTaskText(e.target.value)} placeholder="Add a new task..." 
               className="w-full p-2.5 bg-slate-700 border border-slate-600 rounded-md focus:ring-1 focus:ring-sky-500 text-slate-100 text-sm" />
        <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-4 gap-2 text-xs">
            <div><label htmlFor="newTaskDueDate" className="block text-slate-400 mb-0.5">Due Date</label><input type="date" id="newTaskDueDate" value={newTaskDueDate} onChange={e => setNewTaskDueDate(e.target.value)} className="w-full p-1.5 bg-slate-600 border-slate-500 rounded"/></div>
            <div><label htmlFor="newTaskDueTime" className="block text-slate-400 mb-0.5">Due Time</label><input type="time" id="newTaskDueTime" value={newTaskDueTime} onChange={e => setNewTaskDueTime(e.target.value)} className="w-full p-1.5 bg-slate-600 border-slate-500 rounded"/></div>
            <div><label htmlFor="newTaskReminder" className="block text-slate-400 mb-0.5">Reminder</label><select id="newTaskReminder" value={newTaskReminder} onChange={e => setNewTaskReminder(Number(e.target.value))} className="w-full p-1.5 bg-slate-600 border-slate-500 rounded">{reminderOptions.map(o=><option key={o.value} value={o.value}>{o.label}</option>)}</select></div>
            <div><label htmlFor="newTaskScheduledDate" className="block text-slate-400 mb-0.5">Schedule Date</label><input type="date" id="newTaskScheduledDate" value={newTaskScheduledDate} onChange={e => setNewTaskScheduledDate(e.target.value)} className="w-full p-1.5 bg-slate-600 border-slate-500 rounded"/></div>
        </div>
        <button type="submit" disabled={!newTaskText.trim()} className="w-full px-4 py-2 bg-sky-600 hover:bg-sky-700 text-white font-semibold rounded-md shadow disabled:opacity-60 flex items-center justify-center space-x-2 text-sm">
            <PlusIconInternal className="w-5 h-5"/><span>Add Task</span>
        </button>
      </form>

      {topLevelTasks.length === 0 ? (
        <div className="flex-grow flex flex-col items-center justify-center text-center p-6 text-slate-400">
            <ClipboardDocumentCheckIconInternal className="w-16 h-16 text-slate-500 mb-3"/>
            <p className="text-lg">No tasks yet. Add one above!</p>
        </div>
      ) : (
        <ul className="space-y-2 flex-grow overflow-y-auto min-h-0 pr-1"> 
          {topLevelTasks.map(task => renderTaskItem(task))}
        </ul>
      )}
    </div>
  );
};

// Internal Icons
const PlusIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 4.5v15m7.5-7.5h-15" /></svg>;
const PlusCircleIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M12 9v6m3-3H9m12 0a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const TrashIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M14.74 9l-.346 9m-4.788 0L9.26 9m9.968-3.21c.342.052.682.107 1.022.166m-1.022-.165L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12.56 0c1.153 0 2.24.032 3.223.094M21 12a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>;
const ClipboardDocumentCheckIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M10.125 2.25h-4.5c-.621 0-1.125.504-1.125 1.125v17.25c0 .621.504 1.125 1.125 1.125h12.75c.621 0 1.125-.504 1.125-1.125v-9M10.125 2.25c.882 0 1.724.222 2.494.626M10.125 2.25a2.25 2.25 0 00-2.25 2.25M10.125 2.25v3.375c0 .621.504 1.125 1.125 1.125h3.375M9 15l2.25 2.25L15 12" /></svg>;
const CalendarDaysIconInternal = (props: React.SVGProps<SVGSVGElement>) => <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" strokeWidth={1.5} stroke="currentColor" {...props}><path strokeLinecap="round" strokeLinejoin="round" d="M6.75 3v2.25M17.25 3v2.25M3 18.75V7.5a2.25 2.25 0 012.25-2.25h13.5A2.25 2.25 0 0121 7.5v11.25m-18 0A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75m-18 0v-7.5A2.25 2.25 0 015.25 9h13.5A2.25 2.25 0 0121 11.25v7.5" /></svg>;


export default TasksFeature;
