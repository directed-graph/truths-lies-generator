
FROM implementing/node-grpc-web

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

ENTRYPOINT ["npx", "http-server"]
CMD ["-p", "8081"]

