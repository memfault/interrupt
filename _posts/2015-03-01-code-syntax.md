---
title: Code Syntax
---
Per inserire del codice in evidenza all'interno di un post è sufficente utilizzare alcuni tag particolari, come descritto direttamente all'interno della [documentazione di Jekyll](http://jekyllrb.com/docs/templates/#code-snippet-highlighting). Così facendo il codice verrà racchiuso all'interno di una classe CSS ``.highlight`` e verrà evidenziato secondo lo stile presente nel file [syntax.scss](https://github.com/mojombo/tpw/blob/master/css/syntax.css). Questo file è lo stile standard adottato direttamente da **Github** per evidenziare il codice. 

This is a CSS example:
{% highlight css linenos %}

body {
  background-color: #fff;
  }

h1 {
  color: #ffaa33;
  font-size: 1.5em;
  }

{% endhighlight %}

And this is a HTML example, with a linenumber:
{% highlight html linenos %}

<html>
  <a href="example.com">Example</a>
</html>

{% endhighlight %}

Last, a Ruby example:
{% highlight ruby linenos %}

def hello
  puts "Hello World!"
end

{% endhighlight %}
