FROM alpine:3.16

WORKDIR /tools

RUN apk add git make curl gcc tar
RUN apk add libc-dev

RUN curl -R -O http://www.lua.org/ftp/lua-5.4.4.tar.gz
RUN tar zxf lua-5.4.4.tar.gz
RUN rm lua-5.4.4.tar.gz
RUN cd lua-5.4.4 && make all test && make install

RUN apk add git make automake autoconf libtool
RUN git clone https://github.com/swig/swig --branch v4.1.0

RUN apk add pcre2-dev
RUN apk add bison flex
RUN apk add gcc g++

RUN cd swig && ./autogen.sh && ./configure && make
RUN cd swig && make install

RUN apk add cmake

# Check installed programs
RUN luac -v
RUN swig -version

WORKDIR /app