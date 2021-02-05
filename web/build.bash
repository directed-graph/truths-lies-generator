#!/usr/bin/env bash

image_name="${IMAGE_NAME:-implementing/truths-lies-generator-service}"
temp_name="${TEMP_NAME:-$(tr -dc A-Za-z0-9 </dev/urandom | head -c 16)}"
api_key="${1}"

set -e -o pipefail

if [[ -z "${api_key}" ]]; then
    echo "must provide API key"
    exit 1
fi

docker build -t "${image_name}" -f web/Dockerfile .
docker create -ti --name "${temp_name}" "${image_name}" bash

git checkout pages

docker cp "${temp_name}:/var/www/html/" .
docker rm -f "${temp_name}"

# indeed, because we run client-side, it is possible to get the key
sed -i "s/<API_KEY>/${api_key}/" html/dist/main.js

git checkout master

