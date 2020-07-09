# Make post_url use relative_url (shipped in Jekyll 4.0)
# https://github.com/jekyll/jekyll/commit/2591f33aa861ebe0294cc2b5d7b718f3cea5e21c
module Jekyll
  module Tags
    class FixedPostUrl < PostUrl
      include Jekyll::Filters::URLFilters

      def render(context)
        @context = context
        return relative_url(super)
      ensure
        @context = nil
      end
    end
  end
end

Liquid::Template.register_tag('post_url', Jekyll::Tags::FixedPostUrl)
