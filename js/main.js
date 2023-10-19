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

// Ensures backward compatibility with IE old versions
function searchScroll() {
	const scrollBox = document.getElementById('search-results');
	const SCROLLED_CLASSNAME = 'scrolled-down';

	if (document.addEventListener && scrollBox) {
		scrollBox.addEventListener('scroll', () => {
			if (scrollBox.scrollTop > 50)  {
				scrollBox.classList.add(SCROLLED_CLASSNAME);
			} else {
				scrollBox.classList.remove(SCROLLED_CLASSNAME);
			}
		});
	}
}

// TODO dark mode
// darkModeSetup();
menuClick();
searchScroll();
