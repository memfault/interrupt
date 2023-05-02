FROM debian:11.5-slim

RUN apt-get update --yes && \
    apt-get upgrade --yes && \
    apt-get install --yes --no-install-recommends \
      ca-certificates \
      build-essential \
      git python gcc gcc-arm-none-eabi libnewlib-arm-none-eabi \
      vim \
      && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


RUN useradd --create-home -d /home/tiny --shell /bin/bash --password tiny tiny
USER tiny
WORKDIR /home/tiny

COPY --chown=tiny:tiny renode ./renode

WORKDIR /home/tiny/renode
RUN make
