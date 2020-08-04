---
layout: page
title: Contributing
---
# Contributing

Interrupt is first and foremost a community of embedded software engineers,
and as such we'd be thrilled to showcase your work, promote your projects, and
learn from your experiences.

We welcome all embedded software content, from beginner tutorials to bleeding
edge technology demonstrations.

We believe good technical content is worth paying for, so we will gladly
compensate you for your time spent writing for Interrupt. We also provide
editorial support, and promote your posts across social media and our
newsletter.

To get in touch about contributing, simply submit the form below. You may also
email us at  [interrupt@memfault.com](mailto:interrupt@memfault.com), or open a
pull request on [Github](https://github.com/memfault/interrupt).

<div class="contribute-section container">
  <iframe class="form" src="https://docs.google.com/forms/d/e/1FAIpQLSfOrn7QxNZmuIDhHYf_aSbVzd-zUZUxHiPc56ecukt53LhWJw/viewform?embedded=true" frameborder="0" marginheight="0" marginwidth="0">Loading...</iframe>
</div>


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
