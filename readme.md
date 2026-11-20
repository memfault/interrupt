<p align="center">
  <img width="300" src="https://user-images.githubusercontent.com/1041679/117912668-bf573700-b294-11eb-9e3f-9cb521b750dc.png"/>
</p>

Interrupt is a community for embedded software makers and professionals alike.

- [Interrupt Slack Channel](https://interrupt-slack.herokuapp.com/)
- [Interrupt Discourse](https://community.memfault.com/)

## Contributing

Interrupt welcomes submissions on embedded software topics.

Prior to getting in touch, you should get yourself acquainted with our
[Code of Conduct](https://interrupt.memfault.com/code-of-conduct).

To submit your content, either email us at interrupt@memfault.com, or open a
pull request!

See [Contributing](https://interrupt.memfault.com/contributing) for more
information.

## Running

### Docker (Recommended)

Follow the instructions in the
[Install Docker Engine](https://docs.docker.com/engine/install/) according to
your operating system.

Clone the repo, run in docker:

```bash
$ git clone https://github.com/memfault/interrupt.git
$ cd interrupt
$ ./interrupt-server.sh
```

You can now access the server at [http://0.0.0.0:4000](http://0.0.0.0:4000)

### Locally

You'll need:

- Python 3.8 or later
- Ruby 3.3.8

Optionally, to run the linters, you'll need `yarn` (easy way is via
[`volta`](https://volta.sh/)).

#### Install Dependencies

Clone the repo and install Python dependencies:

```bash
$ git clone https://github.com/memfault/interrupt.git
$ cd interrupt
# Setup a virtual environment to avoid cluttering your system
$ python3 -m venv .venv
# Activate the environment
$ source .venv/bin/activate
$ pip install -r requirements.txt
```

The virtual environment can be deactivated with `deactivate`.

We highly recommend setting up a version manager for Ruby, such as `rbenv`.
Follow the instructions [here](https://github.com/rbenv/rbenv) to set it up for
your operating system and install the right version of Ruby.

Install Ruby dependencies:

```bash
# Check that your Ruby version is correct
$ ruby -v
$ bundle install
```

Install `yarn` dependencies (this also installs the git hooks):

```bash
$ yarn install
```

#### Launch

Serve with the following command, which will also open up the site in your
browser:

```bash
$ bundle exec jekyll serve --drafts --livereload --open-url --future
```

## Agent Skills

Skills for AI coding agents live in `.agents/skills/`. Each skill is a folder
with a `SKILL.md` describing what it does and the steps it follows.

| Skill             | What it does                                                                                                                                                                            |
| ----------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `publish-article` | Moves a draft from `_drafts/` to `_posts/`, auditing it against the checklist in `.gitmessage-publish-article` and proposing fixes. Every change needs your approval before it is made. |

### Hooking up a skill

Claude Code only discovers project skills in `.claude/skills/`, so each skill is
symlinked there from `.agents/skills/`:

```bash
$ mkdir -p .claude/skills
$ ln -s ../../.agents/skills/publish-article .claude/skills/publish-article
```

The symlink for `publish-article` is already committed. When you add a new
skill, add it under `.agents/skills/<name>/` and create the matching symlink.
Other agents that read `.agents/skills/` directly need no extra setup.

Once hooked up, run the skill in Claude Code with `/publish-article <draft>`,
for example `/publish-article draft_webrtc.md`, or ask Claude to publish a
draft.

## Acknowledgements

Interrupt is based on the Emerald theme by
[Jacopo Rabolini](https://www.jacoporabolini.com/). Emerald is available on
[Github](https://github.com/KingFelix/emerald).

---

Interrupt is sponsored and edited by [Memfault](https://memfault.com).
