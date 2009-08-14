./HttpTask_unittest

curl -o "./normal.org" "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html"

CASE_LIST="normal"
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
