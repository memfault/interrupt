FROM ruby:2.7-alpine

RUN apk --update add --no-cache --virtual build-dependencies build-base bash

WORKDIR /memfault/interrupt

ENTRYPOINT ["./entrypoint.sh"]
