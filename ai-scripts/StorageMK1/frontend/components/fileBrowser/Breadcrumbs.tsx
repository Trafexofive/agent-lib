
import React from 'react';
import { HomeIcon, ChevronRightIcon } from '../icons/heroicons';

interface BreadcrumbsProps {
  currentPath: string;
  onNavigate: (path: string) => void;
}

const Breadcrumbs: React.FC<BreadcrumbsProps> = ({ currentPath, onNavigate }) => {
  const pathSegments = currentPath.split('/').filter(Boolean);

  const handleNavigateToSegment = (index: number) => {
    const newPath = '/' + pathSegments.slice(0, index + 1).join('/');
    onNavigate(newPath);
  };

  return (
    <nav className="flex items-center text-sm sm:text-base" aria-label="Breadcrumb">
      <ol role="list" className="flex items-center space-x-1 sm:space-x-2">
        <li>
          <div>
            <button
              onClick={() => onNavigate('/')}
              className="text-text-secondary hover:text-text-primary transition-colors duration-150 flex items-center"
            >
              <HomeIcon className="flex-shrink-0 h-5 w-5 mr-1" aria-hidden="true" />
              <span className="sr-only">Home</span>
            </button>
          </div>
        </li>
        {pathSegments.map((segment, index) => (
          <li key={segment + index}>
            <div className="flex items-center">
              <ChevronRightIcon className="flex-shrink-0 h-5 w-5 text-gray-500" aria-hidden="true" />
              <button
                onClick={() => handleNavigateToSegment(index)}
                className={`ml-1 sm:ml-2 text-sm font-medium ${index === pathSegments.length - 1 ? 'text-text-primary cursor-default' : 'text-text-secondary hover:text-text-primary transition-colors duration-150'}`}
                aria-current={index === pathSegments.length - 1 ? 'page' : undefined}
                disabled={index === pathSegments.length -1}
              >
                {segment}
              </button>
            </div>
          </li>
        ))}
      </ol>
    </nav>
  );
};

export default Breadcrumbs;
