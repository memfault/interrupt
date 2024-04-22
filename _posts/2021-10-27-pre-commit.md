---
title: Automatically format and lint code with pre-commit
description:
  How to use pre-commit for automated linting, formatting, and styling firmware
  code by using Python, clang-format, clang-tidy, and prettier
author: noah
image: img/pre-commit/cover.png
tags: [git, toolchain, better-firmware]
---

I love clean and tidy codebases. I love them even more if I don't have to
manually spend hours doing it myself. The problem is not everyone shares the
same love and passion for tidy codebases, especially when we're pressed for time
trying to get a new firmware build released.

This is where automated formatting and linting tools come in. These tools are
generally run in [continuous integration]({% post_url
2019-09-17-continuous-integration-for-firmware %}) to make sure that all code
committed to the main branch follows the team's agreed-upon format and
structure. We can do one better and hook up these tools to run locally on any
commit or update to a version-controlled branch by using git hooks.

I'm here to talk about one of my favorite tools that is built upon git hooks,
[pre-commit](https://pre-commit.com/), and specifically to detail how you can
use it to format and lint firmware code as you program.

<!-- excerpt start -->

This article provides some background and guidelines for using `pre-commit` to
automatically format and lint C-language firmware codebases. We'll cover general
guidelines and go over an example set of checks that can be helpful when working
on firmware code.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

> Note: jump to [Using `pre-commit`](#using-pre-commit) for the TLDR!

[![img/pre-commit/pre-commit-pass.png](/img/pre-commit/pre-commit-pass.png)](/img/pre-commit/pre-commit-pass.png)

## What is formatting/linting?

Imagine you want to transform the following snippet:

```c
int  main (   int argc, char  ** argv)
   {
printf("hello!\n");
		return  0;}
```

into this, with the push of a button (or even automatically on save!):

```c
int main(int argc, char **argv) {
  printf("hello!\n");
  return 0;
}
```

That's what **formatting** tools can do. For the C language, some popular
formatters are:

- [clang-format](https://releases.llvm.org/15.0.0/tools/clang/docs/ClangFormat.html)
- [uncrustify](http://uncrustify.sourceforge.net/)
- [GNU Indent](https://www.gnu.org/software/indent/)
- [Astyle](http://astyle.sourceforge.net/)

**Linting** is a general term for various static analysis/style checking tools,
generally lighter (in terms of setup and computation) compared to full static
analysis. Typically these tools are designed to detect, for example:

- some simple bug categories (similar to extended compiler warnings)
- possible stylistic errors
  ([an example](https://releases.llvm.org/15.0.0/tools/clang/tools/extra/docs/clang-tidy/checks/bugprone/suspicious-missing-comma.html))
- security-related checks (eg. using `sprintf` instead of `snprintf`)

These types of tools are _very_ common in other software engineering domains
(for example, [ESLint](https://eslint.org/) for javascript), and many popular
projects use automated formatters/linters (Linux, Chromium for example).

## Why format/lint your source code

A lot of digital ink has been spilt on this topic, but to me, these are the
reasons for performing automated styling (formatting) and linting-

### 1. ♻️ Reduce churn in pull request reviews!

The **best** way to document a project's style is to have a tool automate it. If
a pull request has any style violations, we only need a single comment asking:

> Nit: could you run the formatter before merging?

Rather than commenting/suggesting changes on each violation.

It also reduces frustration and simplifies new developer onboarding (no
gatekeeping based on mysterious style choices in the repository).

Codebases should **prioritize** adding automated styling if there's a style
guide that is enforced. For example, Linux has the
["checkpatch.pl"](https://www.kernel.org/doc/html/latest/dev-tools/checkpatch.html)
tool that should always be run before submitting a patch.

### 2. 😪 Remove manual overhead for styling code

I think it's generally preferable to have a consistent style throughout a
codebase- automated styling makes it easy!

Most editors/IDE's will support a way to run styling (eg VSCode has the
`editor.formatOnSave` option, including supporting formatting
[only modified lines](https://code.visualstudio.com/updates/v1_49#_only-format-modified-text)).

### 3. 🐛 Catch bugs or typos

Aside from styling/formatting code, other tools provide checks such as:

- common security checks
- ensure parseable syntax in configuration files
- linting (static analysis checks)

## Why `pre-commit`

A typical software workflow might look like this:

1. **modify** source code
2. **commit changes** to version control (git)
3. **submit** changes to a remote repository (github PR, email list, etc)

A nice "neutral ground" point for running styling/linting is at step #2; there
are countless different methods for modifying source code, which makes it
complicated to integrate with any developer's particular setup (though there are
efforts in this space, eg [EditorConfig](https://editorconfig.org/)).

### git hooks

`git` has a notion of "hooks", which are scripts that run at specific points in
the `git` workflow, including just before committing:

> <https://git-scm.com/book/en/v2/Customizing-Git-Git-Hooks>

There are a couple of different tools that provide ways of adding checks at the
pre-commit stage, some examples:

- [husky](https://github.com/typicode/husky) and
  [lint-staged](https://github.com/okonet/lint-staged) (primarily js-focused)
- [Arcanist](https://secure.phabricator.com/book/phabricator/article/arcanist_lint/)
  (PHP-based, very powerful and customizable, can be difficult to configure)
- [overcommit](https://github.com/sds/overcommit) (Ruby-based, can be difficult
  to customize)

These all do the job, but have some complexities in setup and configuration that
make them a little difficult to install and use ☹️. The whole point is to
_reduce_ friction, after all!

### The `pre-commit` tool

I was in the process of adding Arcanist to a repo at one of my previous jobs
when my coworker (thanks Patrick!) pointed me to `pre-commit`:

> <https://pre-commit.com/>

I was immediately impressed. `pre-commit` really stands out in these areas:

- super simple setup
- easy and flexible configuration
- can be installed in a Python virtual environment, which our team was already
  using

It took about 10 minutes to add `pre-commit` to our repo, and it helped me keep
my patches clean and tidy!

## Why this applies to firmware code

Other popular languages/frameworks have de-facto styling tools, for example:

- Rust: <https://github.com/rust-lang/rustfmt>
- Python: <https://github.com/astral-sh/ruff> or <https://github.com/psf/black>
- Javascript: <https://prettier.io/>
- Go: <https://go.dev/blog/gofmt>

Firmware repositories tend to be a little different:

- ad-hoc or large variety of build systems/packaging approaches (no standard
  `package.json`)
- often multiple programming languages used (C/C++, Rust, Python, bash,
  Javascript, etc)
- tooling can be difficult to set up (especially across Linux/Windows/Mac hosts)
- sometimes enthusiasm for development tooling can be low (creates a need for
  extremely low friction tools)

Tools like `pre-commit` are especially relevant to firmware repositories:

- independent tooling setup (doesn't rely on an existing package manager)
- cross-language!
- quick and simple usage

## Using `pre-commit`

There are some wonderful instructions provided here:

> <https://pre-commit.com/#quick-start>

Briefly, here's how to set up and use the tool:

```bash
# install it with pip
python -m pip install pre-commit

# generate a sample config (you'll want to modify it)
pre-commit sample-config > .pre-commit-config.yaml

# activate pre-commit in a repository
pre-commit install

# pre-commit will now run on future `git commit` operations!

# optionally, the tool can be run standalone too:
# manually run pre-commit on the current checked-in files (git add'd).
pre-commit run

# run pre-commit on all files selected in the config
pre-commit run --all-files
```

See `pre-commit --help` for information on running the tool. A useful command is
`pre-commit autoupdate`, which will update all the checks to the latest tag!

Example run:

[![img/pre-commit/pre-commit-example.png](/img/pre-commit/pre-commit-example.png)](/img/pre-commit/pre-commit-example.png)

### Setup

`pre-commit` is configured with a file name `.pre-commit-config.yaml` at the
root of your repository.

This file selects the hooks to be installed + used, and contains other
configuration values such as paths to exclude from linting, etc. The
documentation here is the reference for config values:

> <https://pre-commit.com/index.html#adding-pre-commit-plugins-to-your-project>

For adding plugins, there's a registry of existing hooks maintained here:

> <https://pre-commit.com/hooks.html>

There are a _lot_ of existing hooks!

Note that individual tools will honor their normal configuration files (eg
`.prettierrc.yml`), so if you have some tools already set up, running them from
`pre-commit` will be the same.

### Example configuration

➡️ A configuration file with **all** the discussed checkers can be found
[here](https://github.com/memfault/interrupt/blob/master/example/pre-commit/.pre-commit-config.yaml).

For a C-and-python firmware repo, here's the config I typically start with:

```yaml
# See https://pre-commit.com for more information
# See https://pre-commit.com/hooks.html for more hooks

# Don't run pre-commit on files under third-party/
exclude: "^\
  (third-party/.*)\
  "

repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.6.0
    hooks:
      - id: check-added-large-files # prevents giant files from being committed.
      - id: check-case-conflict # checks for files that would conflict in case-insensitive filesystems.
      - id: check-merge-conflict # checks for files that contain merge conflict strings.
      - id: check-yaml # checks yaml files for parseable syntax.
      - id: detect-private-key # detects the presence of private keys.
      - id: end-of-file-fixer # ensures that a file is either empty, or ends with one newline.
      - id: fix-byte-order-marker # removes utf-8 byte order marker.
      - id: mixed-line-ending # replaces or checks mixed line ending.
      - id: requirements-txt-fixer # sorts entries in requirements.txt.
      - id: trailing-whitespace # trims trailing whitespace.

  - repo: https://github.com/pre-commit/mirrors-prettier
    rev: v2.5.1
    hooks:
      - id: prettier
        files: \.(js|ts|jsx|tsx|css|less|html|json|markdown|md|yaml|yml)$

  - repo: https://github.com/astral-sh/ruff-pre-commit
    rev: v0.4.1
    hooks:
      - id: ruff
      - id: ruff-format

  - repo: https://github.com/pre-commit/mirrors-clang-format
    rev: v18.1.4
    hooks:
      - id: clang-format
```

The next subsections describe each check above.

#### Basic checks

From the `pre-commit-hooks` repo, these are basic text checks. Most are pretty
self-explanatory- I'm particularly fond of `trim-trailing-whitespace`, since my
editor is set up to do that when saving a file by default 😅.

If your repo has standalone scripts (`chmod +x`), the following additional
checks are quite useful (I've often goofed this up accidentally):

```yaml
- id: check-executables-have-shebangs # ensures that (non-binary) executables have a shebang.
- id: check-shebang-scripts-are-executable # ensures that (non-binary) files with a shebang are executable.
```

The `requirements-txt-fixer` hook only applies if your repository has a
`requirements.txt` file for Python packages.

Some other quite useful ones that are more situation-specific:

- `check-json` - if you have JSON files in your repo
- `check-merge-conflict` - useful if you often rebase/merge
- `check-symlinks` and `destroyed-symlinks` - very helpful if there's symlinks
  checked in to the index
- `check-vcs-permalinks` - particularly useful if there's a lot of documentation
  files tracked
- `file-contents-sorter` - if there are files that benefit from a reliable
  ordering, this is a handy hook

#### Prettier

`prettier` is a Javascript tool that can format many different language types. I
find it does a nice job on Markdown, Javascript, JSON, and YAML files in
particular.

Installing `prettier` can be a bit of a pain if you don't have `node` etc
already available; `pre-commit` manages the tool itself here, which is ✨
amazing ✨!

#### Python checks

- `ruff` - a Python linter that can catch a lot of common issues, and also
  formats and sorts imports. Replaces `flake8` and `isort` and `black` in one
  go!

Another very useful check is `pylint`:

```yaml
- repo: https://github.com/PyCQA/pylint
  rev: v3.1.0
  hooks:
    - id: pylint
```

However, you'll want to spend some quality time with your `.pylintrc` file to
tune to your needs. Out of the box pylint is very strict (but 100% compliance
often leads to nice code!).

There are a lot of other nice checkers available for python projects (eg
`bandit`), definitely worth looking through them.

#### clang-format

This runs the clang-format formatting tool over your C/C++ source files. You'll
probably want to add a `.clang-format` file to the base of your repo to
configure the tool; for example, here's what I usually use for my personal
projects:

```config
---
BasedOnStyle:  Google
---
Language: Cpp
...
```

That config is just selecting the Google C++ style built in to `clang-format`.

The hook provides its own copy of the `clang-format` binary, which means there's
no required setup outside of `pre-commit`! If you're managing your own
`clang-format` tools for your repo, you could instead call `clang-format` as a
`repo: local` + `language: system` hook.

To configure `clang-format`, either add a `.clang-format` file to the repo (the
[default](https://github.com/pre-commit/mirrors-clang-format/blob/ca42fca/.pre-commit-hooks.yaml#L6)),
or specify command-line options in the hook config (eg `args: ["-style=Google", "-i"]`).

Note that `clang-format` has a lot of configuration options. Recommendations for
how to tune a config is outside the scope of this article, but here's some
starting guidance:

- documentation: <https://releases.llvm.org/15.0.0/tools/clang/docs/ClangFormatStyleOptions.html>
- interactive configurator: <https://zed0.co.uk/clang-format-configurator/>
- generate a stub config with ex:
  `clang-format --style=Google --dump-config > .clang-format` (this will dump
  out _all_ the options set by that config. Optionally, the `BasedOnStyle:`
  option will set all of them to those values.

There's also a community-provided
[`clang-format-diff`](https://github.com/jlebar/pre-commit-hooks), which uses
the `git-clang-format` tool to only apply formatting to lines modified in the
current patch (if you're not ready to run a format pass across your entire
repo).

> Extra note: if you do run a whitespace or formatting pass over the entire
> repository, I recommend setting up a `.git-blame-ignore-revs` file, see here
> for more information:<br/> <https://michaelheap.com/git-ignore-rev/>

## Additional hooks

There are a few more hooks that are application-specific but provide a _lot_ of
value.

### clang-tidy

[clang-tidy](https://clang.llvm.org/extra/clang-tidy/) is a powerful C/C++
linter that can catch a lot of straightforward errors (ex: forgetting to
`close()` an open file descriptor). While not as sophisticated as full static
analysis checkers, such as [Code Checker]({% post_url
2021-05-19-static-analysis-with-codechecker %}), it's quite useful.

Using it is a little complicated, depending on how your project is set up.
Generally, you'll need the following steps:

1. get your C/C++ sources compiling with Clang
2. generate a `compile_commands.json` file
3. run `clang-tidy` on the source files in question (with `pre-commit`!)

To drive steps 2 + 3 from `pre-commit`, here's an example
`.pre-commit-config.yaml` section:

```yaml
- repo: local
  hooks:
    # keep this before clang-tidy, it generates compile_commands.json for it.
    # requires the 'compiledb' tool, 'pip install compiledb'
    - id: compiledb
      name: compiledb
      entry: compiledb
      language: system
      args: [--overwrite, make, -n, -B]
      always_run: true
      require_serial: true
      pass_filenames: false

- repo: https://github.com/pocc/pre-commit-hooks
  rev: v1.3.5
  hooks:
    - id: clang-tidy
      args: [-checks=clang-diagnostic-return-type]
      files: src/.*\.c
```

Setting up and configuring `clang-tidy` can be difficult (you'll almost
certainly want a `.clang-tidy` configuration file). Here are some articles that
talk through the process:

- <https://developers.redhat.com/blog/2021/04/06/get-started-with-clang-tidy-in-red-hat-enterprise-linux#>
- <https://www.kdab.com/clang-tidy-part-1-modernize-source-code-using-c11c14/>
- <https://www.kdab.com/clang-tidy-part-1-modernize-source-code-using-c11c14/>

### cppcheck

```yaml
- repo: local
  hooks:
    - id: cppcheck
      name: cppcheck
      entry: cppcheck
      language: system
      args:
        [
          --enable=all,
          --suppress=unusedFunction,
          --suppress=unmatchedSuppression,
          --suppress=missingIncludeSystem,
          --suppress=toomanyconfigs,
          --error-exitcode=1,
        ]
      files: \.(c|h|cpp)$
```

The `cppcheck` hook relies on the `cppcheck` binary being available on the host
system (`sudo apt install cppcheck` on Ubuntu) - note the `- repo: local` and
`language: system` specifiers for the hook.

### Dockerfiles

If you're using Docker (eg for [reproducible builds]({% post_url
2019-12-11-reproducible-firmware-builds %}) or CI), this provides a lot of nice
recommendations for Dockerfiles:

```yaml
- repo: https://github.com/pryorda/dockerfilelint-precommit-hooks
  rev: v0.1.0
  hooks:
    - id: dockerfilelint
```

### GitHub Actions

If you're using GitHub Actions for CI, these hooks do some validation on the
config files that can save some churn when testing new actions:

```yaml
- repo: https://github.com/sirosen/check-jsonschema
  rev: 0.28.2
  hooks:
    - id: check-github-actions
    - id: check-github-workflows
```

### Shellcheck

Some useful and quality-of-life checks for shell scripts:

```yaml
- repo: https://github.com/shellcheck-py/shellcheck-py
  rev: v0.10.0.1
  hooks:
    - id: shellcheck
```

### Mypy

If you're using Python type annotations, you can have mypy run in `pre-commit`:

```yaml
- repo: https://github.com/pre-commit/mirrors-mypy
  rev: "v0.931"
  hooks:
    - id: mypy
```

> Note: take a look at `dmypy` for a way to speed up mypy on larger projects,
> but be sure to test that it's working correctly for you:
> <https://mypy.readthedocs.io/en/stable/mypy_daemon.html>

## Continuous Integration (CI)

An article on tooling would hardly be complete without discussing [continuous
integration]({% post_url 2019-09-17-continuous-integration-for-firmware %}) 😄

Here's a couple of examples of how `pre-commit` could be used in CI:

1. The following command will lint all configured files:

   ```bash
   pre-commit run --all-files
   ```

   It's excellent if you want to keep all files polished in CI.

2. If you want to only lint the _changes_ to files (for example, if you're
   incrementally linting/formatting files rather than in One Big Commit), you
   can set the `${TARGET_BRANCH}` from your CI provider (in GitHub Actions, this
   would be {% raw %}`${{ github.event.pull_request.base.ref }}`{% endraw %}

   ```bash
   pre-commit run --from-ref $(git merge-base ${TARGET_BRANCH}) --to-ref HEAD
   ```

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- [Yelp announcement of
pre-commit](https://engineeringblog.yelp.com/2014/08/announcing-pre-commit-yelps-multi-language-package-manager-for-pre-commit-hooks.html)
<!-- prettier-ignore-end -->
