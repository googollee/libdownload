./HttpSession_unittest

curl -o "./normal.org" "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html"
curl -o "./cant_get_right_length.org" "http://www.boost.org/doc/libs/1_39_0/more/getting_started/unix-variants.html"
curl -o "./part_download.org" -r 0-99 "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html"
curl -o "./part_download_from_middle.org" -r 0-99 "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html"

CASE_LIST="normal cant_get_right_length part_download part_download_from_middle"
for i in $CASE_LIST
do
    diff ./$i.download ./$i.org
    if [ "$?" != "0" ]
    then
        echo case $i fail.
        exit -1
    else
        rm ./$i.download ./$i.org
    fi
done
