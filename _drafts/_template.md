---
# This is a template for Interrupt posts. Use previous, more recent posts from the _posts/
# directory for more inspiration and patterns.
#
# When submitting a post for publishing, submit a PR with the post in the _drafts/ directory
# and it will be moved to _posts/ on the date of publish.
#
# e.g.
# $ cp _drafts/_template.md _drafts/my_post.md
#
# It will now show up in the front page when running Jekyll locally.

title: Post Title
description: 
  Post Description (~140 words, used for discoverability and SEO) # update using key words
author: tyler # update with author handle from authors.yml
tags: [tag1, tag2] # update with relevant tags (see https://interrupt.memfault.com/tags for examples!)
---

A little bit of background of why the reader should read this post.

<!-- excerpt start -->

Excerpt Content

<!-- excerpt end -->

Optional motivation to continue onwards

{% include newsletter.html %}

{% include toc.html %}

## Primary Section 1

<!-- Uncomment and update image path to add an image. Place all images under img/my-post-name/ -->
<!-- <p align="center">
 <img width="80%" src="{% img_url my-post-name/my-image.png %}" alt="My post description" />
</p> -->

## Conclusion

Wrap up with a summary and optionally a call for readers to join the conversation.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
