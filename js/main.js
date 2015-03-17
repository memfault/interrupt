var icon = document.getElementById("nav-menu");

// Toggle the "menu-open" class
function toggle() {
	  var nav = document.getElementById("nav");
	  var button = document.getElementById("menu");
	  var site = document.getElementById("wrap");
	  
	  if (nav.className == "menu-open") {
	  	  nav.className = "";
	  	  button.className = "";
	  	  site.className = "";
	  } else {
	  	  nav.className += "menu-open";
	  	  button.className += "btn-close";
	  	  site.className += "fixed";
	    }
	}

// Ensures backward compatibility with IE old versions
function menuClick() {
	if (document.addEventListener) {
		icon.addEventListener('click', toggle);
	} else if (document.attachEvent) {
		icon.attachEvent('onclick', toggle);
	}
}

menuClick();