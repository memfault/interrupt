module Jekyll
  class AuthorPageGenerator < Generator
    safe true

    def generate(site)
      site.data['authors'].each do |author_key, author|
        site.pages << AuthorPage.new(site, site.source, author_key, author)
      end
    end
  end

  class AuthorPage < Page
    def initialize(site, base, author_key, author)
      @site = site
      @base = base
      @dir  = File.join('authors', author_key)
      @name = 'index.html'

      self.process(@name)
      self.read_yaml(File.join(base, '_layouts'), 'author.html')
      self.data['author'] = author
      self.data['author_key'] = author_key
      self.data['title'] = "Author: #{author_key}"
    end
  end
end
