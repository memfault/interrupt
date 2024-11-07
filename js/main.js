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
		icon.addEventListener('click', toggle);
	} else if (document.attachEvent && icon !== null) {
		icon.attachEvent('onclick', toggle);
	} else {
		return;
	}
}

function darkModeSetup() {
    if (getCookie("theme") == "dark") {
        html.addClass("dark");
    }
    toggle = document.getElementById("theme-toggle");
    toggle.click(function(e) {
        e.preventDefault();
        html.toggleClass("dark");
        if (html.hasClass("dark")){
            document.cookie = "theme=dark;path=/";
        }
        else {
            document.cookie = "theme=light;path=/";
        }
    })
}

// Function to set a cookie
function setCookie(name, value, days) {
    const expires = new Date();
    expires.setTime(expires.getTime() + (days * 24 * 60 * 60 * 1000));
    document.cookie = `${name}=${value};expires=${expires.toUTCString()};path=/`;
}

// Function to get the value of a cookie
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

// Function to check the hideBanner cookie and show/hide the banner
function checkAndDisplayBanner() {
    const hideBannerCookie = getCookie('hideBanner');
    const banner = document.querySelector('.banner-notifications');

    if (banner && hideBannerCookie === 'true') {
        banner.style.display = 'none'; // Hide the banner if cookie is set to true
    } else {
        banner.style.display = 'block'; // Show the banner otherwise
    }

	notificationBannerClose();
}


// Closes the banner and save the cookie
function closeBanner() {
    const banner = document.querySelector('.banner-notifications');
    if (!banner) return;

	banner.style.display = 'none';
    setCookie('hideBanner', 'true', 1); // Set the cookie to expire in 1 day
}

// Closes the
function notificationBannerClose() {
	const closeButton = document.querySelector('.js-banner-notification-close');

	if (closeButton) {
		closeButton.addEventListener('click', closeBanner);
	}
}

// TODO dark mode
// darkModeSetup();
menuClick();
searchScroll();

window.addEventListener('load', checkAndDisplayBanner);
