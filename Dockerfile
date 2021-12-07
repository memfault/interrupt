FROM ruby:2.7-alpine

RUN apk --update add --no-cache --virtual build-dependencies build-base bash

WORKDIR /memfault/interrupt

COPY Gemfile .

RUN bundle install

COPY ./ /memfault/interrupt

# Need to copy and install rake requirements here

# Then copy over full contents

EXPOSE 4000

ENTRYPOINT ["./entrypoint.sh"]
