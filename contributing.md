---
layout: page
title: Contributing
---
# Contributing
<br >
Interrupt welcomes submissions on embedded software topics.

Prior to getting in touch, you should get yourself acquainted with our [Code of
Conduct](/code-of-conduct).

To get in touch about contributing, simply submit the form below. You may also
email us at  [interrupt@memfault.com](mailto:interrupt@memfault.com), or open a
pull request on [Github](https://github.com/memfault/interrupt).

<iframe src="https://docs.google.com/forms/d/e/1FAIpQLSfOrn7QxNZmuIDhHYf_aSbVzd-zUZUxHiPc56ecukt53LhWJw/viewform?embedded=true" width="640" height="854" frameborder="0" marginheight="0" marginwidth="0">Loading...</iframe>

## Post formatting

This blog uses Jekyll, and as such posts are formatted using
[Markdown](https://www.markdownguide.org/cheat-sheet) and [Liquid
themes](https://shopify.github.io/liquid/).

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
