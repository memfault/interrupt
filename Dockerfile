FROM ruby:3.2.2-slim-bullseye

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    python3 \
    python3-pip \
    python3-venv \
    nodejs

WORKDIR /memfault/interrupt

COPY Gemfile .
COPY Gemfile.lock .
RUN bundle config force_ruby_platform true
RUN bundle install

COPY requirements.txt .
RUN python3 -m venv /venv && . /venv/bin/activate && \
    python3 -m pip install wheel==0.42.0 && \
    python3 -m pip install -r requirements.txt

COPY entrypoint.sh ./entrypoint.sh

EXPOSE 4000

ENTRYPOINT ["./entrypoint.sh"]
