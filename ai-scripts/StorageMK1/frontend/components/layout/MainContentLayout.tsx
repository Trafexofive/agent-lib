
import React, { ReactNode } from 'react';

interface MainContentLayoutProps {
  children: ReactNode;
}

const MainContentLayout: React.FC<MainContentLayoutProps> = ({ children }) => {
  return (
    <main className="flex-grow container mx-auto p-4 bg-card-bg backdrop-filter backdrop-blur-xs rounded-lg shadow-lg">
      {children}
    </main>
  );
};

export default MainContentLayout;
