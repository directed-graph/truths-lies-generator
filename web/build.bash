#!/usr/bin/env bash

if [[ "${DEBUG:-no}" == "yes" ]]; then
    set -x
fi

image_name="${IMAGE_NAME:-implementing/truths-lies-generator-service}"
instance="${TEMP_NAME:-$(tr -dc A-Za-z0-9 </dev/urandom | head -c 16)}"
api_key="${1}"

set -eu -o pipefail

build="$(git rev-parse HEAD)$(git diff --quiet || echo '+')"
previous_build="$(git show pages | grep '^BUILD: .*$' | cut -d ' ' -f 2 || :)"
if [[ "${SKIP_BUILD_CHECK:-no}" == "no" && \
      "${build}" == "${previous_build}" ]]; then
    echo "build ${build} matches previous build ${previous_build}"
    exit 1
fi

if [[ -z "${api_key}" ]]; then
    echo "must provide API key"
    exit 1
fi

docker build -t "${image_name}" -f web/Dockerfile .
docker create -ti --name "${instance}" "${image_name}" bash

git worktree add "${instance}" pages

rm -r ${instance}/* || :
cd "${instance}"

docker cp "${instance}:/var/www/html/" .
docker rm -f "${instance}"

# indeed, because we run client-side, it is possible to get the key
sed -i "s/<API_KEY>/${api_key}/" html/dist/main.js

mv html/* .
rmdir html

git add .
git commit --allow-empty --message "BUILD: ${build}"
cd ..

git worktree remove "${instance}"

