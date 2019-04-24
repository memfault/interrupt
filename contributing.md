---
layout: page
title: Contributing
---
# Contributing
<br >
Interrupt welcomes submissions on embedded software topics.

Prior to getting in touch, you should get yourself acquainted with our [Code of
Conduct](/code-of-conduct).

To submit your content, either email us at
interrupt@memfault.com, or open a pull request on
[Github](https://github.com/memfault/interrupt).

## Post formatting

This blog uses Jekyll, and as such posts are formatted using Markdown and Liquid themes.

A few things to note:

* Excerpts are identified with comment tags. For example:
{% highlight markdown %}
This is not in the excerpt
<!-- excerpts start -->
This is in the excerpt
<!-- excerpts end -->
{% endhighlight %}

* You may specify a language for synthax highlighting as follow:
{% highlight markdown %}
```c
int foo(void) {
  return 0;
}
```
{% endhighlight %}

## Byline

Please submit your bio & photo in the same pull request as your first post.

Photos should go under `img/author`, while your information should go in
`_data/authors.yml`.
