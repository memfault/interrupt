## About
Emerald is a minimal theme created for Jekyll. The main purpose of Emerald is to provide a clear theme for those who want a blog ready to use, focused on the content and mobile-first.

![Emerald](/img/Emerald01.png "Emerald")

## Setup & usage
Emerald may be installed by simply downloading the .zip folder from the [repository on Github](https://github.com/KingFelix/emerald/archive/master.zip).

After extracting the content from the folder into the selected directory, you can type ``jekyll serve`` from the terminal, than open your browser to ``0.0.0.0:4000/emerald/`` and you will find it there.

Additionally it is possible to fork the repository and use Github Pages as hosting. By following this way it will be enough to change the ``baseurl`` value into the ``_config.yml`` file, with the directory name of your project (for example /blog) or simply with a "/" (slash) if you want install Emerald in the root.

### Branch
Emerald has two branch: 
- ``master``: is for developing pourpose.
- ``gh-pages``: is only for demo site.  

### Baseurl
Emerald was thought to be used mainly with Github, in particular into [project site](https://pages.github.com/). For this reason several tags have been included ``{{ site.baseurl }}`` to refer to the "/emerald/" directory.
You can change the "baseurl" value into the ``config.yml`` file, to match your directory (for example "/blog/") or the root of your project. In that case you must set the "baseurl" value to "/".

### Typography
To maintain the vertical rhythm, it has been applied a **Typographic scale** as a modular scale, with a baseline set to 24px. To maintain this rhythm you need to insert elements like image, video or other contents with a 24px (or multiple) height as refer.

Last but not least: the [Jekyll documentation](http://jekyllrb.com) is the best starting point! 

## Author

### Jacopo Rabolini

- Web site: [www.jacoporabolini.com](http://www.jacoporabolini.com)
- Google+: [+JacopoRabolini](https://plus.google.com/u/0/+JacopoRabolini/posts)

## License
Emerald is released under [MIT License](license.md).
