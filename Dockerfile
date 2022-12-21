FROM ruby:3.0-slim-buster

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    python3 \
    python3-pip

# explicit and recent version of pip avoids needing to build wheel deps
RUN pip3 install pip==22.3.1

WORKDIR /memfault/interrupt

COPY Gemfile .
COPY Gemfile.lock .
RUN bundle install

COPY requirements.txt .
RUN python3 -m pip install -r requirements.txt

COPY entrypoint.sh ./entrypoint.sh

EXPOSE 4000

ENTRYPOINT ["./entrypoint.sh"]
