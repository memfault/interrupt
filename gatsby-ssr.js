const React = require('react');

// Inject dark mode anti-FOUC script before any other scripts
exports.onRenderBody = ({ setHeadComponents }) => {
  setHeadComponents([
    React.createElement('script', {
      key: 'dark-mode-fouc',
      dangerouslySetInnerHTML: {
        __html: `
(function() {
  var theme = localStorage.getItem('theme');
  if (theme === 'dark' || theme === 'light') {
    document.documentElement.setAttribute('data-theme', theme);
  }
})();
        `.trim(),
      },
    }),
  ]);
};
