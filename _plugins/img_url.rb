module Jekyll
  module Tags
    class ImgUrl < Liquid::Tag
      include Jekyll::Filters::URLFilters

      def initialize(tag_name, relative_path, tokens)
        super
  
        @relative_path = relative_path.strip
      end

      def render(context)
        @context = context
        return relative_url('/img/' + @relative_path)
      ensure
        @context = nil
      end
    end
  end
end

Liquid::Template.register_tag('img_url', Jekyll::Tags::ImgUrl)
