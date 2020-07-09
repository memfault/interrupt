module Jekyll
  module Tags
    class TagUrl < Liquid::Tag
      include Jekyll::Filters::URLFilters

      def initialize(tag_name, tag, tokens)
        super
  
        @tag = tag.strip
      end

      def render(context)
        @context = context
        return relative_url('/tag/' + @tag)
      ensure
        @context = nil
      end
    end
  end
end

Liquid::Template.register_tag('tag_url', Jekyll::Tags::TagUrl)
