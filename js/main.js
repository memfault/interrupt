var normal = document.getElementById("nav-menu");
var reverse = document.getElementById("nav-menu-left");
var html = document.documentElement;

var icon = normal !== null ? normal : reverse;

// Toggle the "menu-open" % "menu-opn-left" classes
function toggle() {
  var navRight = document.getElementById("nav");
  var navLeft = document.getElementById("nav-left");
  var nav = navRight !== null ? navRight : navLeft;

  var button = document.getElementById("menu");
  var site = document.getElementById("wrap");

  if (nav.className == "menu-open" || nav.className == "menu-open-left") {
    nav.className = "";
    button.className = "";
    site.className = "";
  } else if (reverse !== null) {
    nav.className += "menu-open-left";
    button.className += "btn-close";
    site.className += "fixed";
  } else {
    nav.className += "menu-open";
    button.className += "btn-close";
    site.className += "fixed";
  }
}

// Ensures backward compatibility with IE old versions
function menuClick() {
  if (document.addEventListener && icon !== null) {
    icon.addEventListener("click", toggle);
  } else if (document.attachEvent && icon !== null) {
    icon.attachEvent("onclick", toggle);
  } else {
    return;
  }
}

function darkModeSetup() {
  // Apply stored preference (anti-FOUC script in <head> already does this,
  // but this handles cases where JS runs after page load).
  var saved = localStorage.getItem("theme");
  if (saved === "dark" || saved === "light") {
    document.documentElement.setAttribute("data-theme", saved);
  }

  var btn = document.getElementById("theme-toggle");
  if (!btn) return;

  updateThemeToggle(btn);

  btn.addEventListener("click", function () {
    var current = document.documentElement.getAttribute("data-theme"); // null, 'dark', or 'light'
    var next;
    if (!current) {
      next = "dark"; // auto → dark
    } else if (current === "dark") {
      next = "light"; // dark → light
    } else {
      next = null; // light → auto (system)
    }

    if (next) {
      document.documentElement.setAttribute("data-theme", next);
      localStorage.setItem("theme", next);
    } else {
      document.documentElement.removeAttribute("data-theme");
      localStorage.removeItem("theme");
    }

    updateThemeToggle(btn);
  });
}

function updateThemeToggle(btn) {
  if (!btn) btn = document.getElementById("theme-toggle");
  if (!btn) return;

  var theme = document.documentElement.getAttribute("data-theme");
  var icon = btn.querySelector("i");

  if (theme === "dark") {
    icon.className = "fa fa-moon-o";
    btn.title = "Dark mode (click for light)";
  } else if (theme === "light") {
    icon.className = "fa fa-sun-o";
    btn.title = "Light mode (click for system)";
  } else {
    icon.className = "fa fa-adjust";
    btn.title = "System theme (click for dark)";
  }
}

// Function to set a cookie
function setCookie(name, value, days) {
  const expires = new Date();
  expires.setTime(expires.getTime() + days * 24 * 60 * 60 * 1000);
  document.cookie = `${name}=${value};expires=${expires.toUTCString()};path=/`;
}

// Function to get the value of a cookie
function getCookie(name) {
  const cookieName = `${name}=`;
  const cookies = document.cookie.split(";");

  for (let i = 0; i < cookies.length; i++) {
    let cookie = cookies[i].trim();
    if (cookie.indexOf(cookieName) === 0) {
      return cookie.substring(cookieName.length, cookie.length);
    }
  }
  return null;
}

// Function to check the hideBanner cookie and show/hide the banner
function checkAndDisplayBanner() {
  const hideBannerCookie = getCookie("hideBanner");
  const banner = document.querySelector(".banner-notifications");

  if (banner && hideBannerCookie === "true") {
    banner.style.display = "none"; // Hide the banner if cookie is set to true
  } else {
    banner.style.display = "block"; // Show the banner otherwise
  }

  notificationBannerClose();
}

// Closes the banner and save the cookie
function closeBanner() {
  const banner = document.querySelector(".banner-notifications");
  if (!banner) return;

  banner.style.display = "none";
  setCookie("hideBanner", "true", 1); // Set the cookie to expire in 1 day
}

// Closes the
function notificationBannerClose() {
  const closeButton = document.querySelector(".js-banner-notification-close");

  if (closeButton) {
    closeButton.addEventListener("click", closeBanner);
  }
}

darkModeSetup();
menuClick();
searchScroll();

window.addEventListener("load", checkAndDisplayBanner);
