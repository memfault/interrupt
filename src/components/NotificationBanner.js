import React, { useEffect } from 'react';
import { useStaticQuery, graphql } from 'gatsby';

const NotificationBanner = () => {
  const data = useStaticQuery(graphql`
    query NotificationQuery {
      allNotificationData {
        nodes {
          notificationKey
          background
          textcolor
          description
        }
      }
    }
  `);

  const notifications = data.allNotificationData.nodes;

  useEffect(() => {
    // Check cookie and show/hide banner
    const hideBannerCookie = getCookie('hideBanner');
    const banner = document.querySelector('.banner-notifications');
    if (!banner) return;

    if (hideBannerCookie === 'true') {
      banner.style.display = 'none';
    } else {
      banner.style.display = 'block';
    }

    const closeButton = document.querySelector('.js-banner-notification-close');
    if (closeButton) {
      closeButton.addEventListener('click', () => {
        banner.style.display = 'none';
        setCookie('hideBanner', 'true', 1);
      });
    }
  }, []);

  if (!notifications.length) return null;

  return (
    <ul className="banner-notifications">
      {notifications.map(notif => (
        <li
          key={notif.notificationKey}
          className="banner-notification"
          style={{
            '--notificationBackground': notif.background,
            '--notificationColor': notif.textcolor,
          }}
        >
          <div
            style={{ textAlign: 'center' }}
            dangerouslySetInnerHTML={{ __html: notif.description }}
          />
          <button className="banner-notification-close js-banner-notification-close" />
        </li>
      ))}
    </ul>
  );
};

function setCookie(name, value, days) {
  const expires = new Date();
  expires.setTime(expires.getTime() + days * 24 * 60 * 60 * 1000);
  document.cookie = `${name}=${value};expires=${expires.toUTCString()};path=/`;
}

function getCookie(name) {
  const cookieName = `${name}=`;
  const cookies = document.cookie.split(';');
  for (let i = 0; i < cookies.length; i++) {
    let cookie = cookies[i].trim();
    if (cookie.indexOf(cookieName) === 0) {
      return cookie.substring(cookieName.length, cookie.length);
    }
  }
  return null;
}

export default NotificationBanner;
