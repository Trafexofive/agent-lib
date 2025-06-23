
import React, { useState, useCallback } from 'react';
import { NotificationItem, ViewType } from '../types'; 

interface StdMetricsFeatureProps {
  addNotification: ( 
    type: NotificationItem['type'], 
    message: string, 
    sourceDetails?: { 
      sourceView?: ViewType; 
    }
  ) => void;
}

const MetricsUtilitySection: React.FC<{ title: string; children: React.ReactNode }> = ({ title, children }) => (
  <section className="bg-slate-800 p-6 rounded-xl shadow-xl">
    <h2 className="text-2xl font-semibold text-teal-400 mb-6">{title}</h2>
    <div className="space-y-4">
      {children}
    </div>
  </section>
);

const ResultDisplay: React.FC<{ label: string; value: string | number | string[]; isCode?: boolean }> = ({ label, value, isCode = false }) => (
  <div className="py-2">
    <span className="text-sm font-medium text-slate-400">{label}: </span>
    {isCode ? (
      <code className="text-md text-emerald-300 bg-slate-700 px-2 py-1 rounded">{Array.isArray(value) ? value.join(', ') : value}</code>
    ) : (
      <span className="text-md text-emerald-300">{Array.isArray(value) ? value.join(', ') : value}</span>
    )}
  </div>
);

const dataUnits = { Bytes: 1, KB: 1024, MB: 1024 ** 2, GB: 1024 ** 3, TB: 1024 ** 4 };
type DataUnit = keyof typeof dataUnits;
const dataUnitOptions = Object.keys(dataUnits) as DataUnit[];

const convertDataSize = (value: number, fromUnit: DataUnit, toUnit: DataUnit): number => {
    if (isNaN(value) || !dataUnits[fromUnit] || !dataUnits[toUnit]) return NaN;
    const valueInBytes = value * dataUnits[fromUnit];
    return valueInBytes / dataUnits[toUnit];
};

const StdMetricsFeature: React.FC<StdMetricsFeatureProps> = ({ addNotification }) => {
  const [statsInput, setStatsInput] = useState('');
  const [statsResults, setStatsResults] = useState<any>(null);
  const [dataSizeInput, setDataSizeInput] = useState<string>('1024');
  const [dataSizeFromUnit, setDataSizeFromUnit] = useState<DataUnit>('KB');
  const [dataSizeToUnit, setDataSizeToUnit] = useState<DataUnit>('MB');
  const [dataSizeOutput, setDataSizeOutput] = useState<string>('');
  const [currentValue, setCurrentValue] = useState<string>('0');
  const [targetValue, setTargetValue] = useState<string>('100');

  const handleCalculateStats = useCallback(() => {
    const numbers = statsInput.split(',').map(s => parseFloat(s.trim())).filter(n => !isNaN(n));
    if (numbers.length === 0) {
      addNotification('error', 'Enter valid numbers.', {sourceView: ViewType.STD_METRICS});
      setStatsResults(null); return;
    }
    numbers.sort((a, b) => a - b);
    const count = numbers.length;
    const sum = numbers.reduce((acc, n) => acc + n, 0);
    const mean = sum / count;
    let median;
    if (count % 2 === 0) median = (numbers[count / 2 - 1] + numbers[count / 2]) / 2;
    else median = numbers[Math.floor(count / 2)];
    const frequencyMap = new Map<number, number>();
    numbers.forEach(num => frequencyMap.set(num, (frequencyMap.get(num) || 0) + 1));
    let maxFreq = 0;
    frequencyMap.forEach(freq => maxFreq = Math.max(maxFreq, freq));
    const mode: number[] = [];
    frequencyMap.forEach((freq, num) => { if (freq === maxFreq && maxFreq > 0) mode.push(num); });
    const modeToDisplay = mode.length > 0 && mode.length < count ? mode : (mode.length === count ? [] : ['N/A']);
    const variance = numbers.reduce((acc, n) => acc + Math.pow(n - mean, 2), 0) / count;
    const stdDev = Math.sqrt(variance);
    setStatsResults({
      Count: count, Sum: sum.toFixed(2), Mean: mean.toFixed(2), Median: median.toFixed(2),
      Mode: modeToDisplay.map(m => typeof m === 'number' ? m.toFixed(2) : m),
      Min: numbers[0].toFixed(2), Max: numbers[count - 1].toFixed(2), 'Std Dev': stdDev.toFixed(2),
    });
    addNotification('success', 'Stats calculated.', {sourceView: ViewType.STD_METRICS});
  }, [statsInput, addNotification]);

  const handleConvertDataSize = useCallback(() => {
    const value = parseFloat(dataSizeInput);
    if (isNaN(value)) {
      addNotification('error', 'Invalid input for data size.', {sourceView: ViewType.STD_METRICS});
      setDataSizeOutput(''); return;
    }
    const result = convertDataSize(value, dataSizeFromUnit, dataSizeToUnit);
    if (isNaN(result)) {
        addNotification('error', 'Conversion failed.', {sourceView: ViewType.STD_METRICS});
        setDataSizeOutput('');
    } else {
        setDataSizeOutput(`${result.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 4})} ${dataSizeToUnit}`);
        addNotification('success', 'Data size converted.', {sourceView: ViewType.STD_METRICS});
    }
  }, [dataSizeInput, dataSizeFromUnit, dataSizeToUnit, addNotification]);

  const current = parseFloat(currentValue) || 0;
  const target = parseFloat(targetValue) || 100;
  let progressPercentage = 0;
  if (target !== 0) progressPercentage = Math.max(0, Math.min(100, (current / target) * 100));
  else if (current > 0) progressPercentage = 100;

  return (
    <div className="space-y-10">
      <header className="text-center mb-10"><h1 className="text-4xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-teal-400 to-cyan-400">Std::Metrics</h1><p className="text-slate-400 mt-1 text-md">Tools for metrics analysis & visualization.</p></header>
      <MetricsUtilitySection title="Statistical Calculator">
        <div><label htmlFor="stats-input" className="block text-sm font-medium text-slate-300 mb-1">Comma-separated Numbers</label><textarea id="stats-input" rows={3} value={statsInput} onChange={(e) => setStatsInput(e.target.value)} placeholder="e.g., 1, 2.5, 3" className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-teal-500 text-slate-100" /></div>
        <button onClick={handleCalculateStats} disabled={!statsInput.trim()} className="px-4 py-2 bg-teal-600 hover:bg-teal-700 text-white font-semibold rounded-md disabled:opacity-50">Calculate</button>
        {statsResults && <div className="mt-4 p-4 bg-slate-700/50 rounded-md grid grid-cols-1 sm:grid-cols-2 gap-x-4">{Object.entries(statsResults).map(([key, value]) => (<ResultDisplay key={key} label={key} value={value as string | number | string[]} isCode />))}</div>}
      </MetricsUtilitySection>
      <MetricsUtilitySection title="Data Size Converter">
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4 items-end">
          <div><label htmlFor="data-size-input" className="block text-sm font-medium text-slate-300 mb-1">Value</label><input type="number" id="data-size-input" value={dataSizeInput} onChange={(e) => setDataSizeInput(e.target.value)} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-teal-500 text-slate-100" /></div>
          <div><label htmlFor="data-size-from-unit" className="block text-sm font-medium text-slate-300 mb-1">From Unit</label><select id="data-size-from-unit" value={dataSizeFromUnit} onChange={(e) => setDataSizeFromUnit(e.target.value as DataUnit)} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-teal-500 text-slate-100">{dataUnitOptions.map(unit => <option key={unit} value={unit}>{unit}</option>)}</select></div>
          <div><label htmlFor="data-size-to-unit" className="block text-sm font-medium text-slate-300 mb-1">To Unit</label><select id="data-size-to-unit" value={dataSizeToUnit} onChange={(e) => setDataSizeToUnit(e.target.value as DataUnit)} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-teal-500 text-slate-100">{dataUnitOptions.map(unit => <option key={unit} value={unit}>{unit}</option>)}</select></div>
        </div>
        <button onClick={handleConvertDataSize} disabled={!dataSizeInput.trim()} className="mt-4 px-4 py-2 bg-teal-600 hover:bg-teal-700 text-white font-semibold rounded-md disabled:opacity-50">Convert</button>
        {dataSizeOutput && <div className="mt-4"><ResultDisplay label="Converted Value" value={dataSizeOutput} isCode /></div>}
      </MetricsUtilitySection>
      <MetricsUtilitySection title="Goal Tracker">
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4 items-end">
          <div><label htmlFor="current-value" className="block text-sm font-medium text-slate-300 mb-1">Current</label><input type="number" id="current-value" value={currentValue} onChange={(e) => setCurrentValue(e.target.value)} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-teal-500 text-slate-100" /></div>
          <div><label htmlFor="target-value" className="block text-sm font-medium text-slate-300 mb-1">Target</label><input type="number" id="target-value" value={targetValue} onChange={(e) => setTargetValue(e.target.value)} className="w-full p-3 bg-slate-700 border border-slate-600 rounded-md focus:ring-2 focus:ring-teal-500 text-slate-100" /></div>
        </div>
        <div className="mt-4"><p className="text-sm text-slate-400 mb-1">Progress: {progressPercentage.toFixed(1)}%</p><div className="w-full bg-slate-700 rounded-full h-6 shadow-inner overflow-hidden"><div className="bg-gradient-to-r from-sky-500 to-teal-500 h-6 rounded-full transition-all duration-500 ease-out text-center text-white text-sm flex items-center justify-center" style={{ width: `${progressPercentage}%` }} role="progressbar" aria-valuenow={progressPercentage} aria-valuemin={0} aria-valuemax={100}>{progressPercentage > 10 && `${progressPercentage.toFixed(0)}%`}</div></div></div>
        {target === 0 && current === 0 && <p className="text-xs text-slate-500 mt-2">Set a target value.</p>}
      </MetricsUtilitySection>
    </div>
  );
};

export default StdMetricsFeature;