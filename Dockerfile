FROM ruby:2.7

WORKDIR /memfault/interrupt

ENTRYPOINT ["./interrupt-cli"]
