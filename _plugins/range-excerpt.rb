require 'jekyll'

module Jekyll
    class Excerpt
        alias_method :original_extract_excerpt, :extract_excerpt

        def extract_excerpt(doc_content)
            original_extract_excerpt(doc_content.split('<!-- excerpt start -->').last)
        end
    end
end
