import './src/styles/main.scss';
import 'prismjs/themes/prism-tomorrow.css';

export const onClientEntry = () => {
  // Restore theme preference on client
  if (typeof window !== 'undefined') {
    const theme = localStorage.getItem('theme');
    if (theme === 'dark' || theme === 'light') {
      document.documentElement.setAttribute('data-theme', theme);
    }
  }
};
