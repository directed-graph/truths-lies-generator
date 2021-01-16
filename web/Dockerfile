
FROM node:12-stretch

RUN apt-get -qq update && apt-get -qq install -y unzip

WORKDIR /tmp

RUN curl -sSL https://github.com/protocolbuffers/protobuf/releases/download/v3.14.0/\
protoc-3.14.0-linux-x86_64.zip -o protoc.zip && \
  unzip -qq protoc.zip && \
  cp ./bin/protoc /usr/local/bin/protoc && \
  cp -r ./include/* /usr/local/include

RUN curl -sSL https://github.com/grpc/grpc-web/releases/download/1.2.1/\
protoc-gen-grpc-web-1.2.1-linux-x86_64 -o /usr/local/bin/protoc-gen-grpc-web && \
  chmod +x /usr/local/bin/protoc-gen-grpc-web

WORKDIR /github/directed-graph/truths-lies-generator

COPY ./web web
COPY ./truths_lies_generator.proto .

RUN protoc -I=. truths_lies_generator.proto \
    --js_out=import_style=commonjs:./web \
    --grpc-web_out=import_style=commonjs,mode=grpcwebtext:./web

WORKDIR /github/directed-graph/truths-lies-generator/web

RUN npm install && \
    npm link grpc-web && \
    npx webpack && \
    mkdir -p /var/www/html/dist && \
    cp index.html /var/www/html && \
    cp dist/main.js /var/www/html/dist

WORKDIR /var/www/html

EXPOSE 8081
ENTRYPOINT ["python3", "-m", "http.server"]
CMD ["8081"]

