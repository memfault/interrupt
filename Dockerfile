FROM ruby:2.7-alpine

RUN apk --update add --no-cache --virtual build-dependencies build-base bash

WORKDIR /memfault/interrupt

COPY Gemfile .
COPY Gemfile.lock .

RUN bundle install

EXPOSE 4000

ENTRYPOINT ["./entrypoint.sh"]
