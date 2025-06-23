
import React, { useEffect, useState } from 'react';
import { NotificationType } from '../../types';
import { CheckCircleIcon, XCircleIcon, InformationCircleIcon, ExclamationIcon, XMarkIcon } from '../icons/heroicons';

interface NotificationToastProps {
  message: string;
  type: NotificationType;
  onDismiss: () => void;
}

const NotificationToast: React.FC<NotificationToastProps> = ({ message, type, onDismiss }) => {
  const [isVisible, setIsVisible] = useState(false);

  useEffect(() => {
    setIsVisible(true); // Trigger appear animation
  }, []);

  const handleDismiss = () => {
    setIsVisible(false);
    setTimeout(onDismiss, 300); // Allow time for slide-out animation
  };

  const typeStyles = {
    success: {
      bg: 'bg-green-600',
      icon: <CheckCircleIcon className="h-6 w-6 text-white" />,
      iconBg: 'bg-green-500',
    },
    error: {
      bg: 'bg-red-600',
      icon: <XCircleIcon className="h-6 w-6 text-white" />,
      iconBg: 'bg-red-500',
    },
    info: {
      bg: 'bg-blue-600',
      icon: <InformationCircleIcon className="h-6 w-6 text-white" />,
      iconBg: 'bg-blue-500',
    },
    warning: {
      bg: 'bg-yellow-500',
      icon: <ExclamationIcon className="h-6 w-6 text-white" />,
      iconBg: 'bg-yellow-400',
    },
  };

  const currentStyle = typeStyles[type];

  return (
    <div
      className={`p-4 rounded-lg shadow-2xl flex items-start space-x-3 ${currentStyle.bg} text-white w-full transform transition-all duration-300 ease-in-out ${isVisible ? 'translate-x-0 opacity-100' : 'translate-x-full opacity-0'}`}
      role="alert"
    >
      <div className={`p-1.5 rounded-full ${currentStyle.iconBg}`}>
        {currentStyle.icon}
      </div>
      <div className="flex-1">
        <p className="text-sm font-medium">{message}</p>
      </div>
      <button
        onClick={handleDismiss}
        className="p-1 -m-1 rounded-full hover:bg-white/20 transition-colors"
        aria-label="Dismiss notification"
      >
        <XMarkIcon className="h-5 w-5 text-white" />
      </button>
    </div>
  );
};

export default NotificationToast;
