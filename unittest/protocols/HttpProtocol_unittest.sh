#!/bin/sh

curl -o curl_easy_setopt.html.org http://curl.haxx.se/libcurl/c/curl_easy_setopt.html

./HttpProtocol_unittest

diff curl_easy_setopt.html curl_easy_setopt.html.org